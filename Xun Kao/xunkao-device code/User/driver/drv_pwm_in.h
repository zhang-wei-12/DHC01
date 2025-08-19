/*
 * eLesson Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

#ifndef DRV_PWM_IN_H
#define DRV_PWM_IN_H

/* includes ----------------------------------------------------------------- */
#include "elab/edf/normal/elab_pwm_in.h"
#include "stm32f4xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 高级控制定时器 */
#define ADVANCE_TIM           		    	TIM4
#define ADVANCE_TIM_CLK_ENABLE()      		__TIM4_CLK_ENABLE()
/* 捕获/比较中断 */
#define ADVANCE_TIM_IRQn					TIM4_IRQn
#define ADVANCE_TIM_IRQHandler    TIM4_IRQHandler
/* 高级控制定时器PWM输入捕获 */
/* PWM输入捕获引脚 */
#define ADVANCE_ICPWM_PIN              		GPIO_PIN_6              
#define ADVANCE_ICPWM_GPIO_PORT        		GPIOB                  
#define ADVANCE_ICPWM_GPIO_CLK_ENABLE()  	__GPIOB_CLK_ENABLE()
#define ADVANCE_ICPWM_AF					GPIO_AF2_TIM4
#define ADVANCE_IC1PWM_CHANNEL        		TIM_CHANNEL_1
#define ADVANCE_IC2PWM_CHANNEL        		TIM_CHANNEL_2

/* public typedef ----------------------------------------------------------- */
typedef struct elab_adc_driver
{
    elab_pwm_in_t device;
    const char *pin_name;
    TIM_HandleTypeDef *htim;
    
	uint32_t freq;
    uint8_t duty;
} elab_pwm_in_driver_t;

/* public functions --------------------------------------------------------- */
void elab_driver_pwm_in_init(elab_pwm_in_driver_t *me, const char *name, const char *pin_name);

#ifdef __cplusplus
}
#endif

#endif  

/* ----------------------------- end of file -------------------------------- */
