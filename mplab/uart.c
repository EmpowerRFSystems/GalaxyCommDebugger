#include <xc.h>
#include "app.h"
#include "uart.h"

//===============================================================================
//	Description:	This function initializes the internal UART.
//
//	Params:			BAUD				ULONG		Desired baud rate
//						MODE_9BIT		UCHAR		Cofnigure 8 or 9 bit mode
//						INTERRUPT_C...	UCHAR		Configure interrupt control
//
//	Returns:			NONE
//===============================================================================
void UART_Initialize (	unsigned char uart_index,
                        unsigned long baud, 
                        unsigned char mode_9bit, 
                        unsigned char interrupt_control )
{
	unsigned long temp = 0;
	unsigned long clock_freq = _XTAL_FREQ;

	/*
	RECEPTION

	1.		Initialize the SPBRGH:SPBRG registers for the appropriate baud rate.
			Set or clear the BRGH and BRG16 bits, as required, to achieve the desired baud rate.
	2.		Enable the asynchronous serial port by clearing bit SYNC and setting bit SPEN.
	3.		If interrupts are desired, set enable bit RCIE.
	4.		If 9-bit reception is desired, set bit RX9.
	5.		Enable the reception by setting bit CREN.
	6.		Flag bit, RCIF, will be set when reception is complete and an interrupt will be
			generated if enable bit, RCIE, was set.
	7.		Read the RCSTA register to get the 9th bit (if enabled) and determine if any error
			occurred during reception.
	8.		Read the 8-bit received data by reading the RCREG register.
	9.		If any error occurred, clear the error by clearing enable bit CREN.
	10.	If using interrupts, ensure that the GIE and PEIE bits in the INTCON register
			(INTCON<7:6>) are set.

	TRANSMISSION

	1.		Initialize the SPBRGH:SPBRG registers for the appropriate baud rate.
			Set or clear the BRGH and BRG16 bits, as required, to achieve the desired baud rate.
	2.		Enable the asynchronous serial port by clearing bit SYNC and setting bit SPEN.
	3. 	If interrupts are desired, set enable bit TXIE.
	4.		If 9-bit transmission is desired, set transmit bit TX9. Can be used as address/data bit.
	5. 	Enable the transmission by setting bit TXEN which will also set bit TXIF.
	6. 	If 9-bit transmission is selected, the ninth bit should be loaded in bit TX9D.
	7.		Load data to the TXREG register (starts transmission).
	8. 	If using interrupts, ensure that the GIE and PEIE bits in the INTCON register (INTCON<7:6>)
			are set.
	*/

	// Set BRG Control
    if (uart_index == UART1_INDEX) {
        BAUDCON1bits.BRG16 = 1;
        TXSTA1bits.BRGH = 1;
    } else if (uart_index == UART2_INDEX) {
        BAUDCON2bits.BRG16 = 1;
        TXSTA2bits.BRGH = 1;
    }

	// Calculate value for baud register, given the states of BRG16 and BRGH
	temp = clock_freq / baud;
    temp = (temp / 4);
	temp = temp - 1;
    if (uart_index == UART1_INDEX) {
        SPBRGH1 = temp >> 8;
        SPBRG1 = (char)temp;
    } else if (uart_index == UART2_INDEX) {
        SPBRGH2 = temp >> 8;
        SPBRG2 = (char)temp;
    }
    
	// RX & TX must be configured as inputs for UART to work
    if (uart_index == UART1_INDEX) {
        TRISCbits.TRISC6 = 1;
        TRISCbits.TRISC7 = 1;
        TXSTA1bits.SYNC = 0;
    } else if (uart_index == UART2_INDEX) {
        TRISBbits.TRISB6 = 1;
        TRISBbits.TRISB7 = 1;
        TXSTA2bits.SYNC = 0;
    }
    
	// 9 bit reception & transmission (Enabled => 1)
	if (mode_9bit == UART_9BIT_MODE)
	{
        if (uart_index == UART1_INDEX) {
            RCSTA1bits.RX9 = 1;
            TXSTA1bits.TX9 = 1;
        } else if (uart_index == UART2_INDEX) {
            RCSTA2bits.RX9 = 1;
            TXSTA2bits.TX9 = 1;
        }
	}
	else
	{
        if (uart_index == UART1_INDEX) {
            RCSTA1bits.RX9 = 0;
            TXSTA1bits.TX9 = 0;
        } else if (uart_index == UART2_INDEX) {
            RCSTA2bits.RX9 = 0;
            TXSTA2bits.TX9 = 0;
        }
	}

    BAUDCON1bits.DTRXP1 = 1;
    
	// Enable Receiver Hardware Circuitry
    if (uart_index == UART1_INDEX) {
        RCSTA1bits.CREN = 1;
    } else if (uart_index == UART2_INDEX) {
        RCSTA2bits.CREN = 1;
    }

    // Enable Serial Port
    if (uart_index == UART1_INDEX) {
        RCSTA1bits.SPEN = 1;
    } else if (uart_index == UART2_INDEX) {
        RCSTA2bits.SPEN = 1;
    }
    
	// Clear RCIF, if set
    if (uart_index == UART1_INDEX) {
        if (PIR1bits.RC1IF)
            temp = RC1REG;
    } else if (uart_index == UART2_INDEX) {
        if (PIR3bits.RC2IF)
            temp = RC1REG;
    }
    
	// Configure Interrupts for UART
	if (interrupt_control == UART_INTERRUPTS_LOW_PRI || interrupt_control == UART_INTERRUPTS_HIGH_PRI)
	{
		// Configure Low or High Priority for UART
		if (interrupt_control == UART_INTERRUPTS_LOW_PRI)
		{
            if (uart_index == UART1_INDEX) {
            	IPR1bits.RC1IP = 0;
                IPR1bits.TX1IP = 0;
            } else if (uart_index == UART2_INDEX) {
            	IPR3bits.RC2IP = 0;
                IPR3bits.TX2IP = 0;
            }
		}
		else if (interrupt_control == UART_INTERRUPTS_HIGH_PRI)
		{
            if (uart_index == UART1_INDEX) {
                IPR1bits.RC1IP = 1;
                IPR1bits.TX1IP = 1;
            } else if (uart_index == UART2_INDEX) {
                IPR3bits.RC2IP = 1;
                IPR3bits.TX2IP = 1;
            }
		}

		// Enable Receive/Transmit Interrupts
		PIE1bits.RC1IE = 1;
		//PIE1bits.TX1IE = 1;

		// Enable Peripheral Device Interrupts
		INTCONbits.PEIE = 1;

		// Enable Global Interrupts
		INTCONbits.GIE = 1;
	}
	// Else, just disable receive and transmit interrupts
	else
	{
		// Disable Interrupts
        if (uart_index == UART1_INDEX) {
            PIE1bits.RC1IE = 0;
            PIE1bits.TX1IE = 0;
        } else if (uart_index == UART2_INDEX) {
            PIE3bits.RC2IE = 0;
            PIE3bits.TX2IE = 0;
        }

        EnableTransmitter(uart_index);
	}

}

void EnableTransmitter(unsigned char uart_index) {
    if (uart_index == UART1_INDEX) {
        TXSTA1bits.TXEN1 = 1;
    } else if (uart_index == UART2_INDEX) {
        TXSTA2bits.TXEN2 = 1;
    }
}

void DisableTransmitter(unsigned char uart_index) {
    if (uart_index == UART1_INDEX) {
        TXSTA1bits.TXEN1 = 0;
    } else if (uart_index == UART2_INDEX) {
        TXSTA2bits.TXEN2 = 0;
    }
}

unsigned char IsTransmitterEnabled(unsigned char uart_index) {
    if (uart_index == UART1_INDEX) {
        if (TXSTA1bits.TXEN1) {
            return TRUE;
        }
    } else if (uart_index == UART2_INDEX) {
        if (TXSTA2bits.TXEN2) {
            return TRUE;
        }
    }

    return FALSE;
}

unsigned char IsTransmitterReady(unsigned char uart_index) {
    if (IsTransmitterEnabled(uart_index)) {
        return _GetTxInterruptFlag(uart_index);
    }
    return FALSE;
}

void PutChar9 (unsigned char uart_index, unsigned int data) {
    unsigned char delay;

    // Enable the transceiver
    EnableTransceiverTX(uart_index);

    // Transceiver turn around time
    delay = 0;
    while (delay < 100) { delay++; }
    
    // Ensure we are not currently waiting for a byte to be sent
    while (!_GetTxInterruptFlag(uart_index));
    
    // If word is 9 bits, configure TX9D bit, and must be done first.
    TXSTA1bits.TX9D = 0;
    if ((data & 0x0100) == 0x0100) {
        if (uart_index == UART1_INDEX) {
            TXSTA1bits.TX9D = 1;
        } else if (uart_index == UART2_INDEX) {
            TXSTA2bits.TX9D = 1;
        }
    }

    if (uart_index == UART1_INDEX) {
        TXREG1 = (unsigned char)(data & 0x00FF);
        // Wait for byte to be shifted (datasheet requires > 1 instruction)
        while (!TXSTA1bits.TRMT);
        while (!TXSTA1bits.TRMT);
    } else if (uart_index == UART2_INDEX) {
        TXREG2 = (unsigned char)(data & 0x00FF);
        // Wait for byte to be shifted (datasheet requires > 1 instruction)
        while (!TXSTA2bits.TRMT);
        while (!TXSTA2bits.TRMT);
    }
    
    // Transceiver turn around time
    delay = 0;
    while (delay < 100) { delay++; }

    DisableTransceiverTX(uart_index);
}

void PutChar9Default (unsigned int data) {
    PutChar9(UART_INDEX_DEFAULT, data);
}

void EnableTransceiverTX(unsigned char uart_index) {
    if (uart_index == UART1_INDEX) {
        PIN_UART1_TX_ENABLE_TRIS = 0;
        PIN_UART1_TX_ENABLE_LATCH = UART1_TX_LATCH_ACTIVE;
    }
    //else if (uart_index == UART2_INDEX) {
        // UART2 is pure TTL, and has no external transceiver
    //}
}

void DisableTransceiverTX(unsigned char uart_index) {
    if (uart_index == UART1_INDEX) {
        PIN_UART1_TX_ENABLE_TRIS = 0;
        PIN_UART1_TX_ENABLE_LATCH = UART1_TX_LATCH_INACTIVE;    
    }
    //else if (uart_index == UART2_INDEX) {
        // UART2 is pure TTL, and has no external transceiver
    //}
}

void EnableTransceiverRX(unsigned char uart_index) {
    if (uart_index == UART1_INDEX) {
        PIN_UART1_RX_ENABLE_TRIS = 0;
        PIN_UART1_RX_ENABLE_LATCH = UART1_RX_LATCH_ACTIVE;
    }
    //else if (uart_index == UART2_INDEX) {
        // UART2 is pure TTL, and has no external transceiver
    //}
}

void DisableTransceiverRX(unsigned char uart_index) {
    if (uart_index == UART1_INDEX) {
        PIN_UART1_TX_ENABLE_TRIS = 0;
        PIN_UART1_TX_ENABLE_LATCH = UART1_RX_LATCH_INACTIVE;    
    }
    //else if (uart_index == UART2_INDEX) {
        // UART2 is pure TTL, and has no external transceiver
    //}
}

unsigned char _GetTxInterruptFlag(unsigned char uart_index) {
    if (uart_index == UART1_INDEX) {
        if (PIR1bits.TX1IF)
            return TRUE;
    } else if (uart_index == UART2_INDEX) {
        if (PIR3bits.TX2IF)
            return TRUE;
    }
    return FALSE;
}

unsigned char _GetRxInterruptFlag(unsigned char uart_index) {
    if (uart_index == UART1_INDEX) {
        if (PIR1bits.RC1IF)
            return TRUE;
    } else if (uart_index == UART1_INDEX) {
        if (PIR3bits.RC2IF)
            return TRUE;
    }
    return FALSE;
}

unsigned char IsRxDataAvailable(unsigned char uart_index) {
    if (uart_index == UART1_INDEX) {
        if (RCSTA1bits.SPEN1 && PIR1bits.RC1IF)
            return TRUE;
    } else if (uart_index == UART2_INDEX) {
        if (RCSTA2bits.SPEN2 && PIR3bits.RC2IF)
            return TRUE;
    }
    return FALSE;
}

unsigned int GetChar9(unsigned char uart_index) {
    if (IsRxDataAvailable(uart_index)) {
        unsigned int data = 0x0000;

        if (uart_index == UART1_INDEX) {        
            // Framing error clears itself on read of RCREG
            if (RCSTA1bits.FERR) {
                data = data | UART_FAULT_FRAMING_ERROR;
            }
            
            // Get buffer contents
            if (RCSTA1bits.RX9D == 1) {
                data = data | 0x0100;
            }
            data += RCREG1;

            // Overruns must be cleared by cycling CREN bit
            if (RCSTA1bits.OERR) {
                data = data | UART_FAULT_OVERRUN_ERROR;
                RCSTA1bits.CREN = 0;
                RCSTA1bits.CREN = 1;
            }

            return data;
            
        } else if (uart_index == UART2_INDEX) {
            // Framing error clears itself on read of RCREG
            if (RCSTA2bits.FERR) {
                data = data | UART_FAULT_FRAMING_ERROR;
            }
            
            // Get buffer contents
            if (RCSTA2bits.RX9D == 1) {
                data = data | 0x0100;
            }
            data += RCREG2;

            // Overruns must be cleared by cycling CREN bit
            if (RCSTA2bits.OERR) {
                data = data | UART_FAULT_OVERRUN_ERROR;
                RCSTA2bits.CREN = 0;
                RCSTA2bits.CREN = 1;
            }
        }
    }
    return UART_FAULT_NO_DATA_AVAILABLE;
}

unsigned int GetChar9Default() {
    return GetChar9(UART_INDEX_DEFAULT);
}

