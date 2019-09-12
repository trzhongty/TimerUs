#include "timdelay.h"
#include "tim.h"		//������ʹ�õĶ�ʱ�ṹ���ⲿ����



//���ܽṹ�嶨�弰��ʼ��
TIMDelay_TypeDef htimdelay =
{
	.WorkNum = 0
};

//������Ϣע��
HAL_StatusTypeDef TIMDelayInit( uint8_t TaskNum, void ( *pfun)() )
{
	if( TaskNum >= TIMDelay_TASK_NUMMAX )
		return HAL_ERROR;
	htimdelay.Task[ TaskNum ].pfun = pfun;
	htimdelay.Task[ TaskNum ].state = TIMDelay_Ready;
	return HAL_OK;
}

//��ʱ�������жϻص��������
void TIMDelayIT()
{
	static uint8_t i;
	
	
	TIMDelayhtim.Instance->CR1 &= ~(TIM_CR1_CEN);	//��ͣCNT����
	
	do
	{
		htimdelay.Task[ htimdelay.TaskNumTab[ 0 ] ].state = TIMDelay_OK;	//����һ��������״̬��ΪOK
		
		if( (htimdelay.Task[ htimdelay.TaskNumTab[ 0 ] ]).use_it )	//����������жϻص�������ûص�����
			(htimdelay.Task[ htimdelay.TaskNumTab[ 0 ] ]).pfun();
		
		--(htimdelay.WorkNum);
		for( i = 0; i < htimdelay.WorkNum; ++i )		//ִ����һ��������ǰ�ƣ�
		{
			htimdelay.TaskarrTab[ i ] = htimdelay.TaskarrTab[ i + 1 ];
			htimdelay.TaskNumTab[ i ] = htimdelay.TaskNumTab[ i + 1 ];
		}
		
	}while( ( htimdelay.WorkNum > 0) && ( 0 == htimdelay.TaskarrTab[ 0 ]));	//��ǰҪִ�е�ARRֵΪ0,������������
	
	if( 0 != htimdelay.WorkNum )		//��ֻ��һ����ʱ����ʱ,������һ������
	{
		__HAL_TIM_SET_AUTORELOAD( &TIMDelayhtim, htimdelay.TaskarrTab[ 0 ] );		//����ARRֵ
		__HAL_TIM_SET_COUNTER( &TIMDelayhtim, 0 );		//����CNT
		HAL_TIM_Base_Start_IT( &TIMDelayhtim);		//����������ʱ��
	}
	else
	{
		HAL_TIM_Base_Stop_IT( &TIMDelayhtim );		//�رն�ʱ��
	}
}

//���ö�ʱ����ʱ������Ϊ����ʱus�����Ƿ�ʹ���С��ϻص�����ָ��
//������Χ���ϸ��޶���us���ô���65534������С��2��TaskNum���ô���TIMDelay_TASK_NUMMAX
HAL_StatusTypeDef TIMDelayStart( uint8_t TaskNum, uint16_t us, uint8_t use_it)
{
	uint16_t 	tmp;
	uint8_t		i;
	
	if( TIMDelay_Busy == htimdelay.Task[ TaskNum ].state  )
		return HAL_BUSY;
	//htimdelay.Task[ TaskNum ].us = us;
	htimdelay.Task[ TaskNum ].state = TIMDelay_Busy;
	htimdelay.Task[ TaskNum ].use_it = use_it;
	
	//���ݶ�ʱ����״̬����ͬ�Ĳ��빦����ʽ
	if( htimdelay.WorkNum != 0)		//������ڹ���״̬
	{
		TIMDelayhtim.Instance->CR1 &= ~(TIM_CR1_CEN);	//��ͣCNT����
		__HAL_TIM_DISABLE_IT( &TIMDelayhtim, TIM_IT_UPDATE);	//�ر��жϣ���ֹ��;�ı�
		
		tmp = ( (TIMDelayhtim.Instance->ARR) - (TIMDelayhtim.Instance->CNT) + 1);	//TMPֵ����Ϊ���ڵ���ʱ��ԭ�������ʱ��
		if( tmp > us)		//�����ǰ��ʱģʽʣ��ʱ����ڲ�����������Ҫ��ʱ�䣬�������������ǰ��
		{
			TIMDelayhtim.Instance->CNT += tmp - us;		//��CNTֵ������ǰ��ʹ�ô����ڿ�ʼ��ʱ�����ʱ��Ϊ���β��������Ҫ��ʱ��
			__HAL_TIM_ENABLE( &TIMDelayhtim );		//�ָ�CNT����
			
			for( i = htimdelay.WorkNum; i > 0; --i )		//��������ǰ�ˣ������������
			{
				htimdelay.TaskNumTab[ i ] = htimdelay.TaskNumTab[ i - 1 ];
				htimdelay.TaskarrTab[ i ] = htimdelay.TaskarrTab[ i - 1 ];
			}
			htimdelay.TaskarrTab[ 1 ] = tmp - 1;	//ԭ�����ڹ�������ʱ�����ͺ��ͺ��ARRֵΪʣ��ʱ��ֵ,����������ʱ�䲻��
			htimdelay.TaskarrTab[ 0 ] = us - 1;		//��ֵ����Ҫ
			htimdelay.TaskNumTab[ 0 ] = TaskNum;	//��¼������	
		}
		else	//�������񽫲�������С�����˳����
		{
			__HAL_TIM_ENABLE( &TIMDelayhtim );		//�ָ�CNT����
			
			for( i = 1, us -= tmp; i < htimdelay.WorkNum; ++i )		//�������δ�ǰ����ۼӱȽ�
			{
				if( us < htimdelay.TaskarrTab[ i ] + 1)		//��������С�ڵ�ǰ���е�ʱ��ֵ������������
				{
					for( tmp = htimdelay.WorkNum; tmp > i + 1; ++tmp)	//����������i��i������������ƣ����˱�����㣬��������������ź��ƣ�ARR����
					{
						htimdelay.TaskNumTab[ tmp ] = htimdelay.TaskNumTab[ tmp - 1 ];
						htimdelay.TaskarrTab[ tmp ] = htimdelay.TaskarrTab[ tmp - 1 ];
					}
					htimdelay.TaskarrTab[ i + 1 ] = htimdelay.TaskarrTab[ i ] - us;		//��������������һ����ţ���ARRֵ��ȥUS
					htimdelay.TaskNumTab[ i + 1 ] = htimdelay.TaskNumTab[ i ];
					
					htimdelay.TaskarrTab[ i ] = us - 1;		//���������
					htimdelay.TaskNumTab[ i ] = TaskNum;
					i = 0;	//��i=0����ʶ�����ҵ�����������
					break;
				}
				else
				{
					us -= htimdelay.TaskarrTab[ i ] + 1;	//����������ڵ�ǰ����ʱ��ֵ����ȥ���ʱ������
				}
			}
			if( 0 != i )	//���i��Ϊ0��˵��û���ҵ�����㣬��ֱ�Ӳ��������
			{
				htimdelay.TaskarrTab[ htimdelay.WorkNum ] = us - 1;
				htimdelay.TaskNumTab[ htimdelay.WorkNum ] = TaskNum;
			}
		}
	}
	else		//�����ڹ���״̬��ֱ�ӷ��õ�һ����
	{
		htimdelay.TaskarrTab[ 0 ] = us - 1;
		htimdelay.TaskNumTab[ 0 ] = TaskNum;
		__HAL_TIM_SET_AUTORELOAD( &TIMDelayhtim, us - 1 );		//����ARRֵ
		__HAL_TIM_SET_COUNTER( &TIMDelayhtim, 0 );		//����CNT
		__HAL_TIM_ENABLE( &TIMDelayhtim );				//������ʱ����ʼ����
	}
	++(htimdelay.WorkNum);	//������+1
	
	__HAL_TIM_ENABLE_IT( &TIMDelayhtim, TIM_IT_UPDATE);		//�����ж�
	
	return HAL_OK;
}
