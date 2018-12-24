
#include "at.h"
#include "honyar_common.h"


static at_write_func_t g_at_write_func = NULL;
static at_table_t *g_at_table;
static uint32_t g_table_size;

int32_t ICACHE_FLASH_ATTR
at_write(uint8_t *buf, uint32_t buf_len)
{
    if(g_at_write_func)
    {
        return g_at_write_func(buf, buf_len);
    }
}

void ICACHE_FLASH_ATTR
at_write_regist(at_write_func_t func)
{
    g_at_write_func = func;
}

static int32_t ICACHE_FLASH_ATTR
split_string_to_args(uint8_t *at_str, uint32_t str_len, uint32_t *pargc, uint8_t **argv)
{
    argv[0] = at_str;
    uint32_t argc = 1;
    uint32_t i = 1;

    for(i = 1; i < str_len; i++) {
	    /*The end*/
	    if('\r' == at_str[i] || '\n' == at_str[i]) {
            at_str[i] = '\0';
            break;
        }
        /*Ignore the space*/
        if(at_str[i] == ' ') {
            at_str[i]='\0';
            continue;	
		}
        /*New args*/
        if(('\0' != at_str[i]) && ('\0' == at_str[i - 1])) {
            argv[argc] = &at_str[i];
            argc++;
        }
        if(argc >= AT_MAX_OPTION_COUNT)
        {
            hy_error("AT too many args, more than %d\r\n", AT_MAX_OPTION_COUNT);
            return -1;
        }
    }
    *pargc=argc;

    return 0;
}

static int32_t ICACHE_FLASH_ATTR
parse_at_string(uint8_t *at_str, uint32_t str_len)
{
    uint32_t i;
    uint32_t argc = 0;
    uint8_t *argv[AT_MAX_OPTION_COUNT];
    
    memset(argv, 0, sizeof(argv[0]) * AT_MAX_OPTION_COUNT);
    if(split_string_to_args(at_str, str_len, &argc, argv)) {
        return -1;
    }
    
    hy_info("AT name: %s\r\n", argv[0]);

    for(i = 0; i < g_table_size; i++) {
        if(g_at_table[i].name && g_at_table[i].func){
            //dl_printf("cmd_table[%d]: %s\r\n", i,g_at_cmd_table[i].exe_cmd);
            if(!os_strcmp((char *)argv[0], (char *)(g_at_table[i].name)))
            {
                return (g_at_table[i].func)(argc, argv);
            }
        }
    }

    return -1;
}

static int32_t ICACHE_FLASH_ATTR
get_at_string(uint8_t *buf, uint32_t buf_len, uint8_t **name)
{
    uint32_t pos = 0;
    uint32_t pre_len = os_strlen(AT_PREFIX);
    
    if(buf_len <= pre_len) {
        return -1;
    }
    
    while(pos < buf_len - pre_len) {
        if(!os_memcmp(buf + pos, AT_PREFIX, pre_len)){
            pos += pre_len;
            *name = buf + pos;
            return 0;
        }
        pos++;
    }
    return 0;
}

int32_t ICACHE_FLASH_ATTR
at_recv_handle(uint8_t *buf, uint32_t buf_len)
{
    uint8_t *at = NULL;
    
    if('\r' == buf[0] || '\n' == buf[0]) {
        return -1;
    }
    
    if(get_at_string(buf, buf_len, &at)) {
        AT_PRINTF(AT_ERR);
        return -1;
    }
    if(parse_at_string(at, buf_len - (at - buf))) {
        AT_PRINTF(AT_ERR);
        return -1;
    }

    AT_PRINTF(AT_OK);
    return 0;
}



int32_t ICACHE_FLASH_ATTR
at_table_regist(at_table_t *table, uint32_t size)
{
    if(!table) {
        return -1;
    }
    g_at_table = table;
    g_table_size = size;
    return 0;
}


void ICACHE_FLASH_ATTR
at_table_init(void)
{
    g_at_table = NULL;
    g_table_size = 0;
}


