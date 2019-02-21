/* 
 * File:   app.h
 * Author: dmiles813
 *
 * Created on December 15, 2017, 3:15 PM
 */

#ifndef APP_H
#define	APP_H

#ifdef	__cplusplus
extern "C" {
#endif

#define _XTAL_FREQ 64000000

#define FIFO_COUNT 4
#define FIFO_SIZE 32
#define HOST_RX_FIFO        0
#define DEVICE_TX_FIFO      1
#define DEVICE_RX_FIFO      2
#define HOST_TX_FIFO        3

#define TRUE 1
#define FALSE 0

#define DIGITAL_OUT_WORD_COUNT 3
    
void TinyDelay();
void DigitalBreakout(unsigned int newData);
unsigned long ToAscii(unsigned long in);
unsigned char NibbleToAscii(unsigned char in);


// LED PINS
#define PIN_LED_RED_TRIS            TRISAbits.TRISA6
#define PIN_LED_RED_LATCH           LATAbits.LATA6
#define PIN_LED_GREEN_TRIS          TRISAbits.TRISA7
#define PIN_LED_GREEN_LATCH         LATAbits.LATA7
#define PIN_LED_BLUE_TRIS           TRISBbits.TRISB5
#define PIN_LED_BLUE_LATCH          LATBbits.LATB5


// DIGITAL BREAKOUT PINS
#define PIN_DIG_OUT_14_TRIS         TRISBbits.TRISB4
#define PIN_DIG_OUT_14_LATCH        LATBbits.LATB4
#define PIN_DIG_OUT_13_TRIS         TRISBbits.TRISB3
#define PIN_DIG_OUT_13_LATCH        LATBbits.LATB3
#define PIN_DIG_OUT_12_TRIS         TRISBbits.TRISB2
#define PIN_DIG_OUT_12_LATCH        LATBbits.LATB2
#define PIN_DIG_OUT_11_TRIS         TRISBbits.TRISB1
#define PIN_DIG_OUT_11_LATCH        LATBbits.LATB1
#define PIN_DIG_OUT_10_TRIS         TRISBbits.TRISB0
#define PIN_DIG_OUT_10_LATCH        LATBbits.LATB0

#define PIN_DIG_OUT_9_TRIS          TRISCbits.TRISC5
#define PIN_DIG_OUT_9_LATCH         LATCbits.LATC5
#define PIN_DIG_OUT_8_TRIS          TRISCbits.TRISC4
#define PIN_DIG_OUT_8_LATCH         LATCbits.LATC4
#define PIN_DIG_OUT_7_TRIS          TRISCbits.TRISC3
#define PIN_DIG_OUT_7_LATCH         LATCbits.LATC3
#define PIN_DIG_OUT_6_TRIS          TRISCbits.TRISC2
#define PIN_DIG_OUT_6_LATCH         LATCbits.LATC2
#define PIN_DIG_OUT_5_TRIS          TRISCbits.TRISC1
#define PIN_DIG_OUT_5_LATCH         LATCbits.LATC1
#define PIN_DIG_OUT_4_TRIS          TRISCbits.TRISC0
#define PIN_DIG_OUT_4_LATCH         LATCbits.LATC0

#define PIN_DIG_OUT_3_TRIS          TRISAbits.TRISA3
#define PIN_DIG_OUT_3_LATCH         LATAbits.LATA3
#define PIN_DIG_OUT_2_TRIS          TRISAbits.TRISA2
#define PIN_DIG_OUT_2_LATCH         LATAbits.LATA2
#define PIN_DIG_OUT_1_TRIS          TRISAbits.TRISA1
#define PIN_DIG_OUT_1_LATCH         LATAbits.LATA1
#define PIN_DIG_OUT_0_TRIS          TRISAbits.TRISA0
#define PIN_DIG_OUT_0_LATCH         LATAbits.LATA0



#ifdef	__cplusplus
}
#endif

#endif	/* APP_H */

