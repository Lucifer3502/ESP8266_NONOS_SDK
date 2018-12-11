#include "honyar_common.h"
#include "xo1008_key.h"

//UNIT: us
#define SHORT_PRESS_TIME_MIN  30000
#define SHORT_PRESS_TIME_MAX  1000000
#define LONG_PRESS_TIME_MIN   3000000

static os_timer_t g_xo1008_key_timer;

static uint8_t ICACHE_FLASH_ATTR
xo1008_key_state(void)
{
    if(GPIO_HIGH == honyar_gpio_get_input(XO1008_KEY_LOGIC_PIN)) {
        return PRESS_UP;
    }
    return PRESS_DOWN;
}

static void ICACHE_FLASH_ATTR
xo1008_key_task(void *arg)
{
    static uint8_t key_state_his = 0;
    uint8_t key_state_cur = xo1008_key_state();
    static uint32_t press_time = 0;
    uint32_t cur_time = system_get_time();
    
    if(honyar_global_timer_is_disable()) {
        return;
    }
    if(key_state_his == key_state_cur) {
        if(PRESS_DOWN == key_state_his && cur_time - press_time >= LONG_PRESS_TIME_MIN) {
            // long press;
            hy_info("long press\r\n");
            honyar_wifi_set_work_status(WIFI_SMARTCONFIG_STATUS);
            dl_config_commit_later();
            dl_config_commit(1);
            honyar_sys_reboot(0);
            return;
        }
    } else if(PRESS_DOWN == key_state_cur) {
        press_time = system_get_time();
    } else {
        if(cur_time - press_time < SHORT_PRESS_TIME_MIN) {
            //ignore; maybe incorrect operation 
        } else if(cur_time - press_time < SHORT_PRESS_TIME_MAX) {
            // short press;
            hy_info("short press\r\n");
        }
    }
    
    key_state_his = key_state_cur;
AGAIN:
    os_timer_arm(&g_xo1008_key_timer, 10, false);
}

    
void ICACHE_FLASH_ATTR
xo1008_key_init(void)
{
    honyar_gpio_config(XO1008_KEY_LOGIC_PIN, GPIO_INTPUT, PULL_NONE);

    os_timer_disarm(&g_xo1008_key_timer);
    os_timer_setfn(&g_xo1008_key_timer, (os_timer_func_t *)xo1008_key_task, NULL);
    os_timer_arm(&g_xo1008_key_timer, 10, false);
}
