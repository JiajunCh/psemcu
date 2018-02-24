#include "adc.h"
#include "pse.h"
#include "intrins.h"

static const uint16_t idata VBandGap_idata _at_ 0xEF;		//save in interal sram address:0xfe uint/mv
static uint16_t xdata VBandGap = 0;
static volatile uint8_t ADC_INTFLAG	= 0;
//========================================================================
// function:		ADC_config
// description:	init config adc
// parameter: 	void
// return: 			void
// version: 		V1.0, 2018-1-11
//========================================================================
void ADC_config(void){
	P0n_pure_input(0x7F);
	P1n_pure_input(0x5F);
	ADC_CONTR = ADC_POWER;		//ADC on
	ADCCFG = ADC_FMT | ADC_SPEED;
	VBandGap = VBandGap_idata;
	EADC = 1;
}

//========================================================================
// function:		ADC_handle
// description:	adc interrupt_ service handle
// parameter: 	void
// return: 			void
// version: 		V1.0, 2018-1-11
//========================================================================
void ADC_handle (void) interrupt 5 {
	ADC_INTFLAG = 1;
  ADC_CONTR &= ~ADC_FLAG;	//clear hardware flag
}

//========================================================================
// function:		Get_ADC12bit
// description:	execute once adc
// parameter: 	channel 0~xx
// return: 			12bit AD value
// version: 		V1.0, 2018-1-11
//========================================================================
uint16_t Get_ADC12bit(uint8_t channel){
	uint16_t temp_ad = 0;
	if(channel > 15) return 0;
	ADC_RES = 0;
	ADC_RESL = 0;
	ADC_INTFLAG = 0;
	ADC_CONTR = ADC_POWER | ADC_START | channel; 
	while( !ADC_INTFLAG );		//wait execute adc
	ADC_INTFLAG = 0;
	temp_ad = ADC_RESL;
	temp_ad |= (ADC_RES << 8);
	return	temp_ad;	//return ad result
}

//========================================================================
// function:		Get_averADC
// description:	execute number of times adc
// parameter: 	channel 0~xx
// return: 			adc average
// version: 		V1.0, 2018-1-11
//========================================================================
uint16_t Get_averADC(uint8_t channel, uint8_t num){
	uint16_t temp_ad=0, aver_ad=0, sum_ad=0;
	uint16_t max_ad=0,min_ad=0xFFFF;
	uint8_t i=0;
	if(channel > 15) return 0;
	for(i=0; i<num; i++){
		temp_ad = Get_ADC12bit(channel);
		sum_ad += temp_ad;
		if(temp_ad < min_ad)
			min_ad = temp_ad;
		if(temp_ad > max_ad)
			max_ad = temp_ad;
		aver_ad = ((sum_ad - min_ad - max_ad) / (num-2));
	}
	return aver_ad;
}

//========================================================================
// function:		Get_ADBandGap
// description:	read bandgap voltage value (mv)
// parameter: 	void
// return: 			bandgap voltage value (mv)
// version: 		V1.0, 2018-1-11
//========================================================================
uint16_t Get_VBandGap(void){
	return VBandGap;
}
