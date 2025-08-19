/* includes ----------------------------------------------------------------- */
#include "elab/3rd/qpc/include/qpc.h"
#include "elab/os/cmsis_os.h"
#include "elab/common/elab_export.h"
#include "elab/common/elab_assert.h"
#include "elab/3rd/Shell/shell.h"
#include "elab/common/elab_log.h"

#include "elab/edf/elab_device.h"
#include "elab/edf/user/elab_button.h"
#include "elab/edf/user/elab_led.h"
#include "config/event_def.h"
#include "../user/app/app.h"
#include "../user/app/data_parse.h"

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

ELAB_TAG("TestOperater");
//Q_DEFINE_THIS_MODULE("TestOperater")

/* public functions --------------------------------------------------------- */
static int32_t test_recon(int32_t argc, char *argv[])
{
    int32_t ret = 0;
    uint32_t seconds = 0;
    
    if (argc != 2)
    {
        elog_error("Not right argument number: %u. It should be 2, such as test_recon secs.", argc);
        ret = -1;
        goto exit;
    }
    
    seconds = atoi(argv[1]);
    
    CmdOperationEvt *CmdOpEvt = Q_NEW(CmdOperationEvt, COMMAND_OPERATION_SIG);
    CmdOpEvt->type = CMD_TYPE_RECOGNITION;
    CmdOpEvt->duration= seconds;
    QACTIVE_POST(AO_Operater, (QEvt *)CmdOpEvt, 0);
    
exit:
    return ret;
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN),
                    test_recon,
                    test_recon,
                    Recon testing function);


static int32_t test_assem(int32_t argc, char *argv[])
{
    int32_t ret = 0;
    uint32_t seconds = 0;
    
    
    if (argc != 2)
    {
        elog_error("Not right argument number: %u. It should be 2, such as test_assem secs.", argc);
        ret = -1;
        goto exit;
    }
    
    seconds = atoi(argv[1]);
    
    CmdOperationEvt *CmdOpEvt = Q_NEW(CmdOperationEvt, COMMAND_OPERATION_SIG);
    CmdOpEvt->type = CMD_TYPE_ASSEMBLY;
    CmdOpEvt->duration= seconds;
    QACTIVE_POST(AO_Operater, (QEvt *)CmdOpEvt, 0);
 
exit:
    return ret;
}

SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN),
                    test_assem,
                    test_assem,
                    Assem testing function);

static int32_t test_reset(int32_t argc, char *argv[])
{
    int32_t ret = 0;
    uint32_t seconds = 0;
    
    if (argc != 2)
    {
        elog_error("Not right argument number: %u. It should be 2, such as test_recon secs.", argc);
        ret = -1;
        goto exit;
    }
    
    seconds = atoi(argv[1]);
    
    CmdOperationEvt *CmdOpEvt = Q_NEW(CmdOperationEvt, COMMAND_OPERATION_SIG);
    CmdOpEvt->type = CMD_TYPE_RESET;
    CmdOpEvt->duration= seconds;
    QACTIVE_POST(AO_Operater, (QEvt *)CmdOpEvt, 0);
    
exit:
    return ret;
}

SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN),
                    test_reset,
                    test_reset,
                    Reset testing function);

static int32_t test_end(int32_t argc, char *argv[])
{
    int32_t ret = 0;
    
    QF_publish_(Q_NEW(QEvt, END_OPERATION_SIG));
    
exit:
    return ret;
}

SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN),
                    test_end,
                    test_end,
                    End testing function);


static int32_t test_parse(int32_t argc, char *argv[])
{
    int32_t ret = 0;
    const char* str = "{\"type\":\"command_recognition\",\"data\":{\"duration\":556,\"timestamp\":1747030569,\"uuid\":\"515ea75e-b7e3-496f-aac1-8109b62859ee\"}}";
    
    int type = parse_command_type(str);
    elog_debug("type:%d!\n", type);	
    
exit:
    return ret;
}

SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN),
                    test_parse,
                    test_parse,
                    Parse testing function);

#ifdef __cplusplus
}
#endif

/* ----------------------------- end of file -------------------------------- */
