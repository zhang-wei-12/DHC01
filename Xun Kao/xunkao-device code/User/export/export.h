
#ifndef EXPORT_H
#define EXPORT_H

#include "elab/edf/user/elab_RSxxx.h"

enum
{
    EXPORT_LEVEL_PIN_MCU        = 1,
    EXPORT_LEVEL_ADC            = 1,
    EXPORT_LEVEL_PWM_MCU        = 1,
    EXPORT_LEVEL_UART           = 2,
    EXPORT_LEVEL_I2C            = 2,
    EXPORT_LEVEL_SPI            = 2,
    EXPORT_LEVEL_OLED           = 3,
    EXPORT_LEVEL_PIN_I2C        = 3,
    EXPORT_LEVEL_BUTTON         = 4,
    EXPORT_LEVEL_LED            = 4,
};

RSxxx_t* get_RSxxx_handle(const char *name);

#endif
