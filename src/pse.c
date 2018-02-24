#include "pse.h"
#include "adc.h"
#include "uart.h"
#include "timer.h"
#include "stdio.h"
	
// global variable
static uint16_t xdata PMaxLed_Blink = 1000;	//power max led blink interval time (ms)
static state_handle xdata Pse_Ch[MAX_CH] = {0};		//handler every channel
static const uint8_t code D[MAX_CH] ={CH_D1,CH_D2,CH_D3,CH_D4,CH_D5,CH_D6};//D_pin channel remap
static const uint8_t code S[MAX_CH] ={CH_S1,CH_S2,CH_S3,CH_S4,CH_S5,CH_S6};//S_pin channel remap
static uint8_t xdata D_VAL[MAX_CH];			//D_pin confirm
static uint16_t xdata FLAG_STIME[MAX_CH];		//working state in time
static uint16_t ADbg = 0;			//adc value real time
static uint16_t Vbg = 0;			//Vbandgap
static uint16_t xdata AD10mv = 0;	//adc value of 10mv real time
static uint16_t xdata AD300mv = 0;	//adc value of 300mv real time
static uint16_t xdata AD400mv = 0;	//adc value of 400mv real time
static uint16_t xdata ADenter = 0;	//adc value of s enter work
static uint16_t xdata AD1600mv = 0;	//adc value of 1600mv real time
static uint16_t xdata AD1900mv = 0;	//adc value of 1900mv real time
static uint8_t xdata Sys_PowerUp = 0;	//48v in: 0/no,1/yes
static uint16_t xdata SysStart_Tick = 0;
static uint8_t xdata CntNotReady = 0;

// private function
static void pse_notready(uint8_t ch);
static void pse_ready(uint8_t ch);
static void pse_working(uint8_t ch);
static void pse_working_hightload(uint8_t ch);
static void pse_working_lowload(uint8_t ch);
static void pse_waiting(uint8_t ch);
static void set_l(uint8_t ch, uint8_t sta);
static void set_g(uint8_t ch, uint8_t sta);
static void pse_translog(uint8_t ch, uint8_t log);

//========================================================================
// function:		WDG_config
// description:	start wdg
// parameter: 	void
// return: 			void
// version: 		V1.0, 2018-1-11
//========================================================================
void WDG_config(void){
	WDT_CONTR = D_EN_WDT | D_WDT_SCALE_128;
}

//========================================================================
// function:		WDG_freed
// description:	wdg clear
// parameter: 	void
// return: 			void
// version: 		V1.0, 2018-1-11
//========================================================================
void WDG_freed(void){
	WDT_CONTR |= D_CLR_WDT;
}

//========================================================================
// function:		timeEv_PMaxLed
// description:	ctrl power max led blink
// parameter: 	system tick
// return: 			void
// version: 		V1.0, 2018-1-11
//========================================================================
void timeEv_PMaxLed(uint8_t tick){
	static uint16_t pmax_tick = 0;
	pmax_tick += tick;
	if(pmax_tick > PMaxLed_Blink){
		PWR_L = !PWR_L;
		pmax_tick = 0;
	}
}

//========================================================================
// function:		timeEv_FlagWork
// description:	calculate working stat intime
// parameter: 	system tick
// return: 			void
// version: 		V1.0, 2018-1-11
//========================================================================
void timeEv_FlagWork(uint8_t tick){
	uint8_t ch;
	for(ch=0; ch<MAX_CH; ch++)
		if(FLAG_STIME[ch] < 0x8000)
			FLAG_STIME[ch] += tick;
}

//========================================================================
// function:		timeEv_PMaxLed
// description:	ctrl power max led blink
// parameter: 	system tick
// return: 			void
// version: 		V1.0, 2018-1-11
//========================================================================
void timeEv_SysStart(uint8_t tick){
	if(SysStart_Tick < 0x8000)
		SysStart_Tick += tick;
}

//========================================================================
// function:		system_init
// description:	system environment init config 
// parameter: 	void
// return: 			void
// version: 		V1.0, 2018-1-11
//========================================================================
void system_init(void){
	uint8_t ch=0;
	Vbg = Get_VBandGap();
	for(ch=0; ch<MAX_CH; ch++){
		set_g(ch, G_OFF);
		Pse_Ch[ch] = pse_notready;
		pse_translog(ch, 'n');
	}
}

//========================================================================
// function:		enter_working
// description:	enter working state
// parameter: 	channel
// return: 			void
// version: 		V1.0, 2018-1-11
//========================================================================
static void enter_working(uint8_t ch){
	set_l(ch, L_ON);		//turn working
	set_g(ch, G_ON);
	FLAG_STIME[ch] = 0xFFFF;
	Pse_Ch[ch] = pse_working;
	pse_translog(ch, 'w');
}

//========================================================================
// function:		leave_working
// description:	leave working state
// parameter: 	channel
// return: 			void
// version: 		V1.0, 2018-1-11
//========================================================================
static void leave_working(uint8_t ch){
	set_g(ch, G_OFF);
	set_l(ch, L_OFF);
	FLAG_STIME[ch] = 0;
	Pse_Ch[ch] = pse_waiting;		//turn waiting
	pse_translog(ch, 't');
}

//========================================================================
// function:		reset_allpse
// description:	48Vin not in,reset state
// parameter: 	channel
// return: 			void
// version: 		V1.0, 2018-1-11
//========================================================================
static void reset_allpse(void){
	uint8_t ch=0;
	Sys_PowerUp = 0;
	for(ch=0; ch<MAX_CH; ch++){
		set_l(ch, L_OFF);
		set_g(ch, G_OFF);
		FLAG_STIME[ch] = 0xFFFF;
		SysStart_Tick = 0;
		Pse_Ch[ch] = pse_notready;		//turn notready
		pse_translog(ch, 'n');
	}
}

//========================================================================
// function:		pse_notready
// description:	pse[ch] in notready state,have not 48Vin
// parameter: 	channel
// return: 			void
// version: 		V1.0, 2018-1-11
//========================================================================
static void pse_notready(uint8_t ch){
	uint16_t temp_adref=0;
	if(SysStart_Tick > NOT_DETECT_TIME){
		SysStart_Tick = 0;
	}
	else
		return;
	temp_adref = Get_averADC(CH_REF, ADC_AVGNUM);
	if(AD1600mv<=temp_adref && AD1900mv>temp_adref){
		CntNotReady++;
		for(ch=0; ch<MAX_CH; ch++)
			set_l(ch, L_ON);
		if(CntNotReady > 3){
			CntNotReady=0;
			Sys_PowerUp = 1;
			for(ch=0; ch<MAX_CH; ch++){
				set_l(ch, L_OFF);
				FLAG_STIME[ch] = 0;
				Pse_Ch[ch] = pse_ready;		//turn ready
				pse_translog(ch, 'r');
			}
		}
	}
	else{
		CntNotReady = 0;
		for(ch=0; ch<MAX_CH; ch++){
			set_l(ch, L_OFF);
		}
	}
}

//========================================================================
// function:		pse_ready
// description:	pse[ch] in ready state
// parameter: 	channel
// return: 			void
// version: 		V1.0, 2018-1-11
//========================================================================
static void pse_ready(uint8_t ch){
	uint16_t temp_adref=0,adref_min=0,adref_max=0,aver_ad=0;
	if(FLAG_STIME[ch] > READY_DETECT_TIME)
		FLAG_STIME[ch] = 0;
	else
		return;
	temp_adref = Get_averADC(CH_REF, ADC_AVGNUM);
	adref_min = D_MINREFFACTOR * (float)temp_adref;
	adref_max = D_MAXREFFACTOR * (float)temp_adref;
	aver_ad = Get_averADC(D[ch], ADC_AVGNUM);	//get d adc value
	if(adref_min<=aver_ad && adref_max>aver_ad){	//confirm d adc in range
		D_VAL[ch]++;
		if(D_VAL[ch] >= 3){		//continuous confirm 3 times
			D_VAL[ch] = 0;
			enter_working(ch);	//turn working
			FLAG_STIME[ch] = 0;
		}
	}
	else
		D_VAL[ch] = 0;
}

//========================================================================
// function:		pse_working
// description:	pse[ch] in normal working state
// parameter: 	channel
// return: 			void
// version: 		V1.0, 2018-1-11
//========================================================================
static void pse_working(uint8_t ch){
	uint16_t aver_ad = 0;
	aver_ad = Get_averADC(S[ch], ADC_AVGNUM);	//get s adc value
	if(FLAG_STIME[ch] < ENTER_WORK_TIME){
		if(aver_ad > ADenter){
			leave_working(ch);		//turn waiting
		}
		return;
	}
	if(aver_ad > AD400mv){		// >400mv
		leave_working(ch);		//turn waiting
	}
	else if(aver_ad > AD300mv){	// >300mv
		Pse_Ch[ch] = pse_working_hightload;	 //turn working_heightload
		pse_translog(ch, 'h');
		FLAG_STIME[ch] = 0;
	}
	else if(aver_ad < AD10mv){	// <=10mv
		Pse_Ch[ch] = pse_working_lowload;		//turn working_lowload
		pse_translog(ch, 'l');
		FLAG_STIME[ch] = 0;
	}
}

//========================================================================
// function:		pse_working_hightload
// description:	pse[ch] in working in hight load
// parameter: 	channel
// return: 			void
// version: 		V1.0, 2018-1-11
//========================================================================
static void pse_working_hightload(uint8_t ch){
	uint16_t aver_ad = 0;
	aver_ad = Get_averADC(S[ch], ADC_AVGNUM);	//get s adc value
	if(aver_ad > AD400mv){	// >400mv
		leave_working(ch);				//turn waiting
	}
	else if(aver_ad > AD300mv){// >300mv
		if(FLAG_STIME[ch] >= EXIT_MIDLOAD_TIME){	//over 300ms
			leave_working(ch);		 //turn waiting
		}
	}
	else if(aver_ad <= AD10mv){// <=10mv
		Pse_Ch[ch] = pse_working_lowload;		//turn working_lowload
		pse_translog(ch, 'l');
		FLAG_STIME[ch] = 0;
	}
	else{
		FLAG_STIME[ch] >>= 1;
		if(0 == FLAG_STIME[ch]){
			Pse_Ch[ch] = pse_working;
			pse_translog(ch, 'w');
		}
	}
}

//========================================================================
// function:		pse_working_lowload
// description:	pse[ch] in working in low load
// parameter: 	channel
// return: 			void
// version: 		V1.0, 2018-1-11
//========================================================================
static void pse_working_lowload(uint8_t ch){
	uint16_t aver_ad = 0;
	aver_ad = Get_averADC(S[ch], ADC_AVGNUM);	//get s adc value
	if(aver_ad > AD400mv){
		leave_working(ch);		//turn waiting
	}
	else if(aver_ad > AD300mv){// >300mv
		Pse_Ch[ch] = pse_working_hightload;	 //turn working_heightload
		pse_translog(ch, 'h');
		FLAG_STIME[ch] = 0;
	}
	else if(aver_ad <= AD10mv){
		if(FLAG_STIME[ch] >= EXIT_LOWLOAD_TIME){
			leave_working(ch);		//turn wait
		}
	}
	else{
		FLAG_STIME[ch] >>= 1;
		if(0 == FLAG_STIME[ch]){
			Pse_Ch[ch] = pse_working;
			pse_translog(ch, 'w');
		}
	}
}

//========================================================================
// function:		pse_waiting
// description:	pse[ch] in waiting state, delay 1000ms turn in ready state
// parameter: 	channel
// return: 			void
// version: 		V1.0, 2018-1-11
//========================================================================
static void pse_waiting(uint8_t ch){
	if(FLAG_STIME[ch] >= EXIT_WAIT_TIME){
		FLAG_STIME[ch] = 0;
		Pse_Ch[ch] = pse_ready;		//turn ready
		pse_translog(ch, 'r');
	}
}

//========================================================================
// function:		parameter_refresh
// description:	reference bandgap vref refresh
// parameter: 	void
// return: 			void
// version: 		V1.0, 2018-1-11
//========================================================================
void parameter_refresh(void){
	uint16_t temp_adref=0;
	ADbg = Get_averADC(CH_BG, ADC_AVGNUM);
	AD1900mv = (uint16_t)((float)ADbg*(W_48VMAX/(float)Vbg));
	AD1600mv = (uint16_t)((float)ADbg*(W_48VMIN/(float)Vbg));
	temp_adref = Get_averADC(CH_REF, ADC_AVGNUM);
	if(Sys_PowerUp && (AD1600mv>temp_adref || AD1900mv<=temp_adref)){
		reset_allpse();
	}
	ADenter = (uint16_t)((float)ADbg*(S_VENTER/(float)Vbg));
	AD400mv = (uint16_t)((float)ADbg*(S_VMAX/(float)Vbg));
	AD300mv = (uint16_t)((float)ADbg*(S_VMID/(float)Vbg));
	AD10mv = (uint16_t)((float)ADbg*(S_VMIN/(float)Vbg));
}

//========================================================================
// function:		pse_process
// description:	all channle scan, execute state_handle
// parameter: 	void
// return: 			void
// version: 		V1.0, 2018-1-11
//========================================================================
void pse_process(void){
	uint8_t ch=0;
	for(ch=0; ch<MAX_CH; ch++){
		if(0 != Pse_Ch[ch]){
			(*Pse_Ch[ch])(ch);
		}
	}
}

//========================================================================
// function:		set_l
// description:	led on/off
// parameter: 	channel, bit_state
// return: 			void
// version: 		V1.0, 2018-1-11
//========================================================================
static void set_l(uint8_t ch, uint8_t sta){
	switch(ch){
		case 0:{
			L1 = sta;
			break;
		}
		case 1:{
			L2 = sta;
			break;
		}
		case 2:{
			L3 = sta;
			break;
		}
		case 3:{
			L4 = sta;
			break;
		}
		case 4:{
			L5 = sta;
			break;
		}
		case 5:{
			L6 = sta;
			break;
		}
		default:break;
	}
}

//========================================================================
// function:		set_g
// description:	base g on/off
// parameter: 	channel, bit_state
// return: 			void
// version: 		V1.0, 2018-1-11
//========================================================================
static void set_g(uint8_t ch, uint8_t sta){
	if(ch>=MAX_CH || sta>1) return;
	switch(ch){
		case 0:{
			G1 = sta;
			break;
		}
		case 1:{
			G2 = sta;
			break;
		}
		case 2:{
			G3 = sta;
			break;
		}
		case 3:{
			G4 = sta;
			break;
		}
		case 4:{
			G5 = sta;
			break;
		}
		case 5:{
			G6 = sta;
			break;
		}
		default:break;
	}
#if DEBUG
	//printf log
	TX1_write2buff('G');
	TX1_write2buff(ch+'1');
	TX1_write2buff('=');
	TX1_write2buff(sta+'0');
	TX1_write2buff('\n');
#endif
}

//========================================================================
// function:		pse_translog
// description:	pse[ch] trans work state log
// parameter: 	channel,log
// return: 			void
// version: 		V1.0, 2018-1-11
//========================================================================
static void pse_translog(uint8_t ch, uint8_t log){
#if DEBUG
	//printf log
	TX1_write2buff(log);
	TX1_write2buff(ch+'1');
	TX1_write2buff('\n');
#endif
}

