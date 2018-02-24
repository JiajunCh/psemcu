#include "timer.h"

volatile uint8_t TIMER1_TICK = 0;

//========================================================================
// function:		TIMER1_config
// description:	start timer1
// parameter: 	void
// return: 			void
// version: 		V1.0, 2018-1-11
//========================================================================
void TIMER1_config(void){
	AUXR &= ~(1<<6);	////Timer1 set as 12T mode
	TMOD = 0x00;
	TL1 = 0x30;	//65536- 24m/12/1k
	TH1 = 0xF8;	//2k_tick/ms
	TR1 = 1;		//start timer1
	ET1 = 1;		//enable interuupt_
}

//========================================================================
// function:		timer1_handle
// description:	timer1 interrupt_ service handle
// parameter: 	void
// return: 			void
// version: 		V1.0, 2018-1-11
//========================================================================
void timer1_handle (void) interrupt 3{
	TIMER1_TICK++;
}

//========================================================================
// function:		get_timer1tick
// description:	return timer1 tick
// parameter: 	void
// return: 			void
// version: 		V1.0, 2018-1-11
//========================================================================
uint8_t get_timer1tick(void){
	return TIMER1_TICK;
}
