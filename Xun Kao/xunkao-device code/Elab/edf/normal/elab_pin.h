/*
 * eLesson Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */
/*Pin设备类，继承设备基类
1、实现设备基类通用IO操作接口（若需要）
2、向上实现Pin设备独有操作接口
3、向下提供Pin设备注册接口
*/

#ifndef ELAB_PIN_H
#define ELAB_PIN_H

/* includes ----------------------------------------------------------------- */
#include "../elab_device.h"

#ifdef __cplusplus
extern "C" {
#endif

/* public define ------------------------------------------------------------ */
enum pin_mode
{
    PIN_MODE_INPUT = 0,
    PIN_MODE_INPUT_PULLUP,
    PIN_MODE_INPUT_PULLDOWN,
    PIN_MODE_OUTPUT_PP,
    PIN_MODE_OUTPUT_OD,

    PIN_MODE_MAX
};

/* public typedef ----------------------------------------------------------- */
typedef struct elab_pin
{
    elab_device_t super;

    const struct elab_pin_ops *ops;
    uint8_t mode;
    bool status;
} elab_pin_t;

typedef struct elab_pin_ops
{
    elab_err_t (* init)(elab_pin_t * const me);
    elab_err_t (* set_mode)(elab_pin_t * const me, uint8_t mode);
    elab_err_t (* get_status)(elab_pin_t * const me, bool *status);
    elab_err_t (* set_status)(elab_pin_t * const me, bool status);
} elab_pin_ops_t;

#define ELAB_PIN_CAST(_dev)             ((elab_pin_t *)_dev)

/* public functions --------------------------------------------------------- */
/* For low-level driver. */
void elab_pin_register(elab_pin_t * const me,
                        const char *name,
                        const elab_pin_ops_t *ops,
                        void *user_data);

/* For high-level code. */
void elab_pin_set_mode(elab_device_t * const me, uint8_t mode);
bool elab_pin_get_status(elab_device_t * const me);
void elab_pin_set_status(elab_device_t * const me, bool status);

#ifdef __cplusplus
}
#endif

#endif  /* ELAB_PIN_H */

/* ----------------------------- end of file -------------------------------- */
