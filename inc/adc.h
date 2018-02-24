#ifndef _ADC_H_
#define _ADC_H_

#include "STC8xxxx.H"

#define ADC_SPEEDMASK	0x0F
#define ADC_SPEED32 	0x00
#define ADC_SPEED64 	0x01
#define ADC_SPEED96 	0x02
#define ADC_SPEED128 	0x03
#define ADC_SPEED160 	0x04
#define ADC_SPEED192 	0x05
#define ADC_SPEED224 	0x06
#define ADC_SPEED256 	0x07
#define ADC_SPEED288 	0x08
#define ADC_SPEED320 	0x09
#define ADC_SPEED352 	0x0A
#define ADC_SPEED384 	0x0B
#define ADC_SPEED416 	0x0C
#define ADC_SPEED448 	0x0D
#define ADC_SPEED480 	0x0E
#define ADC_SPEED512 	0x0F

void ADC_config(void);
unsigned int Get_ADC12bit(uint8_t channel);
uint16_t Get_averADC(uint8_t channel, uint8_t num);
uint16_t Get_VBandGap(void);

#endif
