/*
 * File:   main.c
 * Author: dmiles813
 */

#include "app.h"
#include <xc.h>
#include "osc.h"
#include "uart.h"
#include "fifo.h"
#include "galaxy.h"

// PIC18LF26K22 Configuration Bit Settings
// 'C' source line config statements

// CONFIG1H
#pragma config FOSC = INTIO67   // Oscillator Selection bits (Internal oscillator block)
#pragma config PLLCFG = ON      // 4X PLL Enable (Oscillator multiplied by 4)
#pragma config PRICLKEN = ON    // Primary clock enable bit (Primary clock enabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enable bit (Fail-Safe Clock Monitor disabled)
#pragma config IESO = ON        // Internal/External Oscillator Switchover bit (Oscillator Switchover mode enabled)

// CONFIG2L
#pragma config PWRTEN = OFF     // Power-up Timer Enable bit (Power up timer disabled)
#pragma config BOREN = SBORDIS  // Brown-out Reset Enable bits (Brown-out Reset enabled in hardware only (SBOREN is disabled))
#pragma config BORV = 190       // Brown Out Reset Voltage bits (VBOR set to 1.90 V nominal)

// CONFIG2H
#pragma config WDTEN = OFF      // Watchdog Timer Enable bits (Watch dog timer is always disabled. SWDTEN has no effect.)
#pragma config WDTPS = 32768    // Watchdog Timer Postscale Select bits (1:32768)

// CONFIG3H
#pragma config CCP2MX = PORTC1  // CCP2 MUX bit (CCP2 input/output is multiplexed with RC1)
#pragma config PBADEN = ON      // PORTB A/D Enable bit (PORTB<5:0> pins are configured as analog input channels on Reset)
#pragma config CCP3MX = PORTB5  // P3A/CCP3 Mux bit (P3A/CCP3 input/output is multiplexed with RB5)
#pragma config HFOFST = ON      // HFINTOSC Fast Start-up (HFINTOSC output and ready status are not delayed by the oscillator stable status)
#pragma config T3CMX = PORTC0   // Timer3 Clock input mux bit (T3CKI is on RC0)
#pragma config P2BMX = PORTB5   // ECCP2 B output mux bit (P2B is on RB5)
#pragma config MCLRE = EXTMCLR  // MCLR Pin Enable bit (MCLR pin enabled, RE3 input pin disabled)

// CONFIG4L
#pragma config STVREN = ON      // Stack Full/Underflow Reset Enable bit (Stack full/underflow will cause Reset)
#pragma config LVP = ON         // Single-Supply ICSP Enable bit (Single-Supply ICSP enabled if MCLRE is also 1)
#pragma config XINST = OFF      // Extended Instruction Set Enable bit (Instruction set extension and Indexed Addressing mode disabled (Legacy mode))

// CONFIG5L
#pragma config CP0 = OFF        // Code Protection Block 0 (Block 0 (000800-003FFFh) not code-protected)
#pragma config CP1 = OFF        // Code Protection Block 1 (Block 1 (004000-007FFFh) not code-protected)
#pragma config CP2 = OFF        // Code Protection Block 2 (Block 2 (008000-00BFFFh) not code-protected)
#pragma config CP3 = OFF        // Code Protection Block 3 (Block 3 (00C000-00FFFFh) not code-protected)

// CONFIG5H
#pragma config CPB = OFF        // Boot Block Code Protection bit (Boot block (000000-0007FFh) not code-protected)
#pragma config CPD = OFF        // Data EEPROM Code Protection bit (Data EEPROM not code-protected)

// CONFIG6L
#pragma config WRT0 = OFF       // Write Protection Block 0 (Block 0 (000800-003FFFh) not write-protected)
#pragma config WRT1 = OFF       // Write Protection Block 1 (Block 1 (004000-007FFFh) not write-protected)
#pragma config WRT2 = OFF       // Write Protection Block 2 (Block 2 (008000-00BFFFh) not write-protected)
#pragma config WRT3 = OFF       // Write Protection Block 3 (Block 3 (00C000-00FFFFh) not write-protected)

// CONFIG6H
#pragma config WRTC = OFF       // Configuration Register Write Protection bit (Configuration registers (300000-3000FFh) not write-protected)
#pragma config WRTB = OFF       // Boot Block Write Protection bit (Boot Block (000000-0007FFh) not write-protected)
#pragma config WRTD = OFF       // Data EEPROM Write Protection bit (Data EEPROM not write-protected)

// CONFIG7L
#pragma config EBTR0 = OFF      // Table Read Protection Block 0 (Block 0 (000800-003FFFh) not protected from table reads executed in other blocks)
#pragma config EBTR1 = OFF      // Table Read Protection Block 1 (Block 1 (004000-007FFFh) not protected from table reads executed in other blocks)
#pragma config EBTR2 = OFF      // Table Read Protection Block 2 (Block 2 (008000-00BFFFh) not protected from table reads executed in other blocks)
#pragma config EBTR3 = OFF      // Table Read Protection Block 3 (Block 3 (00C000-00FFFFh) not protected from table reads executed in other blocks)

// CONFIG7H
#pragma config EBTRB = OFF      // Boot Block Table Read Protection bit (Boot Block (000000-0007FFh) not protected from table reads executed in other blocks)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

buffer16 buffers[FIFO_COUNT];
galaxyBuffer galaxyCommands[GALAXY_COMMAND_COUNT];
unsigned int digitalOutHyst[DIGITAL_OUT_WORD_COUNT];
unsigned int led_green_delay = 0;
unsigned int led_red_delay = 0;
unsigned char addressDatagramCount = 0;

// High priority interrupt
void interrupt tc_int(void) {
    if (TMR1IE && TMR1IF)
    {
        TMR1IF=0;
    }
}

// Low priority interrupt
void interrupt low_priority LowIsr(void) {
//    PIN_DIG_OUT_12_TRIS = 0;
//    PIN_DIG_OUT_12_LATCH = 1;

    if(INTCONbits.T0IF && INTCONbits.T0IE)  // If Timer flag is set & Interrupt is enabled
    {
        // Transmit a char
        if (IsTransmitterReady(UART1_INDEX) && TXSTA1bits.TRMT == 1 && !IsFifoEmpty(&buffers[DEVICE_TX_FIFO])) {
            unsigned int data = FifoDequeue(&buffers[DEVICE_TX_FIFO]);
            PutChar9Default(data);
            led_red_delay = 100;
        }

        unsigned int resetValue = 0xFFFF - 120;
        TMR0H = (resetValue >> 8);
        TMR0L = (resetValue & 0xFF);
        INTCONbits.T0IF = 0;            // Clear the interrupt flag 
    }

//    PIN_DIG_OUT_12_LATCH = 0;
}

void main(void) {

    unsigned long l = 0;
    unsigned long m = 0;
    unsigned int data;
    unsigned char commandNumber = 0;
    unsigned int i = 0;
    unsigned int j = 0;
    unsigned char tf = 0;
    unsigned char c = 0;
    unsigned short crc;
    unsigned char galaxyCommandCount = 0;
        
    ANSELA = 0;
    ANSELB = 0;
    ANSELC = 0;

    ConfigureOscillator();
    UART_Initialize(
            UART1_INDEX,
            UART_BAUD_19200,
            UART_9BIT_MODE,
            UART_INTERRUPTS_DISABLED
    );
//    UART_Initialize(
//            UART2_INDEX,
//            UART_BAUD_115200,
//            UART_8BIT_MODE,
//            UART_INTERRUPTS_DISABLED
//    );
    EnableTransceiverRX(UART1_INDEX);
//    EnableTransceiverRX(UART2_INDEX);

    // Initialize Digital Sequencer
    for (unsigned char x=0; x < 4; x++) {
        digitalOutHyst[x] = 0x0000;
    }
        
    // DISCONNECT
    c=0;
    galaxyCommands[c].word_count = 0;
    galaxyCommands[c].buffer[galaxyCommands[c].word_count++] = 0x01FF;
    galaxyCommands[c].buffer[galaxyCommands[c].word_count++] = 0x0007;
    galaxyCommands[c].buffer[galaxyCommands[c].word_count++] = 0x0057;
    galaxyCommands[c].buffer[galaxyCommands[c].word_count++] = 0x0004;
    galaxyCommands[c].buffer[galaxyCommands[c].word_count++] = 0x0001;
    galaxyCommands[c].crc = compute_crc(&galaxyCommands[c].buffer[0], galaxyCommands[c].word_count);
    galaxyCommandCount++;

    // CHOOSE SLOT
    c=1;
    galaxyCommands[c].word_count = 0;
    galaxyCommands[c].buffer[galaxyCommands[c].word_count++] = 0x01FF;
    galaxyCommands[c].buffer[galaxyCommands[c].word_count++] = 0x0006;
    galaxyCommands[c].buffer[galaxyCommands[c].word_count++] = 0x0043;
    galaxyCommands[c].buffer[galaxyCommands[c].word_count++] = (GALAXY_MAX_SLOTS - 1);
    galaxyCommands[c].crc = compute_crc(&galaxyCommands[c].buffer[0], galaxyCommands[c].word_count);
    galaxyCommandCount++;

    // POLL SLOTS
    for (unsigned char slot=0; slot <= GALAXY_MAX_SLOTS; slot++) {
        c++;
        galaxyCommands[c].word_count = 0;
        galaxyCommands[c].buffer[galaxyCommands[c].word_count++] = 0x01FF;
        galaxyCommands[c].buffer[galaxyCommands[c].word_count++] = 0x0006;
        galaxyCommands[c].buffer[galaxyCommands[c].word_count++] = 0x0050;
        galaxyCommands[c].buffer[galaxyCommands[c].word_count++] = slot;
        galaxyCommands[c].crc = compute_crc(&galaxyCommands[c].buffer[0], galaxyCommands[c].word_count);
        galaxyCommandCount++;
    }

    for (unsigned char x=0; x <= FIFO_COUNT; x++) {
        FifoInitialize(&buffers[x]);
    };

//    T1CON = 0x1;               //Configure Timer1 interrupt
//    PIE1bits.TMR1IE = 1;           
//    INTCONbits.PEIE = 1;
//    RCONbits.IPEN=0x01;
//    IPR1bits.TMR1IP=0x01;            // TMR1 high priority ,TMR1 Overflow Interrupt Priority bit
//    INTCONbits.GIE = 1;
//    PIR1bits.TMR1IF = 0;

    T0CONbits.TMR0ON = 0;                   // Off
    T0CONbits.T08BIT = 0;                   // 16 bit
    T0CONbits.T0CS = 0;                     // Clocked by instruction cycle (FOSC/4)
    T0CONbits.T0SE = 0;                     // Low to high transition
    T0CONbits.PSA = 0;                      // Use prescaler
    T0CON = (T0CON & 0xF8) | 0x07;          // 1:256 prescaler
    INTCON2bits.TMR0IP=0x00;                // Use low priority ISR
    INTCONbits.TMR0IE = 1;                  // Enable interrupt on TMR0 overflow
    T0CONbits.TMR0ON = 1;                   // Enable the timer

    RCONbits.IPEN = 1;                      // Enable interrupt priorities
    INTCONbits.GIEL = 1;                    // Enable Global Low Priority Interrupts
    INTCONbits.GIE = 1;                     // Enable interrupts

    while (1) {
        PIN_DIG_OUT_12_TRIS = 0;
        if (PIN_DIG_OUT_12_LATCH == 0) {
            PIN_DIG_OUT_12_LATCH = 1;
        } else {
            PIN_DIG_OUT_12_LATCH = 0;
        }

        PIN_DIG_OUT_9_TRIS = 0;
        PIN_DIG_OUT_9_LATCH = RCSTA1bits.FERR;
        PIN_DIG_OUT_10_TRIS = 0;
        PIN_DIG_OUT_10_LATCH = RCSTA1bits.OERR;

        TinyDelay();

        // LED Control
        if (l % 32768 == 0) {
            PIN_LED_BLUE_TRIS = 0;
            if (PIN_LED_BLUE_LATCH == 1) {
                PIN_LED_BLUE_LATCH = 0;
            } else {
                PIN_LED_BLUE_LATCH = 1;
            }
        }
        if (led_red_delay > 0) {
            PIN_LED_RED_TRIS = 0;
            PIN_LED_RED_LATCH = 0;
            led_red_delay--;
        } else {
            PIN_LED_RED_LATCH = 1;
        }
        if (led_green_delay > 0) {
            PIN_LED_GREEN_TRIS = 0;
            PIN_LED_GREEN_LATCH = 0;
            led_green_delay--;
        } else {
            PIN_LED_GREEN_TRIS = 1;
        }

        TinyDelay();

        if (l == 700000) {
            for (unsigned char k=0; k < galaxyCommands[commandNumber].word_count; k++) {
                FifoEnqueue(&buffers[DEVICE_TX_FIFO], galaxyCommands[commandNumber].buffer[k]);
            }    
            FifoEnqueue(&buffers[DEVICE_TX_FIFO], galaxyCommands[commandNumber].crc >> 8);
            FifoEnqueue(&buffers[DEVICE_TX_FIFO], galaxyCommands[commandNumber].crc & 0xFF);
            
            commandNumber++;
            if (commandNumber > galaxyCommandCount) {
                commandNumber = 2;
            }
            
            l = 0;
        }

        l++;
    }
 
}

void TinyDelay() {
    // Receive a char
    if (IsRxDataAvailable(UART1_INDEX) && !IsFifoFull(&buffers[DEVICE_RX_FIFO])) {
        unsigned int data = GetChar9(UART1_INDEX);
        //FifoEnqueue(&buffers[DEVICE_RX_FIFO], data);
        DigitalBreakout(data);
        led_green_delay = 5000;
    }

//    if (!IsFifoEmpty(&buffers[DEVICE_RX_FIFO])) {
//        led_green_delay = 10000;
//        unsigned int temp = FifoDequeue(&buffers[DEVICE_RX_FIFO]);
//        DigitalBreakout(temp);
//        unsigned long temp2 = ToAscii(temp);
//    }

}

unsigned long ToAscii(unsigned long in) {
    unsigned long temp;
    temp = NibbleToAscii((unsigned char)(in >> 12));
    temp = temp << 8;
    temp += NibbleToAscii((unsigned char)(in >> 8));
    temp = temp << 8;
    temp += NibbleToAscii((unsigned char)(in >> 4));
    temp = temp << 8;
    temp += NibbleToAscii((unsigned char)(in));
    return temp;
}

unsigned char NibbleToAscii(unsigned char in) {
    in = in & 0x0F;
    if (in >= 0x00 && in <= 0x09) {
        return '0' + in;
    } else if (in == 0x0A) {
        return 'A';
    } else if (in == 0x0B) {
        return 'B';
    } else if (in == 0x0C) {
        return 'C';
    } else if (in == 0x0D) {
        return 'D';
    } else if (in == 0x0E) {
        return 'E';
    } else if (in == 0x0F) {
        return 'F';
    } else {
        return '?';
    }
}

void DigitalBreakout(unsigned int newData) {
    // Shift hysteresis words
    for (unsigned char x=0; x < (DIGITAL_OUT_WORD_COUNT - 1); x++) {
        digitalOutHyst[x] = digitalOutHyst[x+1];
    }
    // Store new data at end of shift register
    digitalOutHyst[DIGITAL_OUT_WORD_COUNT - 1] = newData;

    if ((newData & 0x01FF) == 0x0100) {
        newData = newData | 0x0800;
    }
    
    unsigned int output = newData;

//    unsigned int mask = 0x4000;

//    PIN_DIG_OUT_14_TRIS = 0;
//    if ((output & mask) == mask) {
//        PIN_DIG_OUT_14_LATCH = 1;
//    } else {
//        PIN_DIG_OUT_14_LATCH = 0;
//    }
//    
//    mask = mask >> 1;
//    PIN_DIG_OUT_13_TRIS = 0;
//    if ((output & mask) == mask) {
//        PIN_DIG_OUT_13_LATCH = 1;
//    } else {
//        PIN_DIG_OUT_13_LATCH = 0;
//    }
//    
//    mask = mask >> 1;
//    PIN_DIG_OUT_12_TRIS = 0;
//    if ((output & mask) == mask) {
//        PIN_DIG_OUT_12_LATCH = 1;
//    } else {
//        PIN_DIG_OUT_12_LATCH = 0;
//    }
//    
//    mask = mask >> 1;
//    PIN_DIG_OUT_11_TRIS = 0;
//    if ((output & mask) == mask) {
//        PIN_DIG_OUT_11_LATCH = 1;
//    } else {
//        PIN_DIG_OUT_11_LATCH = 0;
//    }
//    
//    mask = mask >> 1;
//    PIN_DIG_OUT_10_TRIS = 0;
//    if ((output & mask) == mask) {
//        PIN_DIG_OUT_10_LATCH = 1;
//    } else {
//        PIN_DIG_OUT_10_LATCH = 0;
//    }
//    
//    mask = mask >> 1;
//    PIN_DIG_OUT_9_TRIS = 0;
//    if ((output & mask) == mask) {
//        PIN_DIG_OUT_9_LATCH = 1;
//    } else {
//        PIN_DIG_OUT_9_LATCH = 0;
//    }
    
    unsigned int mask = 0x0200;
    mask = mask >> 1;
    PIN_DIG_OUT_8_TRIS = 0;
    if ((output & mask) == mask) {
        PIN_DIG_OUT_8_LATCH = 1;
    } else {
        PIN_DIG_OUT_8_LATCH = 0;
    }
    
    mask = mask >> 1;
    PIN_DIG_OUT_7_TRIS = 0;
    if ((output & mask) == mask) {
        PIN_DIG_OUT_7_LATCH = 1;
    } else {
        PIN_DIG_OUT_7_LATCH = 0;
    }
    
    mask = mask >> 1;
    PIN_DIG_OUT_6_TRIS = 0;
    if ((output & mask) == mask) {
        PIN_DIG_OUT_6_LATCH = 1;
    } else {
        PIN_DIG_OUT_6_LATCH = 0;
    }
    
    mask = mask >> 1;
    PIN_DIG_OUT_5_TRIS = 0;
    if ((output & mask) == mask) {
        PIN_DIG_OUT_5_LATCH = 1;
    } else {
        PIN_DIG_OUT_5_LATCH = 0;
    }
    
    mask = mask >> 1;
    PIN_DIG_OUT_4_TRIS = 0;
    if ((output & mask) == mask) {
        PIN_DIG_OUT_4_LATCH = 1;
    } else {
        PIN_DIG_OUT_4_LATCH = 0;
    }
    
    mask = mask >> 1;
    PIN_DIG_OUT_3_TRIS = 0;
    if ((output & mask) == mask) {
        PIN_DIG_OUT_3_LATCH = 1;
    } else {
        PIN_DIG_OUT_3_LATCH = 0;
    }
    
    mask = mask >> 1;
    PIN_DIG_OUT_2_TRIS = 0;
    if ((output & mask) == mask) {
        PIN_DIG_OUT_2_LATCH = 1;
    } else {
        PIN_DIG_OUT_2_LATCH = 0;
    }
    
    mask = mask >> 1;
    PIN_DIG_OUT_1_TRIS = 0;
    if ((output & mask) == mask) {
        PIN_DIG_OUT_1_LATCH = 1;
    } else {
        PIN_DIG_OUT_1_LATCH = 0;
    }
    
    mask = mask >> 1;
    PIN_DIG_OUT_0_TRIS = 0;
    if ((output & mask) == mask) {
        PIN_DIG_OUT_0_LATCH = 1;
    } else {
        PIN_DIG_OUT_0_LATCH = 0;
    }
}

