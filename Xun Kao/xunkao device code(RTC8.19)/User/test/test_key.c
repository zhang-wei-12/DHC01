/* includes ----------------------------------------------------------------- */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "elab/common/elab_assert.h"
#include "elab/3rd/Shell/shell.h"
#include "elab/common/elab_log.h"
#include "elab/edf/user/elab_button.h"

#include "../driver/drv_button.h"
#include "../driver/drv_pin.h"

#ifdef __cplusplus
extern "C" {
#endif

ELAB_TAG("TestKey");

//引用按钮设备
extern pin_key_src_t _pin_key_src[MAX_KEY_IN];

/* private function prototype ----------------------------------------------- */
static void _button_cb(elab_button_t *const me, uint8_t event_id);

/* private variables -------------------------------------------------------- */
static bool button_KEY = false;

static osSemaphoreId_t sem = NULL;

/* private functions -------------------------------------------------------- */
/**
  * @brief Testing function for led toggle.
  * @retval None
  */
static int32_t test_key(int32_t argc, char *argv[])
{
    int32_t ret = 0;
    
    if (argc != 2)
    {
        elog_error("Not right argument number: %u. It should be 2, such as test_key key_no.", argc);
        ret = -1;
        goto exit;
    }
        
    sem = osSemaphoreNew(1, 0, NULL);
    elab_assert(sem != NULL);
    
    elab_button_set_event_callback(elab_device_find(argv[1]), _button_cb);
    
    /* Button testing. */
    elog_debug("Buttons testing starts. --------------------------------\r\n");
    for(int i = 0; i < MAX_KEY_IN; i++)
    {
        if (strcmp(argv[1], _pin_key_src[i].key_no) == 0)
        {
            button_KEY = true;
            elog_debug("Please press the %s button!\r\n", _pin_key_src[i].key_no);
            osSemaphoreAcquire(sem, osWaitForever);
            button_KEY = false;
        }
    }
    
    elab_button_set_event_callback(elab_device_find(argv[1]), NULL);
   
exit:
    return ret;
}

static void _button_cb(elab_button_t *const me, uint8_t event_id)
{
    elab_device_t *dev = ELAB_DEVICE_CAST(me);

    //if (event_id == ELAB_BUTTON_EVT_CLICK)
    if (event_id == ELAB_BUTTON_EVT_PRESSED)
    {
        for(int i = 0; i < MAX_KEY_IN; i++)
        {
            if (button_KEY && elab_device_of_name(dev, _pin_key_src[i].key_no))
            {
                elog_debug("%s pressed ......!\r\n", _pin_key_src[i].key_no);
                osSemaphoreRelease(sem);
            }
        }
    }
}

SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN),
                    test_key,
                    test_key,
                    KEY testing function);

#ifdef __cplusplus
}
#endif

/* ----------------------------- end of file -------------------------------- */
