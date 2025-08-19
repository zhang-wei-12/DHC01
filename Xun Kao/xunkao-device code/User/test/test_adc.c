/* includes ----------------------------------------------------------------- */
#include <stdlib.h>
#include "elab/common/elab_assert.h"
#include "elab/3rd/Shell/shell.h"
#include "elab/common/elab_log.h"
#include "elab/edf/normal/elab_adc.h"
#include "elab/common/elab_export.h"

#ifdef __cplusplus
extern "C" {
#endif

ELAB_TAG("TestAdc");

static void adc_cache_cb(struct elab_adc *const me, float *buffer);

/* private functions -------------------------------------------------------- */
static int32_t test_adc_get_value(int32_t argc, char *argv[])
{
    int32_t ret = 0;
    elab_device_t * adc = elab_device_find("ADC_VOL");
    elab_assert(adc != NULL);
    
    float vol = elab_adc_get_value(adc);
    elog_debug("vol is %0.2f\n", vol);

    return ret;
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN),
                    test_adc_get,
                    test_adc_get_value,
                    ADC testing function);

static int32_t test_adc_auto(int32_t argc, char *argv[])
{
    int32_t ret = 0;
    static float vol = 0.0;
    elab_adc_attr_t attr =
    {
        .factor = (3.3 / 4096.0),
        .interval = 500
    };
    
    elab_device_t * adc = elab_device_find("ADC_VOL");
    elab_assert(adc != NULL);
    
    elab_adc_en_auto_read(adc, true);
    elab_adc_cache_start(adc, adc_cache_cb, &vol);
    elab_adc_set_attr(adc, &attr);
    
    osDelay(1000);
    elog_debug("vol is %0.2f\n", vol);
    
    return ret;
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN),
                    test_adc_auto,
                    test_adc_auto,
                    ADC testing function);

static void adc_cache_cb(struct elab_adc *const me, float *buffer)
{
   elab_assert(me != NULL);
   elab_assert(buffer != NULL);
   elab_assert(me->ops->get_value != NULL);
    
   uint32_t value = me->ops->get_value(me);
   *buffer = me->attr.factor * value;
}

#ifdef __cplusplus
}
#endif

/* ----------------------------- end of file -------------------------------- */
