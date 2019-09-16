/**
  ******************************************************************************
  * @file    timerus.h
  * @author  ThinkHome
  * @brief   基于单个硬件定时器实现的多任务US级别中断回调软件定时器
  ******************************************************************************
  * @version 	0.1
  * @date		2019年9月12日23:46:52
  * @note		基于单个硬件定时器实现的多任务US级别中断回调软件定时器
  * @since		初次创建
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TIMERUS_H__
#define __TIMERUS_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stdint.h"

/** @addtogroup TimerUs
  * @{
  */
/* Exported constants --------------------------------------------------------*/
/** @defgroup TimerUs Exported constants
  * @{
  */


/** @defgroup TimerUs 移植相关宏定义分组
  * @{
  */

#ifndef TIMERUS_TASK_NUMMAX
    #define TIMERUS_TASK_NUMMAX		2		///<总共提供两个互补干扰的延时任务
#endif

/**
  * @}
  */


 /**
  * @}
  */

/* Exported types ------------------------------------------------------------*/
/** @defgroup TimerUs Exported Types
  * @{
  */

typedef enum
{
	TIMERUS_READY 	= 0,
	TIMERUS_BUSY 	= 1,
	TIMERUS_OK 	= 2
}TimerUs_state_enum;

//每个独立的延时任务所存储的数据
typedef struct
{
	void 		( *pfun)();		//回调功能函数指针
//	uint16_t	us;				//延时长度
//	uint16_t	timarr;			//加载到ARR的值
	uint8_t		use_it;			//非零为启用回调函数，否则不回调
	volatile TimerUs_state_enum state;		//工作状态
	
}TimerUs_Task_TypeDef;

typedef struct
{
	TimerUs_Task_TypeDef	Task[ TIMERUS_TASK_NUMMAX ];	//每个任务的信息结构体数组
	uint16_t	TaskNumTab[ TIMERUS_TASK_NUMMAX ];		//决定延时任务的执行顺序
	uint16_t	TaskarrTab[ TIMERUS_TASK_NUMMAX ];	//每个延时任务的时间点,从小到大排序，对应的任务序号为TaskTab内的序号
	
    void              * TimHandle;      //定时器句柄(指针)  //用于方便对接
	volatile uint8_t	WorkNum;        //在一次连续的定时器工作期间的任务总数
    uint8_t             TaskNum;        //记录已注册的任务数量,与序号上限
	
}TimerUs_TypeDef;

/**
  * @}
  */

/* Exported macro ------------------------------------------------------------*/
/** @defgroup TimerUs Exported Macros
  * @{
  */

/**
  * @}
  */

/* Exported functions --------------------------------------------------------*/
/** @addtogroup TimerUs_Exported_Functions
  * @{
  */

 /** @addtogroup TimerUs 需移植对接函数接口
  * @{
  */

/**
  * @brief  暂停硬件定时器计数
  * @note	只暂停不重置
  * @param  pTimerUs	主结构体指针
  */
void TimerUs_HAL_PauseTim( TimerUs_TypeDef *pTimerUs );

/**
  * @brief  恢复硬件定时器计数
  * @note	只恢复不重置
  * @param  pTimerUs	主结构体指针
  */
void TimerUs_HAL_EnableTim( TimerUs_TypeDef *pTimerUs );

/**
  * @brief  设置硬件定时器自动重载值
  * @param  pTimerUs	主结构体指针
  * 		AutoReload	自动重载值
  */
void TimerUs_HAL_SetTimAutoReload( TimerUs_TypeDef *pTimerUs , uint16_t AutoReload);

/**
  * @brief  获取硬件定时器自动重载值
  * @param  pTimerUs	主结构体指针
  */
uint16_t TimerUs_HAL_GetTimAutoReload( TimerUs_TypeDef *pTimerUs);

/**
  * @brief  设置硬件定时器计数器值
  * @param  pTimerUs	主结构体指针
  * 		Cnt			计数器值
  */
void TimerUs_HAL_SetTimCounter( TimerUs_TypeDef *pTimerUs , uint16_t Cnt);

/**
  * @brief  获取硬件定时器计数值
  * @param  pTimerUs	主结构体指针
  */
uint16_t TimerUs_HAL_GetTimCounter( TimerUs_TypeDef *pTimerUs);

/**
  * @brief  关闭硬件定时器并关闭中断
  * @param  pTimerUs	主结构体指针
  */
void TimerUs_HAL_StopTimIt( TimerUs_TypeDef *pTimerUs );

/**
  * @brief  开启硬件定时器并开启中断
  * @param  pTimerUs	主结构体指针
  */
void TimerUs_HAL_StartTimIt( TimerUs_TypeDef *pTimerUs );

/**
  * @brief  使能硬件定时器中断
  * @param  pTimerUs	主结构体指针
  */
void TimerUs_HAL_EnableTimIt( TimerUs_TypeDef *pTimerUs );

/**
  * @brief  失能硬件定时器中断
  * @param  pTimerUs	主结构体指针
  */
void TimerUs_HAL_DisableTimIt( TimerUs_TypeDef *pTimerUs );

 /**
  * @}
  */

 /** @addtogroup TimerUs_Exported_Functions
  * @{
  */

/**
  * @brief  硬件定时器中断函数
  * @param  pTimerUs    定时器主结构体指针
  */
void TimerUs_IT( TimerUs_TypeDef *pTimerUs );

/**
  * @brief  初始化定时器结构体
  * @param  pTimerUs    定时器主结构体指针
  * @param  TimHandle   硬件定时器句柄（指针）
  */
 void TimerUs_Init( TimerUs_TypeDef *pTimerUs, void * TimHandle );

/**
  * @brief  创建一个定时任务
  * @param  pTimerUs    定时器主结构体指针
  * @param  pFun        定时中断回调函数,存在一个u8参数，指明任务序号
  * @return 返回任务序号，小于0表示创建失败
  */
int8_t TimerUs_TaskCreate( TimerUs_TypeDef *pTimerUs, void ( *pFun)( uint8_t TaskNum ) );

/**
  * @brief  启用定时器任务
  * @note	参数范围限定，us不得大于65534，不得小于2，
  * 		TaskNum不得大于TimerUs_TASK_NUMMAX
  * @param  TaskNum	定时器任务序号
  * @param  us		定时时间，单位us
  * @param  use_it	是否使用中断回调
  * @return 成功与否
  * @retval 0	成功
  * 		-1	失败
  * 		-2	繁忙
  */
int8_t TimerUs_Start( TimerUs_TypeDef *pTimerUs, uint8_t TaskNum, uint16_t us, uint8_t use_it);



 /**
  * @}
  */

/**
  * @}
  */

/* Private types -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
/** @defgroup TimerUs Private Constants
  * @{
  */

/**
  * @}
  */

/* Private macros ------------------------------------------------------------*/
/** @defgroup TimerUs Private Macros
  * @{
  */

/**
  * @}
  */

/* Private functions ---------------------------------------------------------*/
/** @defgroup TimerUs Private Functions
  * @{
  */

/**
  * @}
  */

///TimerUs分组结束花括号
/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* __TIMERUS_H__ */

/*************************** trzhongty@163.com **************END OF FILE*******/









