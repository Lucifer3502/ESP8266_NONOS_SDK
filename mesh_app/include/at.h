
#ifndef _AT_H_
#define _AT_H_

#include "honyar_types.h"

/*参数个数*/
#define AT_MAX_OPTION_COUNT     (10)
/*命令长度*/
#define AT_MAX_LEN              (256)
/*支持AT指令集个数*/
#define AT_TABLE_MAX_COUNT      (100)

/*格式et. :  
1. 每个指令必须用AT_PREFIX开头，后面接at 指令
2. 参数之间用空格隔开，参数最多AT_MAX_OPTION_COUNT
AT#REBOOT   //系统复位
AT#CMD argv[1] argv[2] ...
*/
#define AT_PREFIX               "AT#"
#define AT_OK                   "OK\r\n"
#define AT_ERR                  "ERR\r\n"

#define AT_PRINTF(fmt, args...) do \
{\
    uint8_t buf[256] = {0};\
    at_write(buf, (uint32_t)os_sprintf((char *)buf, fmt, ##args));\
} while(0 == 1)\



typedef struct{
    uint8_t *name;
    int32_t (*func)(uint32_t argc, uint8_t *argv[]);
}at_table_t;

typedef int32_t (*at_write_func_t)(uint8_t *buf, uint32_t buf_len);

int32_t at_write(uint8_t *buf, uint32_t buf_len);

void at_write_regist(at_write_func_t func);

int32_t at_recv_handle(uint8_t *buf, uint32_t buf_len);

int32_t at_table_regist(at_table_t *tables, uint32_t size);

void at_table_init(void);

#endif

