/**
  ******************************************************************************
  * @file    timerus.c
  * @author  trzhongty@163.com
  * @brief   基于单个硬件定时器实现的多任务US级别中断回调软件定时器
  ******************************************************************************
  * @version 	0.1
  * @date		2019年9月12日23:33:12
  * @note		基于单个硬件定时器实现的多任务US级别中断回调软件定时器
  * @since		初次创建
  *
  @verbatim
  @endverbatim
  */

/* Includes ------------------------------------------------------------------*/
#include "timerus.h"

/** @defgroup TimerUs
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
/** @defgroup TimerUs Exported Functions
  * @{
  */

/**
  * @brief  初始化定时器结构体
  * @param  pTimerUs    定时器主结构体指针
  * @param  TimHandle   硬件定时器句柄（指针）
  */
 void TimerUs_Init( TimerUs_TypeDef *pTimerUs, void * TimHandle )
 {
     pTimerUs->TaskNum = 0;
     pTimerUs->WorkNum = 0;
     pTimerUs->TimHandle = TimHandle;
 }

/**
  * @brief  创建一个定时任务
  * @param  pTimerUs    定时器主结构体指针
  * @param  pFun    定时中断回调函数,存在一个u8参数，指明任务序号
  * @return 返回任务序号，小于0表示创建失败
  */
int8_t TimerUs_TaskCreate( TimerUs_TypeDef *pTimerUs, void ( *pFun)(void) )
{
    int8_t TaskNum = (int8_t)( pTimerUs->TaskNum ) + 1;

    if ( TaskNum > TIMERUS_TASK_NUMMAX ||
         TaskNum < 0 )
        return -1;

	pTimerUs->Task[ TaskNum ].pfun = pFun;
	pTimerUs->Task[ TaskNum ].state = TIMERUS_READY;
	return (pTimerUs->TaskNum)++;
}

/**
  * @brief  硬件定时器中断函数
  * @param  pTimerUs    定时器主结构体指针
  */
void TimerUs_IT( TimerUs_TypeDef *pTimerUs )
{
	static uint8_t i;
	
	TimerUs_HAL_PauseTim( pTimerUs );	//暂停CNT计数
	do
	{
		pTimerUs->Task[ pTimerUs->TaskNumTab[ 0 ] ].state = TIMERUS_OK;	//将第一序列任务状态设为OK
		
		if( (pTimerUs->Task[ pTimerUs->TaskNumTab[ 0 ] ]).use_it )	//如果启用了中断回调，则调用回调函数
			(pTimerUs->Task[ pTimerUs->TaskNumTab[ 0 ] ]).pfun();
		
		--(pTimerUs->WorkNum);
		for( i = 0; i < pTimerUs->WorkNum; ++i )		//执行完一个，序列前移，
		{
			pTimerUs->TaskarrTab[ i ] = pTimerUs->TaskarrTab[ i + 1 ];
			pTimerUs->TaskNumTab[ i ] = pTimerUs->TaskNumTab[ i + 1 ];
		}
		
	}while( ( pTimerUs->WorkNum > 0) && ( 0 == pTimerUs->TaskarrTab[ 0 ]));	//当前要执行的ARR值为0,并且有任务数
	
	if( 0 != pTimerUs->WorkNum )		//不只有一个延时任务时,启动下一个任务
	{
        TimerUs_HAL_SetTimAutoReload( pTimerUs, pTimerUs->TaskarrTab[ 0 ] );		//设置ARR值
		TimerUs_HAL_SetTimCounter( pTimerUs, 0 );   //清零CNT
        TimerUs_HAL_StartTimIt( pTimerUs );         //重新启动定时器
	}
	else
	{
        TimerUs_HAL_StopTimIt( pTimerUs );		    //关闭定时器
	}
}

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
int8_t TimerUs_Start( TimerUs_TypeDef *pTimerUs, uint8_t TaskNum, uint16_t us, uint8_t use_it)
{
	uint16_t 	tmp;
	uint8_t		i;
	
	if( TIMERUS_BUSY == pTimerUs->Task[ TaskNum ].state  )
		return -2;
	//pTimerUs->Task[ TaskNum ].us = us;
	pTimerUs->Task[ TaskNum ].state = TIMERUS_BUSY;
	pTimerUs->Task[ TaskNum ].use_it = use_it;
	
	//根据定时器的状态，不同的插入功能形式
	if( pTimerUs->WorkNum != 0)		//如果处于工作状态
	{
        TimerUs_HAL_PauseTim( pTimerUs );	        //暂停CNT计数
        TimerUs_HAL_DisableTimIt( pTimerUs );       //关闭中断使能，防止中途改变
		
		tmp = ( ( TimerUs_HAL_GetTimAutoReload( pTimerUs ) ) - ( TimerUs_HAL_GetTimCounter( pTimerUs ) ) + 1);	//TMP值含义为现在到定时器原本溢出的时间
		if( tmp > us)		//如果当前计时模式剩余时间大于插入任务所需要的时间，则将任务插入置最前端
		{
            //将CNT值增加提前，使得从现在开始计时溢出的时间为本次插入任务的要求时间
            TimerUs_HAL_SetTimCounter( pTimerUs, TimerUs_HAL_GetTimCounter( pTimerUs ) + tmp - us );
            
			TimerUs_HAL_EnableTim( pTimerUs );		//恢复CNT计数
			
			for( i = pTimerUs->WorkNum; i > 0; --i )		//插入至最前端，所有任务后移
			{
				pTimerUs->TaskNumTab[ i ] = pTimerUs->TaskNumTab[ i - 1 ];
				pTimerUs->TaskarrTab[ i ] = pTimerUs->TaskarrTab[ i - 1 ];
			}
			pTimerUs->TaskarrTab[ 1 ] = tmp - 1;	//原本处于工作的延时任务被滞后，滞后的ARR值为剩余时间值,而后续任务时间不变
			pTimerUs->TaskarrTab[ 0 ] = us - 1;		//该值不重要
			pTimerUs->TaskNumTab[ 0 ] = TaskNum;	//记录任务标号	
		}
		else	//否则任务将插入至从小到大的顺序中
		{
			TimerUs_HAL_EnableTim( pTimerUs );		//恢复CNT计数
			
			for( i = 1, us -= tmp; i < pTimerUs->WorkNum; ++i )		//任务依次从前向后累加比较
			{
				if( us < pTimerUs->TaskarrTab[ i ] + 1)		//插入任务小于当前序列的时间值，则插入该序列
				{
					for( tmp = pTimerUs->WorkNum; tmp > i + 1; ++tmp)	//插入至序列i，i后所有任务后移，除了被插入点，其他后续任务序号后移，ARR不变
					{
						pTimerUs->TaskNumTab[ tmp ] = pTimerUs->TaskNumTab[ tmp - 1 ];
						pTimerUs->TaskarrTab[ tmp ] = pTimerUs->TaskarrTab[ tmp - 1 ];
					}
					pTimerUs->TaskarrTab[ i + 1 ] = pTimerUs->TaskarrTab[ i ] - us;		//被插入的任务后移一个序号，且ARR值减去US
					pTimerUs->TaskNumTab[ i + 1 ] = pTimerUs->TaskNumTab[ i ];
					
					pTimerUs->TaskarrTab[ i ] = us - 1;		//新任务插入
					pTimerUs->TaskNumTab[ i ] = TaskNum;
					i = 0;	//用i=0来标识，已找到插入点情况，
					break;
				}
				else
				{
					us -= pTimerUs->TaskarrTab[ i ] + 1;	//插入任务大于当前序列时间值，减去这个时间差，后移
				}
			}
			if( 0 != i )	//如果i不为0，说明没有找到插入点，则直接插入至最后
			{
				pTimerUs->TaskarrTab[ pTimerUs->WorkNum ] = us - 1;
				pTimerUs->TaskNumTab[ pTimerUs->WorkNum ] = TaskNum;
			}
		}
	}
	else		//不处于工作状态，直接放置第一序列
	{
		pTimerUs->TaskarrTab[ 0 ] = us - 1;
		pTimerUs->TaskNumTab[ 0 ] = TaskNum;
        TimerUs_HAL_SetTimAutoReload( pTimerUs, us - 1 );		//设置ARR值
        TimerUs_HAL_SetTimCounter( pTimerUs, 0 );		//清零CNT
        TimerUs_HAL_EnableTim( pTimerUs );		//恢复CNT计数
	}
	++(pTimerUs->WorkNum);	//任务数+1
	
	TimerUs_HAL_EnableTimIt( pTimerUs );		//开启中断
	
	return 0;
}

/**
  * @brief  查询指定定时任务是否完成
  * @param  TaskNum	定时器任务序号
  * @return 0未结束或者未开始或者错误，1定时任务结束
  */
uint8_t TimerUs_IsDone( TimerUs_TypeDef *pTimerUs, uint8_t TaskNum )
{
    return ( TIMERUS_OK == pTimerUs->Task[ TaskNum ].state )?( 1 ):( 0 );
}

///函数分组结束花括号
/**
  * @}
  */

///模块分组结束花括号
/**
  * @}
  */

/*************************** trzhongty@163.com **************END OF FILE*******/
