/*
 * eLesson Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

#ifndef DRV_UTIL_H
#define DRV_UTIL_H

/* includes ----------------------------------------------------------------- */
#include <stdbool.h>
#include <stdint.h>
#include "stm32f4xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

/* public functions --------------------------------------------------------- */
bool check_pin_name_valid(const char *name);
GPIO_TypeDef *get_port_from_name(const char *name);
uint16_t get_pin_from_name(const char *name);
void gpio_clock_enable(const char *name);

uint8_t get_uart_num_form_name(const char *name);
uint32_t get_altet_from_uart(const char *name);

void uart_clock_enable(const char *name);
void dma_clock_enable(const char *name);
void dma_irq_enable(DMA_Stream_TypeDef *inst);
void uart_irq_enable(USART_TypeDef *inst);

uint8_t get_adc_num_form_name(const char *name);
void adc_clock_enable(const char *name);
uint32_t get_adc_channel_from_num(uint8_t channel_num);

#ifdef __cplusplus
}
#endif

#endif  /* DRV_UTIL_H */

/* ----------------------------- end of file -------------------------------- */
