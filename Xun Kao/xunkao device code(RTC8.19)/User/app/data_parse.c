#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include "data_parse.h"
#include "app.h"
#include "common/elab_assert.h"
#include "common/elab_log.h"
#include "elab/edf/elab_device.h"
#include "elab/edf/normal/elab_rtc.h"
#include "elab/common/elab_export.h"

ELAB_TAG("DataParse");

static char* get_current_timestamp(void);
static char *create_single_result(const char* uuid, char *name, int result);

//初始化钩子
void init_cjson_hooks(void)
{
    cJSON_Hooks cJSONhooks_freeRTOS;
    cJSONhooks_freeRTOS.malloc_fn = pvPortMalloc;
    cJSONhooks_freeRTOS.free_fn   = vPortFree;
    cJSON_InitHooks(&cJSONhooks_freeRTOS);
}

static char *create_single_result(const char* uuid, char *name, int result) 
{
    elab_assert(name != NULL);
   
     //创建root对象
    cJSON *root = cJSON_CreateObject();
    elab_assert(root != NULL);
    // 添加类型字段
    cJSON_AddStringToObject(root, CMD_TYPE, name);
    
    //创建数据data对象
    cJSON *data = cJSON_CreateObject();
    elab_assert(data != NULL);
    cJSON_AddStringToObject(data, DATA_UUID, uuid);
    
    cJSON_AddItemToObject(root, CMD_DATA, data);
    
    cJSON_AddNumberToObject(data, DATA_TORGUE, result);

    // 获取当前时间戳
    char* timestamp = get_current_timestamp();
    cJSON_AddStringToObject(data, DATA_TIME, timestamp);

    // 将 cJSON 对象转换为字符串
    //char *json_str = cJSON_Print(root);
    char *json_str = cJSON_PrintUnformatted(root);
    elab_assert(json_str != NULL);
    
    // 清理 cJSON 对象，防止内存泄漏
    cJSON_Delete(root);
    
    return json_str;
}

// 创建识别操作结果
char *create_button_result(const char* uuid, Button_t bnts[], int num, int optype){
    cJSON *root = cJSON_CreateObject();
    elab_assert(root != NULL);
    cJSON *data = cJSON_CreateObject();
    elab_assert(data != NULL);
    cJSON *button_presses = cJSON_CreateArray();
    elab_assert(button_presses != NULL);
    cJSON *button_item = NULL;
    
    // 添加 type 字段
    if (optype == CMD_TYPE_RECOGNITION)
    {
        cJSON_AddStringToObject(root, CMD_TYPE, RESULT_RECOGNITION);
    }
    else if (optype == CMD_TYPE_RESET)
    {
        cJSON_AddStringToObject(root, CMD_TYPE, RESULT_RESET);
    }
   
    //TODO: 这里会越界	
    cJSON_AddStringToObject(root, DATA_UUID, uuid);
    // 将 data 对象添加到 root 对象
    //cJSON_AddItemToObject(root, CMD_DATA, data);
    
    // 添加 total_buttons_pressed 字段
    cJSON_AddNumberToObject(root, DATA_BUTTON_NUM, num);
    // 将 button_presses 数组添加到 data 对象
    //cJSON_AddItemToObject(data, DATA_BUTTON_PRESSE, button_presses);
    int index[NUM_BUTTONS] = {0};
    int type[NUM_BUTTONS] = {0};
    
    for(int i = 0; i < num; i++)
    {
        index[i] = bnts[i].index;
        type[i] = bnts[i].type;
    }
    
    cJSON_AddItemToObject(root, DATA_BUTTON_ID, cJSON_CreateIntArray(index, num));
    cJSON_AddItemToObject(root, DATA_BUTTON_OP, cJSON_CreateIntArray(type, num));
    //向button_presses数组中添加元素
    /*
    for (uint8_t i = 0; i < num; i++) {
        button_item = cJSON_CreateObject();
        elab_assert(button_item != NULL);
        cJSON_AddNumberToObject(button_item, DATA_BUTTON_ID, ids[i]);
        cJSON_AddItemToArray(button_presses, button_item);
    }
    */
    
    // 获取当前时间戳
    //char* timestamp = get_current_timestamp();
    //cJSON_AddStringToObject(root, DATA_TIME, timestamp);
    
    // 将 cJSON 对象转换为字符串
    //char *json_str = cJSON_Print(root);
    char *json_str = cJSON_PrintUnformatted(root);
    elab_assert(json_str != NULL);
    //elog_debug("create_button_result: json_str %s\n", json_str);
    
    // 清理 cJSON 对象
    cJSON_Delete(root);

    return json_str;
}

// 创建拆装操作结果
char *create_assembly_result(const char *uuid, int result) {
    
    return create_single_result(uuid, RESULT_ASSEMBLY, result);
}

// 创建对中操作结果
char *create_alignment_result(const char *uuid, int result) {
    
    return create_single_result(uuid, RESULT_ALIGNMENT, result);
}

// 创建心跳请求
char *create_heartbeat_request(void) {
     //创建root对象
    cJSON *root = cJSON_CreateObject();
    elab_assert(root != NULL);
    
    // 添加类型字段
    cJSON_AddStringToObject(root, CMD_TYPE, HEARTBEAT_REQ);
    //创建数据data对象
    cJSON *data = cJSON_CreateObject();
    elab_assert(data != NULL);
    
    // 获取当前时间戳
    char* timestamp = get_current_timestamp();
    elab_assert(timestamp != NULL);
    cJSON_AddStringToObject(data, DATA_TIME, timestamp);
    
    // 将数据对象添加到根对象
    cJSON_AddItemToObject(root, CMD_DATA, data);

    // 将 cJSON 对象转换为字符串
    //char *json_str = cJSON_Print(root);
    char *json_str = cJSON_PrintUnformatted(root);
    elab_assert(json_str != NULL);
    //elog_debug("json_str is %s\n", json_str);
    
    // 清理 cJSON 对象，防止内存泄漏
    cJSON_Delete(root);
    
    return json_str;
}

// 创建HTTP Request
int create_http_request(char *request_buf, int buf_len, char *url, char *json_str)
{
    elab_assert(request_buf != NULL);
    elab_assert(json_str != NULL);
    elab_assert(url != NULL);
    elab_assert(buf_len > 0);
    
    int content_length = snprintf(request_buf, buf_len,
                                "POST %s HTTP/1.1\r\n"
                                "Host: %s\r\n"
                                "Content-Type: application/json; charset=UTF-8\r\n"              
                                "Content-Length: %d\r\n\r\n"
                                "%s",
                                 url, SERVER_IP, strlen(json_str), json_str);
    
    vPortFree(json_str);
								
	return content_length;
}



//解析命令类型
int parse_command_type(const char *json_str) 
{
	int ret = -1;
    elab_assert(json_str != NULL);
    
    //elog_debug("parse_command_type: %s\n", json_str);
    cJSON *json = cJSON_Parse(json_str);
    
    if (json == NULL) //非协议消息
    {
        //elog_debug("cJSON_Parse NULL:%s\n", cJSON_GetErrorPtr());
        elog_debug("parse_command_type: Err Raw JSON String:%s\n", json_str);
        return -1;
    }
    
    cJSON *type_item = cJSON_GetObjectItem(json, CMD_TYPE);
  
    if (type_item != NULL && type_item->valuestring != NULL)
    {
        if (strcmp(type_item->valuestring, CONNECT_INIT) == 0) 
            ret = CMD_TYPE_CONNECT;
        else if (strcmp(type_item->valuestring, HEARTBEAT_ACK) == 0) 
            ret = CMD_TYPE_HEARTBEAT;
        else if (strcmp(type_item->valuestring, COMMAND_RECOGNITION) == 0) 
            ret = CMD_TYPE_RECOGNITION;
        else if (strcmp(type_item->valuestring, COMMAND_RESET) == 0) 
            ret = CMD_TYPE_RESET;
        else if (strcmp(type_item->valuestring, COMMAND_ASSEMBLY) == 0)
            ret = CMD_TYPE_ASSEMBLY;
        else if (strcmp(type_item->valuestring, COMMAND_ALIGNMENT) == 0)
            ret = CMD_TYPE_ALIGNMENT;
        else if (strcmp(type_item->valuestring, COMMAND_END) == 0)
            ret = CMD_TYPE_END;
    }
    
    cJSON_Delete(json);
	return ret;
}

// 解析命令时长
int parse_command_duration(const char *json_str) 
{
    int duration = -1;
    elab_assert(json_str != NULL);
    
    cJSON *json = cJSON_Parse(json_str);
    if (json == NULL) //非协议消息
    {
        //elog_debug("cJSON_Parse NULL:%s\n", cJSON_GetErrorPtr());
        elog_debug("parse_command_duration: Err Raw JSON String:\n%s\n", json_str);
        return -1;
    }
    
    
    cJSON *data_item = cJSON_GetObjectItem(json, CMD_DATA);
    
    if (data_item != NULL)
    {
        // 提取 duration 字段的值
        cJSON *duration_item = cJSON_GetObjectItem(data_item, DATA_DURATION);
        if (duration_item != NULL)
            duration = duration_item->valueint;
        else
            elog_debug("parse_command_duration: duration_item is NULL%s\n");
    }

    cJSON_Delete(json);
	return duration;
}

// 解析UUDI
void parse_command_uuid(const char *json_str, char* uuid) 
{
    int duration = -1;
    char *ret = NULL;
    elab_assert(json_str != NULL);
    
    cJSON *json = cJSON_Parse(json_str);
    if (json == NULL) //非协议消息
    {
        //elog_debug("cJSON_Parse NULL:%s\n", cJSON_GetErrorPtr());
        elog_debug("parse_command_uuid: Err Raw JSON String:\n%s\n", json_str);
        return;
    }
    

    cJSON *data_item = cJSON_GetObjectItem(json, CMD_DATA);
    
    if (data_item != NULL)
    {
        // 提取 uuid 字段的值
        cJSON *uuid_item = cJSON_GetObjectItem(data_item, DATA_UUID);
        if (uuid_item != NULL && uuid_item->valuestring != NULL)
        {
            strncpy(uuid, uuid_item->valuestring, 63);
            uuid[63] = '0';
        }
    }

    cJSON_Delete(json);
}

//解析同步时间
int parse_command_syntime(const char *json_str) 
{
    int time = 0;
    elab_assert(json_str != NULL);
    
    cJSON *json = cJSON_Parse(json_str);
    if (json == NULL) //非协议消息
    {
        //elog_debug("cJSON_Parse NULL:%s\n", cJSON_GetErrorPtr());
        elog_debug("parse_command_syntime: Err Raw JSON String:\n%s\n", json_str);
        return -1;
    }
    
    cJSON *duration_item = cJSON_GetObjectItem(json, DATA_TIME);
    
    if (duration_item != NULL && duration_item->valuestring != NULL)
    {
        time = atoi(duration_item->valuestring);
    }

    cJSON_Delete(json);
	return time;
}

static char* get_current_timestamp(void)
{
    elab_device_t *dev = NULL;
    elab_rtc_time_t rt = {0};
    static char timestamp[64] = {0};
    
    dev = elab_device_find("rtc");
    elab_assert(dev != NULL);
    
    //获取当前时间
    elab_rtc_get_time(dev, &rt);
    
    struct tm time_struct = {
        .tm_sec = rt.time.second,
        .tm_min = rt.time.minute,
        .tm_hour = rt.time.hour,
        .tm_mday = rt.date.day,
        .tm_mon = rt.date.month - 1,    // tm_mon 是从0开始的（0表示1月）
        .tm_year = rt.date.year - 1900, // tm_year 是从1900年开始的
        .tm_isdst = 0                   // 不考虑夏令时
    };
    
//    sprintf(timestamp, "%04d-%02d-%02d %02d:%02d:%02d", 
//                        rt.date.year, rt.date.month, rt.date.day,
//                        rt.time.hour, rt.time.minute, rt.time.second); 

    sprintf(timestamp, "%d", mktime(&time_struct));
    
    return timestamp;
}
