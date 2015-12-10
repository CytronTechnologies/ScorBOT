
//	include
//==========================================================================
#include <pic.h> 


//	configuration
//==========================================================================
__CONFIG ( 0x3F32 );

//	define
//==========================================================================
#define	rs			RB1
#define	e			RB2
#define	lcd_data	PORTD
#define sw1			RB0
#define sw2			RB5
#define skreset		RB4
#define motor1a		RE1
#define motor1b		RE2
#define motor1spd	RC2
#define motor2a		RC3
#define motor2b		RC4
#define motor2spd	RC1
#define led1		RA0
#define led2		RA1
#define led3		RA2
#define led4		RA3
#define led5		RA5
#define led6		RE0
#define led			RC0

//selfdefine UART
#define uart_rx			RB3
#define uart_tx			RC5
#define uart_rx_tris	TRISB3
#define uart_tx_tris	TRISC5

//define timer
#define timer			TMR1L
#define timer_freq		20000000/16
#define baudrate		9600

//skps protocol
#define	p_select		0
#define p_joyl			1
#define p_joyr			2
#define p_start			3
#define p_up			4
#define p_right			5
#define p_down			6
#define p_left			7
#define	p_l2			8
#define	p_r2			9
#define p_l1			10
#define p_r1			11
#define p_triangle		12
#define p_circle		13
#define p_cross			14
#define	p_square		15
#define p_joy_lx		16
#define	p_joy_ly		17
#define p_joy_rx		18
#define p_joy_ry		19
#define p_joy_lu		20
#define p_joy_ld		21
#define p_joy_ll		22
#define p_joy_lr		23
#define p_joy_ru		24
#define p_joy_rd		25
#define p_joy_rl		26
#define p_joy_rr		27

#define	p_con_status	28
#define p_motor1		29
#define p_motor2		30

//servo define
#define head			0x41
#define l1				0x42
#define l2				0x43
#define l3				0x44
#define r1				0x47
#define r2				0x48
#define r3				0x49
#define c1				0x4A
#define tail1			0x4C
#define tail2			0x4D
#define gripper			0x4E

//	global variable
//==========================================================================
unsigned char mob_temp,servo_temp,u_change=1,uart_datah,uart_datal,uart_speed,servo_var=0,uart_select;
unsigned char dataf=130,datar=130,datatr=130,datatl=130;
unsigned int head_val=319,l1_val=866,l2_val=1076,l3_val=1017,r1_val=84,r2_val=0;
unsigned int r3_val=300,c1_val=689,tail1_val=731,tail2_val=0,gripper_val=0;


//	function prototype
//==========================================================================
//	LCD	functions
//==========================================================================
void delay(unsigned long data)
{
	for( ;data>0;data-=1);
}

void e_pulse(void)
{
	e=1;
	delay(50);
	e=0;
	delay(50);
}

void send_config(unsigned char data)
{
	rs=0;								//clear rs into config mode 
	lcd_data=data;
	delay(50);
	e_pulse();
}

void send_char(unsigned char data)
{
	rs=1;								//set rs into write mode
	lcd_data=data;						 
	delay(50);
	e_pulse();
}

void lcd_goto(unsigned char data)
{
 	if(data<16)
	{
	 	send_config(0x80+data);
	}
	else
	{
	 	data=data-20;
		send_config(0xc0+data);
	}
}

void lcd_clr(void)
{
 	send_config(0x01);
	delay(50);	
}

void send_string(const char *s)
{          
	unsigned char i=0;
  	while (s && *s)send_char (*s++);

}

void send_dec(unsigned long data,unsigned char num_dig)
{
	
	if(num_dig>=10)
	{
		data=data%10000000000;
		send_char(data/1000000000+0x30);
	}	
	if(num_dig>=9)
	{
		data=data%1000000000;
		send_char(data/100000000+0x30);
	}	
	if(num_dig>=8)
	{
		data=data%100000000;
		send_char(data/10000000+0x30);
	}	
	if(num_dig>=7)
	{
		data=data%10000000;
		send_char(data/1000000+0x30);
	}	
	if(num_dig>=6)
	{
		data=data%1000000;
		send_char(data/100000+0x30);
	}	
	if(num_dig>=5)
	{
		data=data%100000;
		send_char(data/10000+0x30);
	}	
	if(num_dig>=4)
	{
		data=data%10000;
		send_char(data/1000+0x30);
	}
	if(num_dig>=3)
	{
		data=data%1000;
		send_char(data/100+0x30);
	}
	if(num_dig>=2)
	{
		data=data%100;
		send_char(data/10+0x30);
	}
	if(num_dig>=1)
	{
		data=data%10;
		send_char(data+0x30);
	}
}

// Additional UART 
//============================================================
void uart_init(void)
{
	//tris
	uart_tx_tris=0;
	uart_rx_tris=1;
	
	//initial value
	uart_tx=1;
	
	//delay
	timer=0;

	delay(5000);
	
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
	
	return data;
}

//servo control
//======================================================================
void send_cmd(unsigned char num, unsigned int data, unsigned char ramp)		//send 4 bytes of command to control servo's position and speed
{
	unsigned char higher_byte=0, lower_byte=0;
	
	//position value from 0-1463 are greater than a byte
	//so needs two bytes to send
	higher_byte=(data>>6)&0x003f;	//higher byte = 0b00xxxxxx
	lower_byte=data&0x003f;			//lower byte  = 0b00xxxxxx

	
			uart_send(num);								//First byte is the servo channel 0x41-0x60
			uart_send(higher_byte);						//second byte is the higher byte of position 0b00xxxxxx
			uart_send(lower_byte);						//third byte is the lower byte of position 0b00xxxxxx
			uart_send(ramp);							//fourth byte is the speed value from 0-63 

}


//development use function
//===============================================================
unsigned char error_count_rec_infwait(void)	
{
	unsigned char a=0,b=0;
	unsigned long error_count=0;

	while (RCIF == 0);	
	a = RCREG;
	RCIF=0;

	return a;
}

unsigned char error_count_rec(void)		
{
	unsigned char a=0,b=0;
	unsigned long error_count=0;

	while (RCIF == 0)
	{
		error_count+=1;
		if(error_count>=1700000)
		{
			b = RCREG;
			RCIF=0;
			a=255;
			goto error_end;
		}
	}
	
	a = RCREG;
	RCIF=0;

 	error_end:

	error_count=0;
	return a;
}

unsigned char ok_receive(void)
{
	unsigned char error_occur=0;

	if(error_count_rec()!='O')
	{
		error_occur=1;
		goto end_ok;	
	}
	if(error_count_rec()!='K')
	{
		error_occur=1;
		goto end_ok;	
	}
	if(error_count_rec()!=0x0D)
	{
		error_occur=1;
		goto end_ok;	
	}

	end_ok:	
	return error_occur;
}

unsigned char enter_receive(unsigned char num)
{
	unsigned char error_occur=0,temp=0,count=0,q=0;

	for(q=0;q<num;q++)
	{
	 	wait_enter_again:
	
		temp=error_count_rec();
		count+=1;
		if(temp!=0x0D)
		{
			if(temp==255)
			{
				error_occur=1;
				goto end_enter;
			}
			goto wait_enter_again;	
		}
		count=0;
	}

 end_enter:
	count=0;
	return error_occur;
}


unsigned char enter_receive_inf(unsigned char num)
{
	unsigned char error_occur=0,temp=0,count=0,q=0;

	for(q=0;q<num;q++)
	{
	 	wait_enter_again:
	
			temp=error_count_rec_infwait();
			count+=1;
			if(temp!=0x0D)
			{
				if(temp==255)
				{
					error_occur=1;
					goto end_enter;
				}
				goto wait_enter_again;	
			}
			count=0;
	}

 end_enter:
	count=0;
	return error_occur;
}

//mobility function
//=========================================================================
void forward(unsigned char speed)
{
	CCPR1L=speed;
	CCPR2L=speed;

	motor1a	=0;
	motor1b	=1;
	motor2a=0;
	motor2b=1;	
		
}

void reverse(unsigned char speed)
{
	CCPR1L=speed;
	CCPR2L=speed;

	motor1a	=1;
	motor1b	=0;
	motor2a=1;
	motor2b=0;	
}

void fturn(unsigned char l_speed, unsigned char r_speed)
{
	CCPR1L=l_speed;
	CCPR2L=r_speed;

	motor1a	=0;
	motor1b	=1;
	motor2a=0;
	motor2b=1;	
}

void rturn(unsigned char l_speed, unsigned char r_speed)
{
	CCPR1L=l_speed;
	CCPR2L=r_speed;

	motor1a	=1;
	motor1b	=0;
	motor2a=1;
	motor2b=0;	
}

void rotatel(unsigned char speed)
{
	CCPR1L=speed;
	CCPR2L=speed;

	motor1a	=0;
	motor1b	=1;
	motor2a=1;
	motor2b=0;	
}

void rotater(unsigned char speed)
{
	CCPR1L=speed;
	CCPR2L=speed;

	motor1a	=1;
	motor1b	=0;
	motor2a=0;
	motor2b=1;	
}


void stop(void)
{
	CCPR1L=0;
	CCPR2L=0;

	motor1a	=0;
	motor1b	=0;
	motor2a=0;
	motor2b=0;	
}

void brake(void)
{
	CCPR1L=255;
	CCPR2L=255;

	motor1a	=1;
	motor1b	=1;
	motor2a=1;
	motor2b=1;	
}

void comp_control_mobility(unsigned char data)
{	
	if(data=='8')		//forward
	{
		if(dataf<230) dataf+=5;
		else dataf=230;
		datar=130;
		datatr=130;
		datatl=130;
		forward(dataf);
		
	}
	if(data=='2')		//reverse
	{		
		dataf=130;
		if(datar<230) datar+=5;
		else datar=230;
		datatr=130;
		datatl=130;
		reverse(datar);
	}
	if(data=='6')		//rotate right
	{
		dataf=130;
		datar=130;
		if(datatr<230) datatr+=5;
		else datatr=230;
		datatl=130;
		rotater(datatr);
	}
	if(data=='4')		//rotate left
	{
		dataf=130;
		datar=130;
		datatr=130;
		if(datatl<230) datatl+=5;
		else datatl=230;
		rotatel(datatl);
	}
	if(data=='9')		//turn right
	{
		dataf=130;
		datar=130;
		if(datatr<230) datatr+=5;
		else datatr=230;
		datatl=130;
		fturn(datatr,0);
	}
	if(data=='7')		//turn left
	{
		dataf=130;
		datar=130;
		datatr=130;
		if(datatl<230) datatl+=5;
		else datatl=230;
		fturn(0,datatl);
	}
	if(data=='5')		//brake
	{		
		dataf=130;
		datar=130;
		datatr=130;
		datatl=130;
		brake();
	}

	

}

// SKPS function
//=======================================================================
void skps_uart_send(unsigned char data)	//function to send out a byte via uart
{	
	while(TXIF==0);					//wait for previous data to finish send out
	TXREG=data;						//send new data
}

unsigned char skps_uart_rec(void)		//function to wait for a byte receive from uart
{
	unsigned char temp;
	while(RCIF==0);					//wait for data to received
	temp=RCREG;					
	return temp;					//return the received data
}

unsigned char skps(unsigned char data)			//function to read button and joystick 
{												//information on ps controller
	skps_uart_send(data);
	return skps_uart_rec();
}

unsigned char skps_vibrate(unsigned char motor, unsigned char value)
{												//function to control the vibrator motor 
	skps_uart_send(motor);							//on ps controller
	skps_uart_send(value);	
}

//	Interrupt function
//==========================================================================
static void interrupt isr (void)
{
	unsigned char test=0;	

	if(RCIF==1)
	{
		RCIF=0;		
		test=RCREG;

		//button control for servo
		if(test=='p')
		{
			if(gripper_val>30)gripper_val-=15;
		}
		
		else if(test==';')
		{
			if(gripper_val<1433)gripper_val+=15;
		}

		else if(test=='y')
		{
			if(r3_val>30)r3_val-=15;
		}

		else if(test=='h')
		{
			if(r3_val<1433)r3_val+=15;
		}
		
		else if(test=='t')
		{
			if(r2_val>30)r2_val-=15;
		}

		else if(test=='g')
		{
			if(r2_val<1433)r2_val+=15;
		}		

		else if(test=='r')
		{
			if(r1_val<1433)r1_val+=15;
		}

		else if(test=='f')
		{
			if(r1_val>30)r1_val-=15;	
		}

		else if(test=='z')
		{
			head_val=319,l1_val=866,l2_val=1076,l3_val=1017,r1_val=84,r2_val=0;
			r3_val=300,c1_val=689,tail1_val=731,tail2_val=0,gripper_val=0;
		}
	
		else if(test=='e')
		{
			if(l3_val<1433)l3_val+=15;
		}
	
		else if(test=='d')
		{
			if(l3_val>30)l3_val-=15;
		}

		else if(test=='w')
		{
			if(l2_val>30)l2_val-=15;
		}

		else if(test=='s')
		{
			if(l2_val<1433)l2_val+=15;
		}
		
		else if(test=='q')
		{
			if(l1_val<1433)l1_val-=15;	
		}
		else if(test=='a')
		{
			if(l1_val>30)l1_val+=15;	
		}
			
		else if(test=='i')
		{
			if(tail1_val<1433)tail1_val+=15;
		}
	
		else if(test=='k')
		{
			if(tail1_val>30)tail1_val-=15;
		}

		else if(test=='o')
		{
			if(tail2_val>30)tail2_val-=15;
		}
	
		else if(test=='l')
		{
			if(tail2_val<1433)tail2_val+=15;
		}	
	
		else if(test=='u')
		{
			if(c1_val<1433)c1_val+=15;
		}
		else if(test=='j')
		{
			if(c1_val>30)c1_val-=15;
		}
		else if(test=='x')
		{
			head_val=782,l1_val=42,l2_val=605,l3_val=387,r1_val=84,r2_val=0;
			r3_val=462,c1_val=126,tail1_val=731,tail2_val=454,gripper_val=731;
		}
		else if(test=='c')
		{
			head_val=319;l1_val=84;l2_val=1076;l3_val=395;r1_val=899;r2_val=0;
			r3_val=958;c1_val=689;tail1_val=731;tail2_val=454;gripper_val=731;
		}
		else if(test=='v')
		{
			head_val=0,l1_val=866,l2_val=1076,l3_val=1017,r1_val=916,r2_val=345;
			r3_val=958,c1_val=1126,tail1_val=731,tail2_val=454,gripper_val=731;
		}
		else if(test=='b')
		{
			head_val=950,l1_val=866,l2_val=277,l3_val=395,r1_val=84,r2_val=0;
			r3_val=462,c1_val=689,tail1_val=731,tail2_val=0,gripper_val=0;
		}
		else if(test=='n')
		{
			head_val=319,l1_val=76,l2_val=1067,l3_val=1126,r1_val=930,r2_val=0;
			r3_val=235,c1_val=689,tail1_val=731,tail2_val=303,gripper_val=840;
		}
		else if(test=='m')
		{
			head_val=0,l1_val=866,l2_val=1076,l3_val=1017,r1_val=126,r2_val=656;
			r3_val=933,c1_val=689,tail1_val=731,tail2_val=0,gripper_val=0;
		}
		else
		{
			mob_temp=test;
		}

		u_change=1;
	}
}

//modes
//=========================================================================
void mode1(void)
{
	while(sw2==0);

	GIE=0;
	PEIE=0;
	RCIE=0;
	RCIF=0;
	
	SPBRG = 10;						//set baud rate to 115200 for 20Mhz
	BRGH = 1;							//baud rate high speed option
	TXEN = 1;							//enable transmission
	CREN = 1;							//enable reception
	SPEN = 1;							//enable serial port
	
	lcd_clr();
	delay(1000);
	lcd_goto(0);							//initial display
	send_string("Bluetooth init");
	skreset=0;
	delay(5000);
	skreset=1;
	enter_receive_inf(2);

	lcd_clr();
	delay(1000);
	lcd_goto(0);							//initial display
	send_string("Connection ");
	lcd_goto(20);							//initial display
	send_string("waiting");
	enter_receive_inf(2);

	GIE=1;
	PEIE=1;
	RCIE=1;
	RCIF=0;

	lcd_clr();
	delay(1000);
	lcd_goto(0);
	send_string("  MODE1=SKKCA");
	lcd_goto(20);
	send_string("   Running");
	while(sw1)
	{
		if(sw2==0)
		{
			while(sw2==0);
			TXREG='a';
			while (TRMT==0);
			TRMT=0;			
		}	

		if(u_change==1)
		{
			comp_control_mobility(mob_temp);
			send_cmd(0x41,head_val,60);	
			send_cmd(0x42,l1_val,50);	
			send_cmd(0x43,l2_val,50);	
			send_cmd(0x44,l3_val,50);	
			send_cmd(0x47,r1_val,50);	
			send_cmd(0x48,r2_val,50);	
			send_cmd(0x49,r3_val,50);	
			send_cmd(0x4a,c1_val,0);	
			send_cmd(0x4c,tail1_val,20);	
			send_cmd(0x4d,tail2_val,20);	
			send_cmd(0x4e,gripper_val,0);
			u_change=0;	
		}

		
	}
	mob_temp=0;
	while(sw1==0);
}

void mode2(void)
{
	while(sw2==0);	

	SPBRG = 129;						//set baud rate to 115200 for 20Mhz
	BRGH = 1;							//baud rate high speed option
	TXEN = 1;							//enable transmission
	CREN = 1;							//enable reception
	SPEN = 1;							//enable serial port

	GIE=1;
	PEIE=1;
	RCIE=1;
	RCIF=0;
	
	skreset=1;
	lcd_clr();
	delay(1000);
	lcd_goto(0);
	send_string("  MODE1=SKXBEE");
	lcd_goto(20);
	send_string("   Running");
	while(sw1)
	{
		if(sw2==0)
		{
			while(sw2==0);
			TXREG='a';
			while (TRMT==0);
			TRMT=0;			
		}	

		if(u_change==1)
		{
			comp_control_mobility(mob_temp);
			send_cmd(0x41,head_val,60);	
			send_cmd(0x42,l1_val,50);	
			send_cmd(0x43,l2_val,50);	
			send_cmd(0x44,l3_val,50);	
			send_cmd(0x47,r1_val,50);	
			send_cmd(0x48,r2_val,50);	
			send_cmd(0x49,r3_val,50);	
			send_cmd(0x4a,c1_val,0);	
			send_cmd(0x4c,tail1_val,20);	
			send_cmd(0x4d,tail2_val,20);	
			send_cmd(0x4e,gripper_val,0);
			u_change=0;	
		}

	}
	mob_temp=0;
	while(sw1==0);
}

void mode3(void)
{
	unsigned char lx, ry, change=1, gcount=0;
	unsigned int gval=193,gcount2=0;
	unsigned char up_v, down_v, left_v, right_v;
	unsigned char sup_v, sdown_v, sleft_v, sright_v, ps_mode=0,pselect_test=0;
	
	while(sw2==0);	
	led=0;
	SPBRG = 129;						//set baud rate to 115200 for 20Mhz
	BRGH = 1;							//baud rate high speed option
	TXEN = 1;							//enable transmission
	CREN = 1;							//enable reception
	SPEN = 1;							//enable serial port

	GIE=0;
	PEIE=0;
	RCIE=0;
	RCIF=0;
	
	skreset=1;
	lcd_clr();
	delay(1000);
	lcd_goto(0);
	send_string("   MODE1=SKPS");
	lcd_goto(20);
	send_string("    Running");
	while(sw1)
	{
		if(sw2==0)
		{
			while(sw2==0);
			TXREG='a';
			while (TRMT==0);
			TRMT=0;
		}	

		if(skps(p_select)==0)
		{
			while(skps(p_select)==0);
			ps_mode^=1;
			if(ps_mode==1)led=1;
			else led=0;
		}

		if(change==1)
		{
			send_cmd(0x41,head_val,60);	
			send_cmd(0x42,l1_val,50);	
			send_cmd(0x43,l2_val,50);	
			send_cmd(0x44,l3_val,50);	
			send_cmd(0x47,r1_val,50);	
			send_cmd(0x48,r2_val,50);	
			send_cmd(0x49,r3_val,50);	
			send_cmd(0x4a,c1_val,0);	
			send_cmd(0x4c,tail1_val,20);	
			send_cmd(0x4d,tail2_val,20);	
			send_cmd(0x4e,gripper_val,0);
			change=0;	
		}

		//read joy stick value process		
		up_v=skps(p_joy_ru);
		down_v=skps(p_joy_rd);
		left_v=skps(p_joy_rl);
		right_v=skps(p_joy_rr);	

		sup_v=skps(p_joy_lu);
		sdown_v=skps(p_joy_ld);
		sleft_v=skps(p_joy_ll);
		sright_v=skps(p_joy_lr);	

		//button control for servo
		if(skps(p_joyl)==0)
		{
			if(ps_mode==1)
			{
				change=1;
			}
			else
			{
				if(gripper_val>30)gripper_val-=30;
				change=1;
			}
		}
		
		if(skps(p_joyr)==0)
		{
			if(ps_mode==1)
			{
				change=1;
			}
			else
			{
				if(gripper_val<1433)gripper_val+=30;
				change=1;
			}
		}

		if(skps(p_triangle)==0)
		{
			if(ps_mode==1)
			{
				head_val=319;l1_val=84;l2_val=1076;l3_val=395;r1_val=899;r2_val=0;
				r3_val=958;c1_val=689;tail1_val=731;tail2_val=454;gripper_val=731;
				change=1;
			}
			else
			{
				if(r3_val>30)r3_val-=30;
				change=1;
			}
		}
		
		if(skps(p_square)==0)
		{
			if(ps_mode==1)
			{
				head_val=782,l1_val=42,l2_val=605,l3_val=387,r1_val=84,r2_val=0;
				r3_val=462,c1_val=126,tail1_val=731,tail2_val=454,gripper_val=731;
				change=1;
			}
			else
			{
				if(r2_val>30)r2_val-=30;
				change=1;
			}
		}

		if(skps(p_circle)==0)
		{
			if(ps_mode==1)
			{
				head_val=0,l1_val=866,l2_val=1076,l3_val=1017,r1_val=916,r2_val=345;
				r3_val=958,c1_val=1126,tail1_val=731,tail2_val=454,gripper_val=731;
				change=1;
			}
			else
			{
				if(r2_val<1433)r2_val+=30;
				change=1;
			}
		}

		if(skps(p_cross)==0)
		{
			if(ps_mode==1)
			{
				head_val=319,l1_val=866,l2_val=1076,l3_val=1017,r1_val=84,r2_val=0;
				r3_val=300,c1_val=689,tail1_val=731,tail2_val=0,gripper_val=0;
				change=1;
			}
			else
			{
				if(r3_val<1433)r3_val+=30;
				change=1;
			}
		}

		if(skps(p_start)==0)
		{
			head_val=319,l1_val=866,l2_val=1076,l3_val=1017,r1_val=84,r2_val=0;
			r3_val=300,c1_val=689,tail1_val=731,tail2_val=0,gripper_val=0;
			change=1;
		}
	
		if(skps(p_up)==0)
		{
			if(ps_mode==1)
			{
				change=1;
			}
			else
			{
				if(l3_val<1433)l3_val+=30;
				change=1;
			}
		}
	
		if(skps(p_down)==0)
		{
			if(ps_mode==1)
			{
				head_val=319,l1_val=76,l2_val=1067,l3_val=1126,r1_val=930,r2_val=0;
				r3_val=235,c1_val=689,tail1_val=731,tail2_val=303,gripper_val=840;
				change=1;
			}
			else
			{
				if(l3_val>30)l3_val-=30;
				change=1;
			}
		}

		if(skps(p_left)==0)
		{
			if(ps_mode==1)
			{
				head_val=950,l1_val=866,l2_val=277,l3_val=395,r1_val=84,r2_val=0;
				r3_val=462,c1_val=689,tail1_val=731,tail2_val=0,gripper_val=0;
				change=1;
			}
			else
			{
				if(l2_val>30)l2_val-=30;
				change=1;
			}
		}

		if(skps(p_right)==0)
		{
			if(ps_mode==1)
			{
				head_val=0,l1_val=866,l2_val=1076,l3_val=1017,r1_val=126,r2_val=656;
				r3_val=933,c1_val=689,tail1_val=731,tail2_val=0,gripper_val=0;
				change=1;
			}
			else
			{
				if(l2_val<1433)l2_val+=30;
				change=1;
			}
		}
			
	
		if(skps(p_l1)==0)
		{
			if(ps_mode==1)
			{
				while(skps(p_l1)==0);
				led2^=1;
				led3^=1;
			}
			else
			{
				if(tail1_val<1433)tail1_val+=30;
				change=1;
			}
		}
	
		if(skps(p_l2)==0)
		{
			if(ps_mode==1)
			{
				while(skps(p_l2)==0);
				led1^=1;
				led4^=1;
			}
			else
			{
				if(tail1_val>30)tail1_val-=30;
				change=1;
			}
		}

		if(skps(p_r1)==0)
		{
			if(ps_mode==1)
			{
				while(skps(p_r1)==0);
				led6^=1;
			}
			else
			{
				if(tail2_val>30)tail2_val-=30;
				change=1;
			}
		}
	
		if(skps(p_r2)==0)
		{
			if(ps_mode==1)
			{
				while(skps(p_r2)==0);
				led5^=1;
			}
			else
			{
				if(tail2_val<1433)tail2_val+=30;
				change=1;
			}
		}
	
	
		//analog control robot hand
		if(skps(p_joy_lu)>0)
		{
			if(skps(p_joy_ll)>0)
			{
				if(ps_mode==1)
				{
					change=1;
				}
				else
				{
					if(l1_val<1433)l1_val-=30;
					change=1;
				}	
			}
			else if(skps(p_joy_lr)>0)
			{
				if(ps_mode==1)
				{
					change=1;
				}
				else
				{
					if(r1_val<1433)r1_val+=30;
					change=1;
				}
			}
		}
		else if(skps(p_joy_ld)>0)
		{
			if(skps(p_joy_ll)>0)
			{
				if(ps_mode==1)
				{
					change=1;
				}
				else
				{
					if(l1_val>30)l1_val+=30;
					change=1;	
				}
			}
			else if(skps(p_joy_lr)>0)
			{
				if(ps_mode==1)
				{
					change=1;
				}
				else
				{
					if(r1_val>30)r1_val-=30;
					change=1;	
				}
			}
		}
		else
		{
			if(skps(p_joy_ll)>0)
			{
				if(ps_mode==1)
				{
					change=1;
				}
				else
				{
					if(c1_val<1433)c1_val+=20;
					change=1;
				}
			}
			else if(skps(p_joy_lr)>0)
			{
				if(ps_mode==1)
				{
					change=1;
				}
				else
				{
					if(c1_val>30)c1_val-=20;
					change=1;
				}
			}
		}
		
				
		//analog control for mobility	
		if(up_v>0)
		{
			if(left_v>0)
			{
				send_cmd(head,638,63);
				if(up_v>left_v)fturn((up_v+140),(up_v-left_v+140));
				else fturn((up_v+140),140);
				if(gcount==0) 
				{
					if(gcount2>7)
					{
						gcount=1;
						gcount2=0;
					}
					else gcount2+=1;
					send_cmd(gripper,150,0);
				}
				else 
				{
					if(gcount2>7)
					{
						gcount=0;
						gcount2=0;
					}
					else gcount2+=1;
					send_cmd(gripper,700,0);
				} 
			}
			else if(right_v>0)
			{
				send_cmd(head,0,63);
				if(up_v>right_v)fturn((up_v-right_v+140),(up_v+140));
				else fturn(140,(up_v+140));
				if(gcount==0) 
				{
					if(gcount2>7)
					{
						gcount=1;
						gcount2=0;
					}
					else gcount2+=1;
					send_cmd(gripper,150,0);
				}
				else 
				{
					if(gcount2>7)
					{
						gcount=0;
						gcount2=0;
					}
					else gcount2+=1;
					send_cmd(gripper,700,0);
				} 
			}
			else
			{
				send_cmd(head,319,63);
				fturn((up_v+140),(up_v+140));
				if(gcount==0) 
				{
					if(gcount2>7)
					{
						gcount=1;
						gcount2=0;
					}
					else gcount2+=1;
					send_cmd(gripper,150,0);
				}
				else 
				{
					if(gcount2>7)
					{
						gcount=0;
						gcount2=0;
					}
					else gcount2+=1;
					send_cmd(gripper,700,0);
				} 
			}
		}
		else if(down_v>0)
		{
			if(left_v>0)
			{
				send_cmd(head,0,63);
				if(down_v>left_v)rturn((down_v+140),(down_v-left_v+140));
				else rturn((down_v+140),140);		
			}
			else if(right_v>0)
			{
				send_cmd(head,638,63);
				if(down_v>right_v)rturn((down_v-right_v+140),(down_v+140));
				else rturn(140,(down_v+140));
			}
			else
			{
				send_cmd(head,319,63);
				rturn((down_v+140),(down_v+140));
			}
		}
		else if(left_v>0)
		{
			send_cmd(head,638,63);
			rotatel((left_v+120));
		}
		else if(right_v>0)
		{
			send_cmd(head,0,63);
			rotater((right_v+120));
		}
		else
		{
			send_cmd(head,319,63);
			brake();
		}
	
	}
	while(sw1==0);
}

//	main function
//==========================================================================
void main(void)
{
	unsigned char mode_var=0;
	unsigned long i=0;
			
	TRISA=0b11010000;				//clear bit 4&5 portA as output and set the rest as input 
	TRISB=0b00101001;				//set portB as output
	TRISD=0b00000000;				//set portD as output
	TRISC=0b11100000;				//set bit4-7 portC as input(connected to 4 row of keypad)
	TRISE=0b00000000;				//set portE as output

	T1CON=0b00100001;	

	//ADCON0=0b00000001;	
	ADCON1=0b00000110;

	send_config(0b00001001);				//clear display at lcd
	send_config(0b00000010);				//Lcd Return to home 
	send_config(0b00000110);				//entry mode-cursor increase 1
	send_config(0b00001100);				//diplay on, cursor off and cursor blink off
	send_config(0b00111000);				//function

	//pwm
	CCP1CON=0b00001100;						
	CCP2CON=0b00001100;
	PR2=255;
	T2CON=0b01111100;
	CCPR1L=0;
	CCPR2L=0;

	lcd_clr();
	delay(10000);
	led=0;
	led1=1;
	led2=1;
	led3=1;
	led4=1;
	led5=1;
	led6=1;
	uart_init();	
	send_cmd(0x41,head_val,0);	
	send_cmd(0x42,l1_val,0);	
	send_cmd(0x43,l2_val,0);	
	send_cmd(0x44,l3_val,0);	
	send_cmd(0x47,r1_val,0);	
	send_cmd(0x48,r2_val,0);	
	send_cmd(0x49,r3_val,0);	
	send_cmd(0x4a,c1_val,0);	
	send_cmd(0x4c,tail1_val,0);	
	send_cmd(0x4d,tail2_val,0);	
	send_cmd(0x4e,gripper_val,0);																		

	while(1)
	{	
		skreset=0;
		if(sw1==0)			//Mode select
		{	
			while(sw1==0);
			if(mode_var<3) mode_var+=1;
			else mode_var=0;
			lcd_clr();
			delay(1000);
		}
			
		if(mode_var==0)
		{
			lcd_goto(0);							//initial display
			send_string(" Wireless Base");
		}

		if(mode_var==1)
		{
			lcd_goto(0);							//initial display
			send_string("  Mode1=SKKCA");
			lcd_goto(20);							//initial display
			send_string("  sw2 to start");
			if(sw2==0) 
			{
				if(mode_var<3) mode_var+=1;
				else mode_var=0;
				mode1();
			}
		}

		if(mode_var==2)
		{
			lcd_goto(0);							//initial display
			send_string("  Mode2=SKXBEE");
			lcd_goto(20);							//initial display
			send_string("  sw2 to start");
			if(sw2==0) 
			{
				if(mode_var<3) mode_var+=1;
				else mode_var=0;
				mode2();
			}
		}

		if(mode_var==3)
		{
			lcd_goto(0);							//initial display
			send_string("  Mode3=SKPS");
			lcd_goto(20);							//initial display
			send_string("  sw2 to start");
			if(sw2==0) 
			{
				if(mode_var<3) mode_var+=1;
				else mode_var=0;
				mode3();
			}
		}

	}	
}

