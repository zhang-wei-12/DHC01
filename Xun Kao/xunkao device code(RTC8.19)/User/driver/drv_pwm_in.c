
/*
 * eLesson Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* includes ----------------------------------------------------------------- */
#include "drv_pwm_in.h"
#include "drv_util.h"
#include "elab/common/elab_assert.h"

#ifdef __cplusplus
extern "C" {
#endif

ELAB_TAG("DriverPWMIN");

static __IO uint32_t capture1 = 0;
static __IO uint32_t capture2 = 0;
static __IO uint32_t period = 0;
static __IO uint32_t pulse_width = 0;
static __IO float frequency = 0.0;
static __IO float duty_cycle = 0.0;

static TIM_HandleTypeDef  TIM_PWMINPUT_Handle;
static float _get_freq(elab_pwm_in_t * const me);
static float _get_duty(elab_pwm_in_t * const me);

static void TIMx_Configuration(void);

static const elab_pwm_in_ops_t pin_driver_ops =
{
    .get_freq = _get_freq,
    .get_duty = _get_duty,
};

void elab_driver_pwm_in_init(elab_pwm_in_driver_t *me, const char *name, const char *pin_name)
{
    elab_assert(me != NULL);
    elab_assert(name != NULL);
    assert_name(check_pin_name_valid(pin_name), pin_name);
    
    //Ӳ����ʼ��
    TIMx_Configuration();
    me->htim = &TIM_PWMINPUT_Handle;
    
    me->pin_name = pin_name;
    elab_pwm_in_register(&me->device, name, &pin_driver_ops, me);
}

static float _get_freq(elab_pwm_in_t * const me)
{
    elab_assert(me != NULL);
    elab_pwm_in_driver_t *driver = (elab_pwm_in_driver_t *)me->super.user_data;
    
    driver->freq = frequency;
    
    return driver->freq;
}

static float _get_duty(elab_pwm_in_t * const me)
{
    elab_assert(me != NULL);
    elab_pwm_in_driver_t *driver = (elab_pwm_in_driver_t *)me->super.user_data;
    	
    driver->duty = duty_cycle;
    
    return driver->duty;
}

static void TIMx_GPIO_Config(void) 
{
	/*����һ��GPIO_InitTypeDef���͵Ľṹ��*/
	GPIO_InitTypeDef GPIO_InitStructure;

	/*������ʱ����ص�GPIO����ʱ��*/
	ADVANCE_ICPWM_GPIO_CLK_ENABLE(); 

	/* ��ʱ���������ų�ʼ�� */
	/* �߼���ʱ�����벶������ */
	GPIO_InitStructure.Pin = ADVANCE_ICPWM_PIN;	
    GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStructure.Pull = GPIO_NOPULL;
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStructure.Alternate = ADVANCE_ICPWM_AF;	
	HAL_GPIO_Init(ADVANCE_ICPWM_GPIO_PORT, &GPIO_InitStructure);
}

 /**
  * @brief  �߼����ƶ�ʱ�� TIMx,x[1,8]�ж����ȼ�����
  * @param  ��
  * @retval ��
  */
static void TIMx_NVIC_Configuration(void)
{
	//������ռ���ȼ��������ȼ�
	HAL_NVIC_SetPriority(ADVANCE_TIM_IRQn, 6, 0);
	// �����ж���Դ
	HAL_NVIC_EnableIRQ(ADVANCE_TIM_IRQn);
}

static void TIM_PWMINPUT_Config(void)
{	
	TIM_IC_InitTypeDef  	TIM_ICInitStructure;
    TIM_MasterConfigTypeDef sMasterConfig;
	
    // ����TIMx_CLK,x[1,8] 
	ADVANCE_TIM_CLK_ENABLE(); 
	/* ���嶨ʱ���ľ����ȷ����ʱ���Ĵ����Ļ���ַ*/
	TIM_PWMINPUT_Handle.Instance = ADVANCE_TIM;
	TIM_PWMINPUT_Handle.Init.Period = 0xFFFF; 	
	// �߼����ƶ�ʱ��ʱ��ԴTIMxCLK = HCLK=180MHz 
	TIM_PWMINPUT_Handle.Init.Prescaler = 180-1;	
	// ����ʱ�ӷ�Ƶ
	TIM_PWMINPUT_Handle.Init.ClockDivision=TIM_CLOCKDIVISION_DIV1;
	// ������ʽ
	TIM_PWMINPUT_Handle.Init.CounterMode=TIM_COUNTERMODE_UP;
	// ��ʼ����ʱ��TIMx, x[1,8]
	HAL_TIM_IC_Init(&TIM_PWMINPUT_Handle);
  
	
	/* IC����˫���ش���*/
	TIM_ICInitStructure.ICPolarity = TIM_ICPOLARITY_BOTHEDGE;
	TIM_ICInitStructure.ICSelection = TIM_ICSELECTION_DIRECTTI;
	TIM_ICInitStructure.ICPrescaler = TIM_ICPSC_DIV1;
	TIM_ICInitStructure.ICFilter = 0x0;
	HAL_TIM_IC_ConfigChannel(&TIM_PWMINPUT_Handle,&TIM_ICInitStructure,ADVANCE_IC1PWM_CHANNEL);
    
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    HAL_TIMEx_MasterConfigSynchronization(&TIM_PWMINPUT_Handle, &sMasterConfig);
    
    HAL_TIM_IC_Start_IT(&TIM_PWMINPUT_Handle,TIM_CHANNEL_1);
}

/**
  * @brief  ��ʼ���߼����ƶ�ʱ����ʱ��1ms����һ���ж�
  * @param  ��
  * @retval ��
  */
static void TIMx_Configuration(void)
{
	TIMx_GPIO_Config();
	
	TIMx_NVIC_Configuration();	
  
	TIM_PWMINPUT_Config();
}

void ADVANCE_TIM_IRQHandler(void)
{
   HAL_TIM_IRQHandler(&TIM_PWMINPUT_Handle);
}

/**
  * @brief  Conversion complete callback in non blocking mode 
  * @param  htim : hadc handle
  * @retval None
  */
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == ADVANCE_TIM && htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
  {
	  /* ��ȡ���벶��ֵ */
	    uint32_t currentCapture = HAL_TIM_ReadCapturedValue(&TIM_PWMINPUT_Handle,ADVANCE_IC1PWM_CHANNEL);	
      if (currentCapture != 0)
      {
           if (capture1 == 0)//�ߵ�ƽ����ʼ1����������
           {
                capture1 = currentCapture;
           }
           else if (capture2 == 0) //�ߵ�ƽ�������͵�ƽ��ʼ
           {
                capture2 = currentCapture;
                pulse_width = capture2 - capture1;//�����ȼ�ʱ
           }
            else//��һ���ߵ�ƽ������1����������
            {
                period = currentCapture - capture1;//�������ڼ�ʱ        
                // ����Ƶ��
                if (period > 0)
                {
                    frequency = HAL_RCC_GetPCLK2Freq() / (TIM_PWMINPUT_Handle.Init.Prescaler + 1) / (float)period;
                }
                // ����ռ�ձ�
                if (pulse_width > 0 && period > 0)
                {
                    duty_cycle = pulse_width / period * (float)100.0;
                }
                
                // ����capture1/capture2
                capture1 = currentCapture;
                capture2 = 0;
            }
      }       
  }
}

#ifdef __cplusplus
}
#endif

/* ----------------------------- end of file -------------------------------- */
