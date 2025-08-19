/* includes ----------------------------------------------------------------- */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "elab/common/elab_assert.h"
#include "elab/3rd/Shell/shell.h"
#include "elab/common/elab_log.h"
#include "elab/edf/normal/elab_watchdog.h"

#ifdef __cplusplus
extern "C" {
#endif

ELAB_TAG("TestWatchdog");

/* private functions -------------------------------------------------------- */
/**
  * @brief Testing function for watchdog.
  * @retval None
  */
static int32_t test_wdg(int32_t argc, char *argv[])
{
    int32_t ret = 0;

    elab_assert(false);//断言后,触发看门狗重启程序
   
    return ret;
}

SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN),
                    test_wdg,
                    test_wdg,
                    Watchdog testing function);


#ifdef __cplusplus
}

#endif

/* ----------------------------- end of file -------------------------------- */
