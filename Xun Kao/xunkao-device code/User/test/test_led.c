/* includes ----------------------------------------------------------------- */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "elab/common/elab_assert.h"
#include "elab/3rd/Shell/shell.h"
#include "elab/common/elab_log.h"
#include "elab/edf/user/elab_led.h"

#ifdef __cplusplus
extern "C" {
#endif

ELAB_TAG("TestLed");

/* private functions -------------------------------------------------------- */
/**
  * @brief Testing function for led toggle.
  * @retval None
  */
static int32_t test_led_toggle(int32_t argc, char *argv[])
{
    int32_t ret = 0;

    if (argc != 3)
    {
        elog_error("Not right argument number: %u. It should be 3.", argc);
        ret = -1;
        goto exit;
    }
    
    if (strcmp(argv[1], "led_R") != 0 
        && strcmp(argv[1], "led_G") != 0 
    && strcmp(argv[1], "led_B") != 0)
    {
        elog_error("Not right device name: %s.", argv[1]);
        ret = -2;
        goto exit;
    }
    
    if (strcmp(argv[2], "100") != 0
        && strcmp(argv[2], "200") != 0
        && strcmp(argv[2], "500") != 0
        && strcmp(argv[2], "1000") != 0)
    {
        elog_error("Not right device value: %s, Should be 100 or 200 or 500 or 1000", argv[2]);
        ret = -3;
        goto exit;
    }
    
    elab_led_toggle(elab_device_find(argv[1]), atoi(argv[2]));
    
exit:
    return ret;
}

/* private functions -------------------------------------------------------- */
/**
  * @brief Testing function for led value.
  * @retval None
  */
static int32_t test_led_value(int32_t argc, char *argv[])
{
    int32_t ret = 0;

    if (argc != 3)
    {
        elog_error("Not right argument number: %u. It should be 3.", argc);
        ret = -1;
        goto exit;
    }
    
    if (strcmp(argv[1], "led_R") != 0 
        && strcmp(argv[1], "led_G") != 0 
        && strcmp(argv[1], "led_B") != 0)
    {
        elog_error("Not right device name: %s.", argv[1]);
        ret = -2;
        goto exit;
    }
    
    elab_assert(atoi(argv[2]) <= UINT8_MAX);
    elab_led_set_value(elab_device_find(argv[1]), atoi(argv[2]));
    
exit:
    return ret;
}

/* private functions -------------------------------------------------------- */
/**
  * @brief Testing function for led clear.
  * @retval None
  */
static int32_t test_led_clear(int32_t argc, char *argv[])
{
    int32_t ret = 0;
    
    if (argc != 2)
    {
        elog_error("Not right argument number: %u. It should be 2.", argc);
        ret = -1;
        goto exit;
    }
    
    if (strcmp(argv[1], "led_R") != 0 
        && strcmp(argv[1], "led_G") != 0 
    && strcmp(argv[1], "led_B") != 0)
    {
        elog_error("Not right device name: %s.", argv[1]);
        ret = -2;
        goto exit;
    }
    
    elab_led_clear(elab_device_find(argv[1]));
    
    exit:
    return ret;
}

SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN),
                    test_led_toggle,
                    test_led_toggle,
                    LED testing function);

SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN),
                    test_led_value,
                    test_led_value,
                    LED testing function);

SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN),
                    test_led_clear,
                    test_led_clear,
                    LED testing function);


#ifdef __cplusplus
}
#endif

/* ----------------------------- end of file -------------------------------- */
