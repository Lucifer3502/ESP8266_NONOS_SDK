
#include "honyar_common.h"

#define HONYAR_PLATFORM_DEBUG_BAUDRATE  115200

struct honyar_task{
    uint32_t enable;
    task_func_t func;
    void *parm;
    uint32_t start;
    uint32_t cycle;
};

static struct honyar_task g_honyar_tasks[TASK_MAX_NUM];
static os_timer_t g_honyar_platform_timer;

int32_t ICACHE_FLASH_ATTR honyar_add_task(task_func_t func, void *parm, uint32_t cycle)
{
    uint32_t i = 0;
    for(i = 0; i < TASK_MAX_NUM; i++) {
        if(g_honyar_tasks[i].enable) {
            continue;
        }
        g_honyar_tasks[i].cycle = cycle;
        g_honyar_tasks[i].func = func;
        g_honyar_tasks[i].parm = parm;
        g_honyar_tasks[i].start = 0;
        g_honyar_tasks[i].enable = 1;
        return 0;
    }
    return -1;
}

int32_t ICACHE_FLASH_ATTR honyar_del_task(task_func_t func)
{
    uint32_t i = 0;
    for(i = 0; i < TASK_MAX_NUM; i++) {
        if(g_honyar_tasks[i].func == func) {
            g_honyar_tasks[i].enable = 0;
            g_honyar_tasks[i].func = NULL;
            g_honyar_tasks[i].parm = NULL;
            g_honyar_tasks[i].cycle = 0;
            g_honyar_tasks[i].start = 0;
            return 0;
        }
    }
    return -1;
}

int32_t ICACHE_FLASH_ATTR honyar_pause_task(task_func_t func)
{
    uint32_t i = 0;
    for(i = 0; i < TASK_MAX_NUM; i++) {
        if(g_honyar_tasks[i].func == func) {
            g_honyar_tasks[i].enable = 0;
            return 0;
        }
    }
    
    return -1;
}

int32_t ICACHE_FLASH_ATTR honyar_continue_task(task_func_t func)
{
    uint32_t i = 0;
    for(i = 0; i < TASK_MAX_NUM; i++) {
        if(g_honyar_tasks[i].func == func) {
            g_honyar_tasks[i].enable = 1;
            return 0;
        }
    }
    
    return -1;
}

static void ICACHE_FLASH_ATTR honyar_task(void *arg)
{
    uint32_t i = 0;
    static uint32_t cycle = 0;
    
    for(i = 0; i < TASK_MAX_NUM; i++) {
        if(0 == g_honyar_tasks[i].enable) {
            continue;
        }
        
        if((cycle - g_honyar_tasks[i].start < g_honyar_tasks[i].cycle) && g_honyar_tasks[i].start) {
            continue;
        }
        
        if(g_honyar_tasks[i].func) {
            g_honyar_tasks[i].func(g_honyar_tasks[i].parm);
            g_honyar_tasks[i].start = cycle;
        }
    }
    cycle++;
    system_soft_wdt_feed();
    os_timer_arm(&g_honyar_platform_timer, TASK_CYCLE_TM_MS, false);
}

static void ICACHE_FLASH_ATTR _honyar_platform_init(void)
{
    UART_SetBaudrate(1, HONYAR_PLATFORM_DEBUG_BAUDRATE);
    system_soft_wdt_restart();
}

void ICACHE_FLASH_ATTR honyar_platform_init(void)
{
    _honyar_platform_init();
    
    os_timer_disarm(&g_honyar_platform_timer);
    os_timer_setfn(&g_honyar_platform_timer, (os_timer_func_t *)honyar_task, NULL);
    os_timer_arm(&g_honyar_platform_timer, TASK_CYCLE_TM_MS, false);
}


uint32 ICACHE_FLASH_ATTR
user_rf_cal_sector_set(void)
{
    enum flash_size_map size_map = system_get_flash_size_map();
    uint32 rf_cal_sec = 0;

    switch (size_map) {
        case FLASH_SIZE_4M_MAP_256_256:
            rf_cal_sec = 128 - 5;
            break;

        case FLASH_SIZE_8M_MAP_512_512:
            rf_cal_sec = 256 - 5;
            break;

        case FLASH_SIZE_16M_MAP_512_512:
        case FLASH_SIZE_16M_MAP_1024_1024:
            rf_cal_sec = 512 - 5;
            break;

        case FLASH_SIZE_32M_MAP_512_512:
        case FLASH_SIZE_32M_MAP_1024_1024:
            rf_cal_sec = 1024 - 5;
            break;

        case FLASH_SIZE_64M_MAP_1024_1024:
            rf_cal_sec = 2048 - 5;
            break;
        case FLASH_SIZE_128M_MAP_1024_1024:
            rf_cal_sec = 4096 - 5;
            break;
        default:
            rf_cal_sec = 0;
            break;
    }

    return rf_cal_sec;
}

void ICACHE_FLASH_ATTR user_rf_pre_init(void)
{}

void ICACHE_FLASH_ATTR user_pre_init(void)
{}

