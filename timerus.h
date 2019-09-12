#ifndef __TIMDELAY_H
#define __TIMDELAY_H

#include "stm32f0xx_hal.h"


//移植配置相关
//======================================================
#define TIMDelayhtim			htim14		//所使用的定时器
#define TIMDelay_TASK_NUMMAX		2			//总共提供两个互补干扰的延时任务

#define TIMDelay_TASK_0_WIFIAT		0		//给不同任务分配序号
#define TIMDelay_TASK_1_UARTBUS		1


//控制相关参数
//======================================================

typedef enum
{
	TIMDelay_Ready 	= 0,
	TIMDelay_Busy 	= 1,
	TIMDelay_OK 	= 2
}TIMDelay_state_enum;

//每个独立的延时任务所存储的数据
typedef struct
{
	void 		( *pfun)();		//回调功能函数指针
//	uint16_t	us;				//延时长度
//	uint16_t	timarr;			//加载到ARR的值
	uint8_t		use_it;			//非零为启用回调函数，否则不回调
	volatile TIMDelay_state_enum state;		//工作状态
	
}TIMDelay_Task_TypeDef;

typedef struct
{
	TIMDelay_Task_TypeDef	Task[ TIMDelay_TASK_NUMMAX ];	//每个任务的信息结构体数组
	uint16_t	TaskNumTab[ TIMDelay_TASK_NUMMAX ];		//决定延时任务的执行顺序
	uint16_t	TaskarrTab[ TIMDelay_TASK_NUMMAX ];	//每个延时任务的时间点,从小到大排序，对应的任务序号为TaskTab内的序号
	
	volatile uint8_t	WorkNum;							//在一次连续的定时器工作期间的任务总数
//	volatile uint8_t	Taskp;								//指向TaskTab的序号
	
}TIMDelay_TypeDef;



//宏函数和函数、全局变量声明
//======================================================

extern TIMDelay_TypeDef htimdelay;

HAL_StatusTypeDef TIMDelayInit( uint8_t TaskNum, void ( *pfun)() );
void TIMDelayIT(void);
HAL_StatusTypeDef TIMDelayStart( uint8_t TaskNum, uint16_t us, uint8_t use_it);

#endif
