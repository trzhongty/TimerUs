#ifndef __TIMDELAY_H
#define __TIMDELAY_H

#include "stm32f0xx_hal.h"


//��ֲ�������
//======================================================
#define TIMDelayhtim			htim14		//��ʹ�õĶ�ʱ��
#define TIMDelay_TASK_NUMMAX		2			//�ܹ��ṩ�����������ŵ���ʱ����

#define TIMDelay_TASK_0_WIFIAT		0		//����ͬ����������
#define TIMDelay_TASK_1_UARTBUS		1


//������ز���
//======================================================

typedef enum
{
	TIMDelay_Ready 	= 0,
	TIMDelay_Busy 	= 1,
	TIMDelay_OK 	= 2
}TIMDelay_state_enum;

//ÿ����������ʱ�������洢������
typedef struct
{
	void 		( *pfun)();		//�ص����ܺ���ָ��
//	uint16_t	us;				//��ʱ����
//	uint16_t	timarr;			//���ص�ARR��ֵ
	uint8_t		use_it;			//����Ϊ���ûص����������򲻻ص�
	volatile TIMDelay_state_enum state;		//����״̬
	
}TIMDelay_Task_TypeDef;

typedef struct
{
	TIMDelay_Task_TypeDef	Task[ TIMDelay_TASK_NUMMAX ];	//ÿ���������Ϣ�ṹ������
	uint16_t	TaskNumTab[ TIMDelay_TASK_NUMMAX ];		//������ʱ�����ִ��˳��
	uint16_t	TaskarrTab[ TIMDelay_TASK_NUMMAX ];	//ÿ����ʱ�����ʱ���,��С�������򣬶�Ӧ���������ΪTaskTab�ڵ����
	
	volatile uint8_t	WorkNum;							//��һ�������Ķ�ʱ�������ڼ����������
//	volatile uint8_t	Taskp;								//ָ��TaskTab�����
	
}TIMDelay_TypeDef;



//�꺯���ͺ�����ȫ�ֱ�������
//======================================================

extern TIMDelay_TypeDef htimdelay;

HAL_StatusTypeDef TIMDelayInit( uint8_t TaskNum, void ( *pfun)() );
void TIMDelayIT(void);
HAL_StatusTypeDef TIMDelayStart( uint8_t TaskNum, uint16_t us, uint8_t use_it);

#endif
