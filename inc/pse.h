#ifndef _PSE_H_
#define _PSE_H_
#include "STC8xxxx.H"

#define	PRJ_NAME	"PSE\n"
#define SW_VER		"Sw_Ver:V1.0\n"
#define HW_VER		"Hw_Ver:V1.0\n"

//*************************************//
#define DEBUG							(0)			//1���򿪵��ԣ�0:�ر�  (�򿪵���ģʽ�����ڿ��Դ�ӡ����״̬)

#define ADC_AVGNUM				(4)			//ÿ��ͨ����������n�κ�ƽ����ȡֵ������4��6��10

#define ADC_SPEED					(2)			//ADC�������ڣ�ȡֵ0~15��Խ��Խ��Խ׼ȷ

#define D_MAXREFFACTOR		(1.16)	//���D������ʣ�������ʼ���S��
#define D_MINREFFACTOR		(1.12)	//���D����С���ʣ�������ʼ���S��

#define W_48VMAX					(1900.0)	//48v��������ѹ(mv)��⣬������ʼ���D��
#define W_48VMIN					(1600.0)	//48v������С��ѹ(mv)��⣬������ʼ���D��

#define S_VENTER					(400.0)	//���S���ս��빤��״̬һ��ʱ���ڵ�����ѹ(mv)��������ֵ�ر�G
#define S_VMAX						(320.0)	//���S������ѹ(mv)��������ֵ�����ر�G��
#define S_VMID						(300.0)	//���S���ϴ��ѹ(mv)��������ֵһ��ʱ���ر�G��
#define S_VMIN						(5.0)		//���S����С��ѹ(mv)��С�ڴ�ֵһ��ʱ���ر�G��

#define NOT_DETECT_TIME		(300)		//δ����48vʱADC�������(ms)��(�������3�η��ϲŽ�����D��)
#define READY_DETECT_TIME	(50)		//����48v��D�������(ms)��(�������3�η��ϲŽ�����S��)
#define ENTER_WORK_TIME		(300)		//���빤��״̬����ʱ(ms)s����VMAX�Ƚϣ���������VENTER�Ƚ�
#define EXIT_LOWLOAD_TIME	(500)		//���S������ĳֵ������������ʱ��(ms)��ر�G��
#define EXIT_MIDLOAD_TIME	(5)			//���S������ĳֵ������������ʱ��(ms)��ر�G��
#define EXIT_WAIT_TIME		(1500)	//�ر�G���󣬵ȴ�һ��ʱ��(ms)�ٿ�ʼ���D��
//*************************************//
//���κθ�����Ҫ���±������Ч

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
