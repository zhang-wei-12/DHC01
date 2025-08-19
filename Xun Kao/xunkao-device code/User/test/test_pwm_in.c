/* includes ----------------------------------------------------------------- */
#include <stdlib.h>
#include "elab/common/elab_assert.h"
#include "elab/3rd/Shell/shell.h"
#include "elab/common/elab_log.h"
#include "elab/edf/normal/elab_pwm_in.h"
#include "elab/common/elab_export.h"

#ifdef __cplusplus
extern "C" {
#endif

ELAB_TAG("TestPWMIN");

#define GAP 567.0  
#define FREQ_MID (10000.0-GAP)
#define FREQ_MIN (5000.0-GAP)
#define FREQ_MAX (15000.0-GAP)
#define IS_VALID_FREQ(freq)  (freq >= FREQ_MIN && freq <= FREQ_MAX)
    
#define CAL_TORQUE(freq, torque) \
do{ \
    if (freq >= FREQ_MID) \
        torque = (freq - FREQ_MID) / FREQ_MIN * 100; \
    else if (freq < FREQ_MID) \
        torque = (FREQ_MID - freq) / FREQ_MIN * (-100);\
}while(0) 

/* private functions -------------------------------------------------------- */
static int32_t test_pwm_in_freq(int32_t argc, char *argv[])
{
    int32_t ret = 0;
    elab_device_t * pwm_in = elab_device_find("PWM_IN");
    elab_assert(pwm_in != NULL);
    
    float freq = elab_pwm_in_freq(pwm_in);
    elog_debug("freq is %f\n", freq);
    
    float torque  = 0.0;
    if (IS_VALID_FREQ(freq))
    {
        CAL_TORQUE(freq, torque);
    }
    elog_debug("torque is %f\n", torque);

    return ret;
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN),
                    test_pwm_freq,
                    test_pwm_in_freq,
                    PWM IN testing function);

static int32_t test_pwm_in_duty(int32_t argc, char *argv[])
{
    int32_t ret = 0;
    elab_device_t * pwm_in = elab_device_find("PWM_IN");
    elab_assert(pwm_in != NULL);
    
    float duty = elab_pwm_in_duty(pwm_in);
    elog_debug("duty is %0f\n", duty);

    return ret;
    
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN),
                    test_pwm_duty,
                    test_pwm_in_duty,
                    PWM IN testing function);

#ifdef __cplusplus
}
#endif

/* ----------------------------- end of file -------------------------------- */
