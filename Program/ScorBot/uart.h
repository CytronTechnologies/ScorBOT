
//	include
//=============================================================================================
#include <pic.h>


//	definition
//=============================================================================================
//define pin for uart communication
#define uart_tx			RC6
#define uart_rx			RC7
#define uart_tx_tris	TRISC6
#define uart_rx_tris	TRISC7

//define timer
#define timer			TMR1L
#define timer_freq		1250000
#define baudrate		115200

//	function prototype
//=============================================================================================
void uart_init(void);
void uart_send(unsigned char data);
unsigned char uart_receive(void);
