/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "bsp/bsp.h"
#include "elab/common/elab_export.h"
#include "app/data_parse.h"
#include "sys_arch.h"
#include "stm32f4xx.h"

/* public functions --------------------------------------------------------- */
/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{   
    bsp_init();
    elab_run();
}

/* ----------------------------- end of file -------------------------------- */
