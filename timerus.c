#include "timdelay.h"
#include "tim.h"		//包含所使用的定时结构体外部声明



//功能结构体定义及初始化
TIMDelay_TypeDef htimdelay =
{
	.WorkNum = 0
};

//任务信息注册
HAL_StatusTypeDef TIMDelayInit( uint8_t TaskNum, void ( *pfun)() )
{
	if( TaskNum >= TIMDelay_TASK_NUMMAX )
		return HAL_ERROR;
	htimdelay.Task[ TaskNum ].pfun = pfun;
	htimdelay.Task[ TaskNum ].state = TIMDelay_Ready;
	return HAL_OK;
}

//延时结束，中断回调函数入口
void TIMDelayIT()
{
	static uint8_t i;
	
	
	TIMDelayhtim.Instance->CR1 &= ~(TIM_CR1_CEN);	//暂停CNT计数
	
	do
	{
		htimdelay.Task[ htimdelay.TaskNumTab[ 0 ] ].state = TIMDelay_OK;	//将第一序列任务状态设为OK
		
		if( (htimdelay.Task[ htimdelay.TaskNumTab[ 0 ] ]).use_it )	//如果启用了中断回调，则调用回调函数
			(htimdelay.Task[ htimdelay.TaskNumTab[ 0 ] ]).pfun();
		
		--(htimdelay.WorkNum);
		for( i = 0; i < htimdelay.WorkNum; ++i )		//执行完一个，序列前移，
		{
			htimdelay.TaskarrTab[ i ] = htimdelay.TaskarrTab[ i + 1 ];
			htimdelay.TaskNumTab[ i ] = htimdelay.TaskNumTab[ i + 1 ];
		}
		
	}while( ( htimdelay.WorkNum > 0) && ( 0 == htimdelay.TaskarrTab[ 0 ]));	//当前要执行的ARR值为0,并且有任务数
	
	if( 0 != htimdelay.WorkNum )		//不只有一个延时任务时,启动下一个任务
	{
		__HAL_TIM_SET_AUTORELOAD( &TIMDelayhtim, htimdelay.TaskarrTab[ 0 ] );		//设置ARR值
		__HAL_TIM_SET_COUNTER( &TIMDelayhtim, 0 );		//清零CNT
		HAL_TIM_Base_Start_IT( &TIMDelayhtim);		//重新启动定时器
	}
	else
	{
		HAL_TIM_Base_Stop_IT( &TIMDelayhtim );		//关闭定时器
	}
}

//启用定时器延时，参数为：延时us数、是否使能中、断回调函数指针
//函数范围有严格限定，us不得大于65534，不得小于2，TaskNum不得大于TIMDelay_TASK_NUMMAX
HAL_StatusTypeDef TIMDelayStart( uint8_t TaskNum, uint16_t us, uint8_t use_it)
{
	uint16_t 	tmp;
	uint8_t		i;
	
	if( TIMDelay_Busy == htimdelay.Task[ TaskNum ].state  )
		return HAL_BUSY;
	//htimdelay.Task[ TaskNum ].us = us;
	htimdelay.Task[ TaskNum ].state = TIMDelay_Busy;
	htimdelay.Task[ TaskNum ].use_it = use_it;
	
	//根据定时器的状态，不同的插入功能形式
	if( htimdelay.WorkNum != 0)		//如果处于工作状态
	{
		TIMDelayhtim.Instance->CR1 &= ~(TIM_CR1_CEN);	//暂停CNT计数
		__HAL_TIM_DISABLE_IT( &TIMDelayhtim, TIM_IT_UPDATE);	//关闭中断，防止中途改变
		
		tmp = ( (TIMDelayhtim.Instance->ARR) - (TIMDelayhtim.Instance->CNT) + 1);	//TMP值含义为现在到定时器原本溢出的时间
		if( tmp > us)		//如果当前计时模式剩余时间大于插入任务所需要的时间，则将任务插入置最前端
		{
			TIMDelayhtim.Instance->CNT += tmp - us;		//将CNT值增加提前，使得从现在开始计时溢出的时间为本次插入任务的要求时间
			__HAL_TIM_ENABLE( &TIMDelayhtim );		//恢复CNT计数
			
			for( i = htimdelay.WorkNum; i > 0; --i )		//插入至最前端，所有任务后移
			{
				htimdelay.TaskNumTab[ i ] = htimdelay.TaskNumTab[ i - 1 ];
				htimdelay.TaskarrTab[ i ] = htimdelay.TaskarrTab[ i - 1 ];
			}
			htimdelay.TaskarrTab[ 1 ] = tmp - 1;	//原本处于工作的延时任务被滞后，滞后的ARR值为剩余时间值,而后续任务时间不变
			htimdelay.TaskarrTab[ 0 ] = us - 1;		//该值不重要
			htimdelay.TaskNumTab[ 0 ] = TaskNum;	//记录任务标号	
		}
		else	//否则任务将插入至从小到大的顺序中
		{
			__HAL_TIM_ENABLE( &TIMDelayhtim );		//恢复CNT计数
			
			for( i = 1, us -= tmp; i < htimdelay.WorkNum; ++i )		//任务依次从前向后累加比较
			{
				if( us < htimdelay.TaskarrTab[ i ] + 1)		//插入任务小于当前序列的时间值，则插入该序列
				{
					for( tmp = htimdelay.WorkNum; tmp > i + 1; ++tmp)	//插入至序列i，i后所有任务后移，除了被插入点，其他后续任务序号后移，ARR不变
					{
						htimdelay.TaskNumTab[ tmp ] = htimdelay.TaskNumTab[ tmp - 1 ];
						htimdelay.TaskarrTab[ tmp ] = htimdelay.TaskarrTab[ tmp - 1 ];
					}
					htimdelay.TaskarrTab[ i + 1 ] = htimdelay.TaskarrTab[ i ] - us;		//被插入的任务后移一个序号，且ARR值减去US
					htimdelay.TaskNumTab[ i + 1 ] = htimdelay.TaskNumTab[ i ];
					
					htimdelay.TaskarrTab[ i ] = us - 1;		//新任务插入
					htimdelay.TaskNumTab[ i ] = TaskNum;
					i = 0;	//用i=0来标识，已找到插入点情况，
					break;
				}
				else
				{
					us -= htimdelay.TaskarrTab[ i ] + 1;	//插入任务大于当前序列时间值，减去这个时间差，后移
				}
			}
			if( 0 != i )	//如果i不为0，说明没有找到插入点，则直接插入至最后
			{
				htimdelay.TaskarrTab[ htimdelay.WorkNum ] = us - 1;
				htimdelay.TaskNumTab[ htimdelay.WorkNum ] = TaskNum;
			}
		}
	}
	else		//不处于工作状态，直接放置第一序列
	{
		htimdelay.TaskarrTab[ 0 ] = us - 1;
		htimdelay.TaskNumTab[ 0 ] = TaskNum;
		__HAL_TIM_SET_AUTORELOAD( &TIMDelayhtim, us - 1 );		//设置ARR值
		__HAL_TIM_SET_COUNTER( &TIMDelayhtim, 0 );		//清零CNT
		__HAL_TIM_ENABLE( &TIMDelayhtim );				//启动定时器开始计数
	}
	++(htimdelay.WorkNum);	//任务数+1
	
	__HAL_TIM_ENABLE_IT( &TIMDelayhtim, TIM_IT_UPDATE);		//开启中断
	
	return HAL_OK;
}
