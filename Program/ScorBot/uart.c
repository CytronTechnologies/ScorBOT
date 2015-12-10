
//	include
//=============================================================================================
#include "uart.h"


//	functions
//=============================================================================================
void uart_init(void)
{
	//tris
	uart_tx_tris=0;
	uart_rx_tris=1;
	
	//initial value
	uart_tx=1;
	
	//delay
	timer=0;
	while(timer<(timer_freq/1000));		//delay 1ms
}

void uart_send(unsigned char data)
{
	unsigned char i;
	
	uart_tx=0;							//start bit
	timer=0;
	while(timer<(timer_freq/baudrate));
	
	for(i=1;i>0;i=i<<1)					//data transmission
	{
		if((data&i)==0)uart_tx=0;
		else uart_tx=1;
		timer=0;
		while(timer<(timer_freq/baudrate));		
	}
	
	uart_tx=1;							//stop bit
	timer=0;
	while(timer<(timer_freq/baudrate));
}

unsigned char uart_receive(void)
{
	unsigned char i,data=0;
	
	while(uart_rx==1);					//wait start bit
	timer=0;
	while(timer<(timer_freq/baudrate));
	
	timer=0;							//wait half bit to read at the center 
	while(timer<(timer_freq/baudrate/2));
	
	for(i=1;i>0;i=i<<1)					//receiving data
	{
		if(uart_rx==1)data=data|i;
		timer=0;
		while(timer<(timer_freq/baudrate));					
	}
	
	//timer=0;							//wait half bit and exit
	//while(timer<(timer_freq/baudrate));
	
	return data;
}