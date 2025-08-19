/*
 * eLesson Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

#ifndef DRV_PIN_H
#define DRV_PIN_H

/* includes ----------------------------------------------------------------- */
#include "elab/edf/normal/elab_pin.h"
#include "stm32f4xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_KEY_IN  24     //输入按键数

typedef enum
{
    PRESS_BNT,      //按钮
    ROTATE_BNT     //旋钮
}key_type_t;

typedef struct pin_key_src 
{ 
     key_type_t type;
     char *key_no;
     char *pin_dev;
     char *pin_name;
}pin_key_src_t;

/* public typedef ----------------------------------------------------------- */
typedef struct elab_pin_driver
{
    elab_pin_t device;
    const char *pin_name;
} elab_pin_driver_t;

/* public functions --------------------------------------------------------- */
/* For example, the pin name should be like "A.02". */
void elab_driver_pin_init(elab_pin_driver_t *me,
                            const char *name, const char *pin_name);

/* For example, the pin name should be like "I2C2.38.02". */
/*
void elab_driver_pin_i2c_init(elab_pin_driver_t *me,
                                const char *name, 
                                const char *pin_name);
*/
#ifdef __cplusplus
}
#endif

#endif  /* DRV_PIN_H */

/* ----------------------------- end of file -------------------------------- */
