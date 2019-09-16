/**
  ******************************************************************************
  * @file    timerus.h
  * @author  ThinkHome
  * @brief   ���ڵ���Ӳ����ʱ��ʵ�ֵĶ�����US�����жϻص������ʱ��
  ******************************************************************************
  * @version 	0.1
  * @date		2019��9��12��23:46:52
  * @note		���ڵ���Ӳ����ʱ��ʵ�ֵĶ�����US�����жϻص������ʱ��
  * @since		���δ���
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


/** @defgroup TimerUs ��ֲ��غ궨�����
  * @{
  */

#ifndef TIMERUS_TASK_NUMMAX
    #define TIMERUS_TASK_NUMMAX		2		///<�ܹ��ṩ�����������ŵ���ʱ����
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

//ÿ����������ʱ�������洢������
typedef struct
{
	void 		( *pfun)();		//�ص����ܺ���ָ��
//	uint16_t	us;				//��ʱ����
//	uint16_t	timarr;			//���ص�ARR��ֵ
	uint8_t		use_it;			//����Ϊ���ûص����������򲻻ص�
	volatile TimerUs_state_enum state;		//����״̬
	
}TimerUs_Task_TypeDef;

typedef struct
{
	TimerUs_Task_TypeDef	Task[ TIMERUS_TASK_NUMMAX ];	//ÿ���������Ϣ�ṹ������
	uint16_t	TaskNumTab[ TIMERUS_TASK_NUMMAX ];		//������ʱ�����ִ��˳��
	uint16_t	TaskarrTab[ TIMERUS_TASK_NUMMAX ];	//ÿ����ʱ�����ʱ���,��С�������򣬶�Ӧ���������ΪTaskTab�ڵ����
	
    void              * TimHandle;      //��ʱ�����(ָ��)  //���ڷ���Խ�
	volatile uint8_t	WorkNum;        //��һ�������Ķ�ʱ�������ڼ����������
    uint8_t             TaskNum;        //��¼��ע�����������,���������
	
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

 /** @addtogroup TimerUs ����ֲ�ԽӺ����ӿ�
  * @{
  */

/**
  * @brief  ��ͣӲ����ʱ������
  * @note	ֻ��ͣ������
  * @param  pTimerUs	���ṹ��ָ��
  */
void TimerUs_HAL_PauseTim( TimerUs_TypeDef *pTimerUs );

/**
  * @brief  �ָ�Ӳ����ʱ������
  * @note	ֻ�ָ�������
  * @param  pTimerUs	���ṹ��ָ��
  */
void TimerUs_HAL_EnableTim( TimerUs_TypeDef *pTimerUs );

/**
  * @brief  ����Ӳ����ʱ���Զ�����ֵ
  * @param  pTimerUs	���ṹ��ָ��
  * 		AutoReload	�Զ�����ֵ
  */
void TimerUs_HAL_SetTimAutoReload( TimerUs_TypeDef *pTimerUs , uint16_t AutoReload);

/**
  * @brief  ��ȡӲ����ʱ���Զ�����ֵ
  * @param  pTimerUs	���ṹ��ָ��
  */
uint16_t TimerUs_HAL_GetTimAutoReload( TimerUs_TypeDef *pTimerUs);

/**
  * @brief  ����Ӳ����ʱ��������ֵ
  * @param  pTimerUs	���ṹ��ָ��
  * 		Cnt			������ֵ
  */
void TimerUs_HAL_SetTimCounter( TimerUs_TypeDef *pTimerUs , uint16_t Cnt);

/**
  * @brief  ��ȡӲ����ʱ������ֵ
  * @param  pTimerUs	���ṹ��ָ��
  */
uint16_t TimerUs_HAL_GetTimCounter( TimerUs_TypeDef *pTimerUs);

/**
  * @brief  �ر�Ӳ����ʱ�����ر��ж�
  * @param  pTimerUs	���ṹ��ָ��
  */
void TimerUs_HAL_StopTimIt( TimerUs_TypeDef *pTimerUs );

/**
  * @brief  ����Ӳ����ʱ���������ж�
  * @param  pTimerUs	���ṹ��ָ��
  */
void TimerUs_HAL_StartTimIt( TimerUs_TypeDef *pTimerUs );

/**
  * @brief  ʹ��Ӳ����ʱ���ж�
  * @param  pTimerUs	���ṹ��ָ��
  */
void TimerUs_HAL_EnableTimIt( TimerUs_TypeDef *pTimerUs );

/**
  * @brief  ʧ��Ӳ����ʱ���ж�
  * @param  pTimerUs	���ṹ��ָ��
  */
void TimerUs_HAL_DisableTimIt( TimerUs_TypeDef *pTimerUs );

 /**
  * @}
  */

 /** @addtogroup TimerUs_Exported_Functions
  * @{
  */

/**
  * @brief  Ӳ����ʱ���жϺ���
  * @param  pTimerUs    ��ʱ�����ṹ��ָ��
  */
void TimerUs_IT( TimerUs_TypeDef *pTimerUs );

/**
  * @brief  ��ʼ����ʱ���ṹ��
  * @param  pTimerUs    ��ʱ�����ṹ��ָ��
  * @param  TimHandle   Ӳ����ʱ�������ָ�룩
  */
 void TimerUs_Init( TimerUs_TypeDef *pTimerUs, void * TimHandle );

/**
  * @brief  ����һ����ʱ����
  * @param  pTimerUs    ��ʱ�����ṹ��ָ��
  * @param  pFun        ��ʱ�жϻص�����,����һ��u8������ָ���������
  * @return ����������ţ�С��0��ʾ����ʧ��
  */
int8_t TimerUs_TaskCreate( TimerUs_TypeDef *pTimerUs, void ( *pFun)( uint8_t TaskNum ) );

/**
  * @brief  ���ö�ʱ������
  * @note	������Χ�޶���us���ô���65534������С��2��
  * 		TaskNum���ô���TimerUs_TASK_NUMMAX
  * @param  TaskNum	��ʱ���������
  * @param  us		��ʱʱ�䣬��λus
  * @param  use_it	�Ƿ�ʹ���жϻص�
  * @return �ɹ����
  * @retval 0	�ɹ�
  * 		-1	ʧ��
  * 		-2	��æ
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

///TimerUs�������������
/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* __TIMERUS_H__ */

/*************************** trzhongty@163.com **************END OF FILE*******/









