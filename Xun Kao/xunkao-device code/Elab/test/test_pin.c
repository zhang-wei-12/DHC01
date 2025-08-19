/*
 * eLesson Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* includes ----------------------------------------------------------------- */
#include <string.h>
#include <stdio.h>
#include "../edf/normal/elab_pin.h"
#include "../3rd/Shell/shell.h"
#include "../common/elab_log.h"

ELAB_TAG("PinTest");

/**
  * @brief  PIN device set testing.
  * @param  argc - argument count
  * @param  argv - argument variant
  * @retval execute result
  */
static int test_pin_set(int argc, char *argv[])
{
    int ret = 0;
    bool status = true;
    elab_device_t *dev = NULL;

    if (argc != 3)
    {
        elog_error("Not right argument number: %u. It should be 3.", argc);
        ret = -1;
        goto exit;
    }

    if (!elab_device_valid(argv[1]))
    {
        elog_error("Not right device name: %s.", argv[1]);
        ret = -2;
        goto exit;
    }

    if (strcmp(argv[2], "0") != 0 && strcmp(argv[2], "1") != 0)
    {
        elog_error("Not right device status: %s.", argv[2]);
        ret = -3;
        goto exit;
    }

    status = strcmp(argv[2], "0") == 0 ? false : true;
    dev = elab_device_find(argv[1]);
    elab_pin_set_status(dev, status);

exit:
    if (ret != 0)
    {
        elog_debug("The command example:\n    test_pin_set pin_name 1\n");
    }
    return ret;
}

/**
  * @brief  gpio test read
  * @param  argc - argument count
  * @param  argv - argument variant
  * @retval execute result
  */
static int test_pin_get(int argc, char *argv[])
{
    int ret = 0;
    bool status = true;
    elab_device_t *dev = NULL;

    if (argc != 2)
    {
        elog_error("Not right argument number: %u. It should be 2.", argc);
        ret = -1;
        goto exit;
    }

    if (!elab_device_valid(argv[1]))
    {
        elog_error("Not right device name: %s.", argv[1]);
        ret = -2;
        goto exit;
    }

    dev = elab_device_find(argv[1]);
    status = elab_pin_get_status(dev);
    printf("PIN device %s status is %u.\n", argv[1], status);

exit:
    if (ret != 0)
    {
        elog_debug("The command example:\n    test_pin_get pin_name\n");
    }
    return ret;
}

/**
  * @brief  Export the shell test command
  */
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN),
                    test_pin_set,
                    test_pin_set,
                    PIN set testing function);

/**
  * @brief  Export the shell test command
  */
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN),
                    test_pin_get,
                    test_pin_get,
                    PIN status get testing function);

/**************** (C) COPYRIGHT Philips Healthcare Suzhou ******END OF FILE****/
