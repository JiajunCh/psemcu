#ifndef _UART_H_
#define _UART_H_

#include "STC8xxxx.H"

#define		Baudrate1		115200UL
#define		TX1_LENGTH	192
#define		RX1_LENGTH	20

extern volatile uint8_t xdata RX1_Buffer[RX1_LENGTH];	//rx buffer
extern volatile bit B_RX1_OK;	//receive complete flag

extern void delay(uint8_t time);

void UART1_config(void);
void UART1_RxProcess(void);
void UART1_TxProcess(void);
void TX1_write2buff(uint8_t dat);
void PrintString1(uint8_t *str);
void timeEv_UARTRx1Rst(uint8_t tick);

#endif
