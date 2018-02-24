#ifndef _PSE_H_
#define _PSE_H_
#include "STC8xxxx.H"

#define	PRJ_NAME	"PSE\n"
#define SW_VER		"Sw_Ver:V1.0\n"
#define HW_VER		"Hw_Ver:V1.0\n"

//*************************************//
#define DEBUG							(0)			//1：打开调试，0:关闭  (打开调试模式，串口可以打印工作状态)

#define ADC_AVGNUM				(4)			//每个通道连续采样n次后平均，取值必须是4或6或10

#define ADC_SPEED					(2)			//ADC采样周期，取值0~15，越大越慢越准确

#define D_MAXREFFACTOR		(1.16)	//检测D极最大倍率，符合则开始检测S极
#define D_MINREFFACTOR		(1.12)	//检测D极最小倍率，符合则开始检测S极

#define W_48VMAX					(1900.0)	//48v插入最大分压(mv)检测，符合则开始检测D极
#define W_48VMIN					(1600.0)	//48v插入最小分压(mv)检测，符合则开始检测D极

#define S_VENTER					(400.0)	//检测S极刚进入工作状态一段时间内的最大电压(mv)，超过此值关闭G
#define S_VMAX						(320.0)	//检测S极最大电压(mv)，超过此值立即关闭G极
#define S_VMID						(300.0)	//检测S极较大电压(mv)，超过此值一段时间会关闭G极
#define S_VMIN						(5.0)		//检测S极最小电压(mv)，小于此值一段时间会关闭G极

#define NOT_DETECT_TIME		(300)		//未插入48v时ADC采样间隔(ms)，(程序会检测3次符合才进入检测D极)
#define READY_DETECT_TIME	(50)		//插入48v后，D极检测间隔(ms)，(程序会检测3次符合才进入检测S极)
#define ENTER_WORK_TIME		(300)		//进入工作状态，延时(ms)s极与VMAX比较，但立即与VENTER比较
#define EXIT_LOWLOAD_TIME	(500)		//检测S极低于某值，持续超过此时间(ms)会关闭G极
#define EXIT_MIDLOAD_TIME	(5)			//检测S极高于某值，持续超过此时间(ms)会关闭G极
#define EXIT_WAIT_TIME		(1500)	//关闭G极后，等待一段时间(ms)再开始检测D极
//*************************************//
//做任何更改需要重新编译才生效

#define MAX_CH	(6)

#define PORT_INIT()	\
P0M0=0x00;	P0M1=0x00;\
P1M0=0x00;	P1M1=0x00;\
P2M0=0xFF;	P2M1=0x00;\
P3M0=0xE0;	P3M1=0x00;\
P4M0=0x03;	P4M1=0x00;\
P5M0=0x00;	P5M1=0x00;

//led output : 0==on,1==off
#define L_ON  (0)
#define L_OFF	(1)
sbit L1 = P2^1;
sbit L2 = P4^2;
sbit L3 = P2^0;
sbit L4 = P4^1;
sbit L5 = P3^7;
sbit L6 = P3^6;

//adc bandgap channel
#define CH_BG	0x0F

//pse Vref channel
#define CH_REF	0x06	//p16

//drain input : adc channel
#define CH_D1	0x0E	//p06
#define CH_D2	0x00	//p10
#define CH_D3	0x01	//p11
#define CH_D4	0x02	//p12
#define CH_D5	0x04	//p14
#define CH_D6	0x03	//p13

//source input : adc channel
#define CH_S1	0x08	//p00
#define CH_S2	0x0B	//p03
#define CH_S3	0x0A	//p02
#define CH_S4	0x09	//p01
#define CH_S5	0x0C	//p04
#define CH_S6	0x0D	//p05

//base output : 0==off,1==on
#define G_ON  (1)
#define G_OFF	(0)
sbit G1 = P2^2;
sbit G2 = P2^3;
sbit G3 = P2^4;
sbit G4 = P2^5;
sbit G5 = P2^6;
sbit G6 = P2^7;

//p reserve : output/input?
sbit P_1 = P3^4;
sbit P_2 = P4^0;
sbit P_3 = P5^5;
sbit P_4 = P1^7;

//ADD reserve : output/input?
sbit ADD0 = P4^4;
sbit ADD1 = P1^5;

//pwrmax led output : 0==on,1==off
#define PL_ON  	(0)
#define PL_OFF	(1)
sbit PWR_L = P3^5;

typedef void (*state_handle)(uint8_t param);

extern void debug_delay(uint8_t time);

void timeEv_PMaxLed(uint8_t tick);
void timeEv_FlagWork(uint8_t tick);
void timeEv_SysStart(uint8_t tick);
void WDG_config(void);
void WDG_freed(void);
void system_init(void);
void parameter_refresh(void);
void pse_process(void);

#endif
