//=============================================================================
//	UART
//=============================================================================

// UART TX PINS
#define PIN_UART1_TX_ENABLE_TRIS        TRISAbits.TRISA5
#define PIN_UART1_TX_ENABLE_LATCH       LATAbits.LATA5
#define UART1_TX_LATCH_ACTIVE           1
#define UART1_TX_LATCH_INACTIVE         0

#define PIN_UART2_TX_ENABLE_TRIS        TRISBbits.TRISB6
#define PIN_UART2_TX_ENABLE_LATCH       LATBbits.LATB6
#define UART2_TX_LATCH_ACTIVE           1
#define UART2_TX_LATCH_INACTIVE         0


// UART RX PINS
#define PIN_UART1_RX_ENABLE_TRIS        TRISAbits.TRISA4
#define PIN_UART1_RX_ENABLE_LATCH       LATAbits.LATA4
#define UART1_RX_LATCH_ACTIVE           0
#define UART1_RX_LATCH_INACTIVE         1

#define UART_INDEX_DEFAULT              1
#define UART1_INDEX                     1
#define UART2_INDEX                     2

// Baud Rates
#define UART_BAUD_115200				115200
#define UART_BAUD_57600					57600
#define UART_BAUD_19200					19200
#define UART_BAUD_9600					9600

// 8 or 9 bit mode
#define UART_8BIT_MODE					0
#define UART_9BIT_MODE					1

// Interrupt Control
#define UART_INTERRUPTS_DISABLED		0
#define UART_INTERRUPTS_LOW_PRI         1
#define UART_INTERRUPTS_HIGH_PRI		2

// Fault Codes (returned in upper bits of data word)
#define UART_FAULT_FRAMING_ERROR        0x0200
#define UART_FAULT_OVERRUN_ERROR        0x0400
#define UART_FAULT_NO_DATA_AVAILABLE    0x0800

void UART_Initialize (	unsigned char uart_index,
                        unsigned long baud, 
						unsigned char mode_9bit,
						unsigned char interrupt_control );
void PutChar9 (unsigned char uart_index, unsigned int data);
void PutChar9Default(unsigned int data);
void EnableTransmitter(unsigned char uart_index);
void DisableTransmitter(unsigned char uart_index);
unsigned char IsTransmitterEnabled(unsigned char uart_index);
unsigned char IsTransmitterReady(unsigned char uart_index);
void EnableTransceiverTX(unsigned char uart_index);
void DisableTransceiverTX(unsigned char uart_index);
void EnableTransceiverRX(unsigned char uart_index);
void DisableTransceiverRX(unsigned char uart_index);
unsigned char _GetTxInterruptFlag(unsigned char uart_index);
unsigned char _GetRxInterruptFlag(unsigned char uart_index);
unsigned char IsRxDataAvailable(unsigned char uart_index);
unsigned int GetChar9(unsigned char uart_index);
unsigned int GetChar9Default();


