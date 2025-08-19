/*
 * eLesson Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

#ifndef DRV_ADC_H
#define DRV_ADC_H

/* includes ----------------------------------------------------------------- */
#include "elab/edf/normal/elab_adc.h"
#include "stm32f4xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MIN_ADC_CHANNEL 1
#define MAX_ADC_CHANNEL 15

/* public typedef ----------------------------------------------------------- */
typedef struct elab_adc_driver
{
    elab_adc_t device;
    
    __IO uint16_t           adc_value;
    
    ADC_HandleTypeDef       adc_handle;
    DMA_HandleTypeDef       dma_handle;
    ADC_ChannelConfTypeDef  adc_conf;
} elab_adc_driver_t;

/* public functions --------------------------------------------------------- */
void elab_driver_adc_init(elab_adc_driver_t *me, const char *name, 
                          const char *pin_name, const char *adc_name, uint8_t channel_num);

#ifdef __cplusplus
}
#endif

#endif  /* DRV_PIN_H */

/* ----------------------------- end of file -------------------------------- */
