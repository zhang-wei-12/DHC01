
/*
 * eLesson Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* includes ----------------------------------------------------------------- */
#include "drv_watchdog.h"
#include "elab/common/elab_assert.h"

#ifdef __cplusplus
extern "C" {
#endif

ELAB_TAG("DriverWatchdog");

/* private function prototype ----------------------------------------------- */
static void _set_time(elab_watchdog_t *me, uint32_t timeout_ms);
static void _feed(elab_watchdog_t *me);

/* private variables -------------------------------------------------------- */
static elab_watchdog_ops_t watchdog_driver_ops =
{
    .set_time = _set_time,
    .feed = _feed,
};

/* public functions --------------------------------------------------------- */
void elab_driver_watchdog_init(elab_watchdog_driver_t *me, const char *name)
{
    elab_assert(me != NULL);
    elab_assert(name != NULL);
    
    IWDG_HandleTypeDef *hiwdg = &me->hiwdg;
    
    hiwdg->Instance = IWDG; // 选择独立看门狗IWDG
    // 配置看门狗
    hiwdg->Init.Prescaler = IWDG_PRESCALER_64; // 预分频器，64分频
    hiwdg->Init.Reload = 0xFFF; // 重载值，最大计数值为0xFFF
    elab_assert(HAL_IWDG_Init(hiwdg) == HAL_OK);

    elab_watchdog_register(&me->device, name, &watchdog_driver_ops, me);
}

static void _set_time(elab_watchdog_t *me, uint32_t timeout_ms)
{
    elab_assert(me != NULL);
    elab_assert(timeout_ms > 0 && timeout_ms <= 6 * 1000);//范围：0~6秒
    
    elab_watchdog_driver_t *driver = (elab_watchdog_driver_t *)me->super.user_data;
    IWDG_HandleTypeDef *hiwdg = &driver->hiwdg;
    
    uint32_t reloadValue = timeout_ms * 40 / (4 * (1 << hiwdg->Init.Prescaler));

    // 检查重载值是否超过0xFFF
    if (reloadValue > 0xFFF) {
        reloadValue = 0xFFF;
    }

    // 重新初始化看门狗
    hiwdg->Init.Reload = reloadValue;
    elab_assert(HAL_IWDG_Init(hiwdg) == HAL_OK);
}

static void _feed(elab_watchdog_t *me)
{
    elab_assert(me != NULL);
    
    elab_watchdog_driver_t *driver = (elab_watchdog_driver_t *)me->super.user_data;
    IWDG_HandleTypeDef *hiwdg = &driver->hiwdg;
    
    // 把重装载寄存器的值放到计数器中，喂狗，防止IWDG复位
	// 当计数器的值减到0的时候会产生系统复位
	HAL_IWDG_Refresh(hiwdg);
}

#ifdef __cplusplus
}
#endif

/* ----------------------------- end of file -------------------------------- */
