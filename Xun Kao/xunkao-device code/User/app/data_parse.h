#ifndef DATA_PARSE_H
#define DATA_PARSE_H

#include "cJSON.h"

// 定义命令类型
typedef enum {
    CMD_TYPE_CONNECT     = 0,
    CMD_TYPE_HEARTBEAT,
    CMD_TYPE_END,             //结束当前操作
    CMD_TYPE_RECOGNITION,     //识别
    CMD_TYPE_RESET,           //复位
    CMD_TYPE_ASSEMBLY,        //拆装
    CMD_TYPE_ALIGNMENT        //对中
}CMD_TYPE_T;

#define CMD_TYPE               "type"
#define CMD_DATA               "data"

#define DATA_TIME              "timestamp"
#define DATA_BUTTON_NUM        "num"
#define DATA_BUTTON_ID         "id"
#define DATA_BUTTON_OP         "op"

#define DATA_UUID              "uuid"

#define DATA_DURATION           "duration"
#define DATA_TORGUE             "torque_value"

#define CONNECT_INIT           "init"         //连接握手
#define HEARTBEAT_REQ  		   "heartbeat"
#define COMMAND_END            "command_end"
#define COMMAND_RECOGNITION    "command_recognition"
#define COMMAND_RESET          "command_reset"
#define COMMAND_ASSEMBLY       "command_assembly"
#define COMMAND_ALIGNMENT      "command_alignment"


#define HEARTBEAT_ACK  		   "heartbeat_ack"
#define RESULT_RECOGNITION     "result_recognition"
#define RESULT_RESET           "result_reset"
#define RESULT_ASSEMBLY        "result_assembly"
#define RESULT_ALIGNMENT       "result_alignment"

typedef struct
{
    unsigned char index; // 按钮编号
    unsigned char type;  //操作类型
    unsigned char seqs;  // 按下顺序
}Button_t;

//初始化钩子
void init_cjson_hooks(void);

//创建识别操作结果
char *create_button_result(const char* uuid, Button_t bnts[], int num, int type);
// 创建拆装操作结果
char *create_assembly_result(const char* uuid, int result);
// 创建对中操作结果
char *create_alignment_result(const char* uuid, int result);
// 创建心跳请求
char *create_heartbeat_request(void);
// 创建HTTP Request
int create_http_request(char *request_buf, int buf_len, char *url, char *json_str);

//解析命令类型
int parse_command_type(const char *json_str);
//解析命令时长
int parse_command_duration(const char *json_str);
//解析同步时间
int parse_command_syntime(const char *json_str);
// 解析UUDI
void parse_command_uuid(const char *json_str, char* uuid);

#endif
