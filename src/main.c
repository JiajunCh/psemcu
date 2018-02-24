#include "STC8xxxx.H"
#include "pse.h"
#include "adc.h"
#include "uart.h"
#include "timer.h"
#include "string.h"

//========================================================================
// function:		any delay function
// description:	delay time in while
// parameter: 	void
// return: 			void
// version: 		V1.0, 2018-1-11
//========================================================================
void delay(uint8_t time){
	for(;time>0;time--);
}

void debug_delay(uint8_t time){
	for(;time>0;time--)
		delay(255);
}

//========================================================================
// function:		timeEvent_Process
// description:	cyclicity_event and deferred_event proccess
// parameter: 	a timerx tick
// return: 			void
// version: 		V1.0, 2018-1-11
//========================================================================
void timeEvent_Process(uint8_t new_tick){
	static uint8_t old_tick = 0;
	static uint8_t cnt_tick = 0;
	cnt_tick = new_tick-old_tick;
	if(0 == cnt_tick) return;
	old_tick = new_tick;

	timeEv_PMaxLed(cnt_tick);
#if DEBUG
	timeEv_UARTRx1Rst(cnt_tick);
#endif
	timeEv_FlagWork(cnt_tick);
	timeEv_SysStart(cnt_tick);
}

//========================================================================
// function:		main
// description:	main
// parameter: 	void
// return: 			void
// version: 		V1.0, 2018-1-11
//========================================================================
void main(void){
	PORT_INIT();
	ADC_config();
#if DEBUG
	UART1_config();
#endif
	TIMER1_config();
	WDG_config();
	EA = 1;	//enable interuupt_
	system_init();
	while(1){
		parameter_refresh();
		pse_process();
#if DEBUG
		UART1_RxProcess();
		UART1_TxProcess();
#endif
		timeEvent_Process( get_timer1tick() );
		WDG_freed();
	}
}
