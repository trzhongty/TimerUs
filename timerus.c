/**
  ******************************************************************************
  * @file    timerus.c
  * @author  trzhongty@163.com
  * @brief   ���ڵ���Ӳ����ʱ��ʵ�ֵĶ�����US�����жϻص������ʱ��
  ******************************************************************************
  * @version 	0.1
  * @date		2019��9��12��23:33:12
  * @note		���ڵ���Ӳ����ʱ��ʵ�ֵĶ�����US�����жϻص������ʱ��
  * @since		���δ���
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
  * @brief  ��ʼ����ʱ���ṹ��
  * @param  pTimerUs    ��ʱ�����ṹ��ָ��
  * @param  TimHandle   Ӳ����ʱ�������ָ�룩
  */
 void TimerUs_Init( TimerUs_TypeDef *pTimerUs, void * TimHandle )
 {
     pTimerUs->TaskNum = 0;
     pTimerUs->WorkNum = 0;
     pTimerUs->TimHandle = TimHandle;
 }

/**
  * @brief  ����һ����ʱ����
  * @param  pTimerUs    ��ʱ�����ṹ��ָ��
  * @param  pFun    ��ʱ�жϻص�����,����һ��u8������ָ���������
  * @return ����������ţ�С��0��ʾ����ʧ��
  */
int8_t TimerUs_TaskCreate( TimerUs_TypeDef *pTimerUs, void ( *pFun)(void) )
{
    int8_t TaskNum = (int8_t)( pTimerUs->TaskNum ) + 1;

    if ( TaskNum > TIMERUS_TASK_NUMMAX ||
         TaskNum < 0 )
        return -1;

	pTimerUs->Task[ pTimerUs->TaskNum ].pfun = pFun;
	pTimerUs->Task[ pTimerUs->TaskNum ].state = TIMERUS_READY;
	return (pTimerUs->TaskNum)++;
}

/**
  * @brief  Ӳ����ʱ���жϺ���
  * @param  pTimerUs    ��ʱ�����ṹ��ָ��
  */
void TimerUs_IT( TimerUs_TypeDef *pTimerUs )
{
	static uint8_t i;
    static uint8_t TaskNumTmp;
	
	TimerUs_HAL_PauseTim( pTimerUs );	//��ͣCNT����
	do
	{
        TaskNumTmp = pTimerUs->TaskNumTab[ 0 ];
        
		//ִ����һ��������ǰ�ƣ�
		--(pTimerUs->WorkNum);
		for( i = 0; i < pTimerUs->WorkNum; ++i )		
		{
			pTimerUs->TaskarrTab[ i ] = pTimerUs->TaskarrTab[ i + 1 ];
			pTimerUs->TaskNumTab[ i ] = pTimerUs->TaskNumTab[ i + 1 ];
		}
        
        pTimerUs->Task[ TaskNumTmp ].state = TIMERUS_OK;	//����һ��������״̬��ΪOK
        
        if( (pTimerUs->Task[ TaskNumTmp ]).use_it )	//����������жϻص�������ûص�����
			(pTimerUs->Task[ TaskNumTmp ]).pfun();
		
	}while( ( pTimerUs->WorkNum > 0) && ( 0 == pTimerUs->TaskarrTab[ 0 ]));	//��ǰҪִ�е�ARRֵΪ0,������������
	
	if( 0 != pTimerUs->WorkNum )		//��ֻ��һ����ʱ����ʱ,������һ������
	{
        TimerUs_HAL_SetTimAutoReload( pTimerUs, pTimerUs->TaskarrTab[ 0 ] );		//����ARRֵ
		TimerUs_HAL_SetTimCounter( pTimerUs, 0 );   //����CNT
        TimerUs_HAL_StartTimIt( pTimerUs );         //����������ʱ��
	}
	else
	{
        TimerUs_HAL_StopTimIt( pTimerUs );		    //�رն�ʱ��
	}
}

/**
  * @brief  ��ֹ������ʱ������
  * @note	�ڶ�ʱ���񵽴�ָ��ʱ�䲢���лص�֮ǰ�����øú���������Ӷ�ʱ����������޳�������
  * @param  pTimerUs    ��ʱ�����ṹ��ָ��
  * @param  TaskNum	��ʱ���������
  */
void TimerUs_AbortTask( TimerUs_TypeDef *pTimerUs, uint8_t TaskNum )
{
    static uint8_t i;

    if ( ( TIMERUS_BUSY != pTimerUs->Task[ TaskNum ].state ) ||
         ( 0 == pTimerUs->WorkNum ) )
        return;
    
    //���ڶ�ʱ�������У��Ҹ�����Ҳ�ڶ�����ʱ�����д���
    TimerUs_HAL_PauseTim( pTimerUs );	        //��ͣCNT����
    
    for ( i = 0; i < pTimerUs->WorkNum; i++)
    {
        //��λ�������ڶ����е����
        if ( pTimerUs->TaskNumTab[ i ] == TaskNum )
            break;
    }
    if ( i < pTimerUs->WorkNum )
    {
        //˵����λ����������ţ���������
        uint16_t tmp;

        pTimerUs->Task[ TaskNum ].state = TIMERUS_READY;
        
        if ( 0 == i )
        {
            //������Ϊ����ִ�е�����
            //TMPֵ����Ϊ���ڵ���ʱ��ԭ�������ʱ��
            tmp = ( ( TimerUs_HAL_GetTimAutoReload( pTimerUs ) ) - ( TimerUs_HAL_GetTimCounter( pTimerUs ) ) + 1);
        }
        else
        {
            //������Ϊ��δִ�е�����
            tmp = pTimerUs->TaskarrTab[ i ] + 1;
        }
        if ( --(pTimerUs->WorkNum) )
        {
            //��һ������ARRʱ�䣬���ϱ��޳������µ�ʱ���
            pTimerUs->TaskarrTab[ i + 1 ] += tmp;
            //�������ǰ��
            for( ; i < pTimerUs->WorkNum; ++i )		
            {
                pTimerUs->TaskarrTab[ i ] = pTimerUs->TaskarrTab[ i + 1 ];
                pTimerUs->TaskNumTab[ i ] = pTimerUs->TaskNumTab[ i + 1 ];
            }
            //����ARRֵ��tmpֵ�����Ϊ��ǰ������ARRֵ
            TimerUs_HAL_SetTimAutoReload( pTimerUs, pTimerUs->TaskarrTab[ 0 ] );
        }
        else
        {
            //����һ�������޳���û��������ִ�����񡣹رն�ʱ��
            TimerUs_HAL_StopTimIt( pTimerUs );
            return;
        }
    }

    TimerUs_HAL_EnableTim( pTimerUs );		//�ָ�CNT����
}


/**
  * @brief  ���ö�ʱ������
  * @note	������Χ�޶���us���ô���65534������С��2��
  * 		TaskNum���ô���TimerUs_TASK_NUMMAX
  * @param  pTimerUs    ��ʱ�����ṹ��ָ��
  * @param  TaskNum	��ʱ���������
  * @param  us		��ʱʱ�䣬��λus
  * @param  use_it	�Ƿ�ʹ���жϻص�
  * @return �ɹ����
  * @retval 0	�ɹ�
  * 		-1	ʧ��
  * 		-2	��æ
  */
int8_t TimerUs_Start( TimerUs_TypeDef *pTimerUs, uint8_t TaskNum, uint16_t us, uint8_t use_it)
{
	uint16_t 	tmp;
	uint8_t		i;
	
    TimerUs_AbortTask( pTimerUs, TaskNum );     //����������Ѿ��ڽ����У������޳������

	//pTimerUs->Task[ TaskNum ].us = us;
	pTimerUs->Task[ TaskNum ].state = TIMERUS_BUSY;
	pTimerUs->Task[ TaskNum ].use_it = use_it;
	
	//���ݶ�ʱ����״̬����ͬ�Ĳ��빦����ʽ
	if( pTimerUs->WorkNum != 0)		//������ڹ���״̬
	{
        TimerUs_HAL_PauseTim( pTimerUs );	        //��ͣCNT����
        TimerUs_HAL_DisableTimIt( pTimerUs );       //�ر��ж�ʹ�ܣ���ֹ��;�ı�
		
		tmp = ( ( TimerUs_HAL_GetTimAutoReload( pTimerUs ) ) - ( TimerUs_HAL_GetTimCounter( pTimerUs ) ) + 1);	//TMPֵ����Ϊ���ڵ���ʱ��ԭ�������ʱ��
		if( tmp > us)		//�����ǰ��ʱģʽʣ��ʱ����ڲ�����������Ҫ��ʱ�䣬�������������ǰ��
		{
            //��CNTֵ������ǰ��ʹ�ô����ڿ�ʼ��ʱ�����ʱ��Ϊ���β��������Ҫ��ʱ��
            TimerUs_HAL_SetTimCounter( pTimerUs, TimerUs_HAL_GetTimCounter( pTimerUs ) + tmp - us );
            
			TimerUs_HAL_EnableTim( pTimerUs );		//�ָ�CNT����
			
			for( i = pTimerUs->WorkNum; i > 0; --i )		//��������ǰ�ˣ������������
			{
				pTimerUs->TaskNumTab[ i ] = pTimerUs->TaskNumTab[ i - 1 ];
				pTimerUs->TaskarrTab[ i ] = pTimerUs->TaskarrTab[ i - 1 ];
			}
			pTimerUs->TaskarrTab[ 1 ] = tmp - 1;	//ԭ�����ڹ�������ʱ�����ͺ��ͺ��ARRֵΪʣ��ʱ��ֵ,����������ʱ�䲻��
			pTimerUs->TaskarrTab[ 0 ] = us - 1;		//��ֵ����Ҫ
			pTimerUs->TaskNumTab[ 0 ] = TaskNum;	//��¼������	
		}
		else	//�������񽫲�������С�����˳����
		{
			TimerUs_HAL_EnableTim( pTimerUs );		//�ָ�CNT����
			
			for( i = 1, us -= tmp; i < pTimerUs->WorkNum; ++i )		//�������δ�ǰ����ۼӱȽ�
			{
				if( us < pTimerUs->TaskarrTab[ i ] + 1)		//��������С�ڵ�ǰ���е�ʱ��ֵ������������
				{
					for( tmp = pTimerUs->WorkNum; tmp > i + 1; ++tmp)	//����������i��i������������ƣ����˱�����㣬��������������ź��ƣ�ARR����
					{
						pTimerUs->TaskNumTab[ tmp ] = pTimerUs->TaskNumTab[ tmp - 1 ];
						pTimerUs->TaskarrTab[ tmp ] = pTimerUs->TaskarrTab[ tmp - 1 ];
					}
					pTimerUs->TaskarrTab[ i + 1 ] = pTimerUs->TaskarrTab[ i ] - us;		//��������������һ����ţ���ARRֵ��ȥUS
					pTimerUs->TaskNumTab[ i + 1 ] = pTimerUs->TaskNumTab[ i ];
					
					pTimerUs->TaskarrTab[ i ] = us - 1;		//���������
					pTimerUs->TaskNumTab[ i ] = TaskNum;
					i = 0;	//��i=0����ʶ�����ҵ�����������
					break;
				}
				else
				{
					us -= pTimerUs->TaskarrTab[ i ] + 1;	//����������ڵ�ǰ����ʱ��ֵ����ȥ���ʱ������
				}
			}
			if( 0 != i )	//���i��Ϊ0��˵��û���ҵ�����㣬��ֱ�Ӳ��������
			{
				pTimerUs->TaskarrTab[ pTimerUs->WorkNum ] = us - 1;
				pTimerUs->TaskNumTab[ pTimerUs->WorkNum ] = TaskNum;
			}
		}
	}
	else		//�����ڹ���״̬��ֱ�ӷ��õ�һ����
	{
		pTimerUs->TaskarrTab[ 0 ] = us - 1;
		pTimerUs->TaskNumTab[ 0 ] = TaskNum;
        TimerUs_HAL_SetTimAutoReload( pTimerUs, us - 1 );		//����ARRֵ
        TimerUs_HAL_SetTimCounter( pTimerUs, 0 );		//����CNT
        TimerUs_HAL_StartTimIt( pTimerUs );		//������ʱ��
	}
	++(pTimerUs->WorkNum);	//������+1
	
	TimerUs_HAL_EnableTimIt( pTimerUs );		//�����ж�
	
	return 0;
}

/**
  * @brief  ��ѯָ����ʱ�����Ƿ����
  * @param  TaskNum	��ʱ���������
  * @return 0δ��������δ��ʼ���ߴ���1��ʱ�������
  */
uint8_t TimerUs_IsDone( TimerUs_TypeDef *pTimerUs, uint8_t TaskNum )
{
    return ( TIMERUS_OK == pTimerUs->Task[ TaskNum ].state )?( 1 ):( 0 );
}

///�����������������
/**
  * @}
  */

///ģ��������������
/**
  * @}
  */

/*************************** trzhongty@163.com **************END OF FILE*******/
