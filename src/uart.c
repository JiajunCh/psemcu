#include "uart.h"
#include "string.h"
#include "pse.h"

uint8_t code CMD_Write[] = "write 1=";
uint8_t code CMD_Read[] = "read 1=";
uint8_t code CMD_Ver[] = "ver\n";

uint16_t xdata sdet = 0;

static const uint8_t UART1_OVTIME = 100;	//usrt rx over time

// global variable
static uint8_t xdata TX1_Read=0;		//tx read pointer 
static uint8_t xdata TX1_Write=0;	//tx write pointer
static uint8_t xdata TX1_Buffer[TX1_LENGTH];	//tx buffer
volatile bit B_TX1_Busy;	//sending busy flag
uint8_t xdata RX1_Timer;
static volatile uint8_t xdata RX1_Write=0;	//tx write pointer
volatile uint8_t xdata RX1_Buffer[RX1_LENGTH];	//rx buffer
volatile bit B_RX1_OK;	//receive complete flag

// private function
static void SetTimer2Baudrate(uint16_t dat);

//========================================================================
// function: 		SetTimer2Baudrate(u16 dat)
// description: timer2 for buadrate generator
// parameter: 	dat:Timer2 reload
// return:	 		void
// version: 		V1.0, 2018-1-11
//========================================================================
static void SetTimer2Baudrate(uint16_t dat){
	AUXR &= ~(1<<4);	//Timer stop
	AUXR &= ~(1<<3);	//Timer2 set As Timer
	AUXR |=  (1<<2);	//Timer2 set as 1T mode
	TH2 = dat / 256;
	TL2 = dat % 256;
	IE2  &= ~(1<<2);	//disable interrupt_
	AUXR |=  (1<<4);	//Timer run enable
}

//========================================================================
// function:		UART1_config
// description:	init config UART1
// parameter: 	void
// return: 			void
// version: 		V1.0, 2018-1-11
//========================================================================
void	UART1_config(void){
	/*********** timer2 for baudrate *****************/
	AUXR |= 0x01;		//S1 BRT Use Timer2;
	SetTimer2Baudrate(65536UL - (MAIN_Fosc>>2) / Baudrate1);

	/*********** timer1 for baudrate *****************/
/*	
	TR1 = 0;
	AUXR &= ~0x01;		//S1 BRT Use Timer1;
	AUXR |=  (1<<6);	//Timer1 set as 1T mode
	TMOD &= ~(1<<6);	//Timer1 set As Timer
	TMOD &= ~0x30;		//Timer1_16bitAutoReload;
	TH1 = (u8)((65536UL - (MAIN_Fosc / 4) / Baudrate1) / 256);
	TL1 = (u8)((65536UL - (MAIN_Fosc / 4) / Baudrate1) % 256);
	ET1 = 0;	//disable interrupt_
	INT_CLKO &= ~0x02;	//disable output clock
	TR1  = 1;
*/

	SCON = (SCON & 0x3f) | 0x40;	//mode 0x00: sync, 0x40: 8bit_variableBuad, 0x80: 9bit_fixedbuad  0xc0: 9bit_variableBuad
	PS  = 1;	//interrupt_ priority
	ES  = 1;	//enable interrupt_
	REN = 1;	//enable receive
	P_SW1 &= 0x3f;
	P_SW1 |= 0x00;	//UART1 switch to, 0x00: P3.0_P3.1, 0x40: P3.6_P3.7, 0x80: P1.6_P1.7 (must use interal clock)
	AUXR2 |=  (1<<4);	//RXDLNTXD,1:link,0:normal

	B_TX1_Busy = 0;
	TX1_Read   = 0;
	TX1_Write  = 0;
}

//========================================================================
// function:		uart1_handle
// description:	uart1 interrupt_ service handle
// parameter: 	void
// return: 			void
// version: 		V1.0, 2018-1-11
//========================================================================
void uart1_handle (void) interrupt 4{
	while(RI){	//"while" instead of "if" to use "break" later
		static uint8_t temp = 0;
		RI = 0;
		temp = SBUF;
		if(B_RX1_OK || '\r'==temp) break;
		RX1_Buffer[RX1_Write++] = temp;
		if('\n'==temp || (RX1_LENGTH-1)==RX1_Write){
			B_RX1_OK = 1;
			RX1_Buffer[RX1_Write] = '\0';
			RX1_Write = 0;
		}
	}

	if(TI){
		TI = 0;
		B_TX1_Busy = 0;
	}
}

//========================================================================
// function: 		UART1_RxProcess
// description: check cmd to process
// parameter: 	void
// return:	 		void
// version: 		V1.0, 2018-1-11
//========================================================================
void UART1_RxProcess(void){
	if(!B_RX1_OK) return;
	if(0 == strncmp(RX1_Buffer, CMD_Ver, strlen(CMD_Ver))){
		RX1_Write=0;
		B_RX1_OK=0;
		PrintString1("HW:V1.0\nSW:V1.0\n");
		PrintString1(__DATE__"\n");
		PrintString1(__TIME__"\n");
	}
	else if(0 == strncmp(RX1_Buffer, CMD_Write, strlen(CMD_Write))){
		RX1_Write=0;
		B_RX1_OK=0;
	}
	else if(0 == strncmp(RX1_Buffer, CMD_Read, strlen(CMD_Read))){
		RX1_Write=0;
		B_RX1_OK=0;
	}
}

//========================================================================
// function: 		UART1_TxProcess
// description: check data_ to send in infinite loop
// parameter: 	void
// return:	 		void
// version: 		V1.0, 2018-1-11
//========================================================================
void UART1_TxProcess(void){
	if((TX1_Read != TX1_Write) && (!B_TX1_Busy)){	//have data_ && uart idle
		SBUF = TX1_Buffer[TX1_Read];
		B_TX1_Busy = 1;
		if(++TX1_Read >= TX1_LENGTH)
			TX1_Read = 0;
	}
}

//========================================================================
// function: 		TX1_write2buff(uint8_t dat)
// description: write data_ in buffer to wait send
// parameter: 	byte data_
// return:	 		void
// version: 		V1.0, 2018-1-11
//========================================================================
void TX1_write2buff(uint8_t dat){
	TX1_Buffer[TX1_Write] = dat;
	if(++TX1_Write >= TX1_LENGTH)
		TX1_Write = 0;
}

//========================================================================
// function: 		PrintString1(uint8_t *str)
// description: write a string to buffer
// parameter: 	string must be end of '\0'
// return:	 		void
// version: 		V1.0, 2018-1-11
//========================================================================
void PrintString1(uint8_t* str){
	for (; *str != 0;	str++)
		TX1_write2buff(*str);	//end of '\0'
}

//========================================================================
// function:		uart1_handle
// description:	uart1 interrupt_ service handle
// parameter: 	void
// return: 			void
// version: 		V1.0, 2018-1-11
//========================================================================
void timeEv_UARTRx1Rst(uint8_t tick){
	static uint16_t uartrst_tick = 0;
	if(0 == RX1_Write){
		uartrst_tick = 0;
		return;
	}
	uartrst_tick += tick;
	if(uartrst_tick > UART1_OVTIME){
		uartrst_tick = 0;
		RX1_Write = 0;
	}
}
