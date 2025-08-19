/* includes ----------------------------------------------------------------- */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "elab/common/elab_assert.h"
#include "elab/3rd/Shell/shell.h"
#include "elab/common/elab_log.h"
#include "elab/common/elab_export.h"
#include "elab/edf/user/elab_RSxxx.h"
#include "../export/export.h"
#include "elab/edf/user/elab_RSxxx.h"

#ifdef __cplusplus
extern "C" {
#endif

ELAB_TAG("TestSerial");

static uint8_t buff_rd;

/* private functions -------------------------------------------------------- */
/**
  * @brief Testing function for serial.
  * @retval None
  */
void RS232_uart1_poll(void) 
{
    RSxxx_t *handle = NULL;
    
	handle = get_RSxxx_handle("RS232_UART1");
    elab_assert(handle != NULL);
    
    int32_t rd_size = RSxxx_read(handle, &buff_rd, 1);
    
    if (rd_size > 0)
    {
        RSxxx_write(handle, &buff_rd, rd_size);
    }
    
}
//POLL_EXPORT(RS232_uart1_poll, 10);

#ifdef __cplusplus
}
#endif

/* ----------------------------- end of file -------------------------------- */
