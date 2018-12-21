#include "honyar_common.h"
#include "xo1008_key.h"

//UNIT: us
#define SHORT_PRESS_TIME_MIN  30000
#define SHORT_PRESS_TIME_MAX  1000000
#define LONG_PRESS_TIME_MIN   3000000
#define LONG_PRESS_TIME_MAX   6000000
#define LONGLONG_PRESS_TIME_MIN   12000000


static os_timer_t g_xo1008_key_timer;

static uint8_t ICACHE_FLASH_ATTR
xo1008_key_state(void)
{
#ifdef DL2106F
    if(GPIO_LOW == honyar_gpio_get_input(XO1008_KEY_LOGIC_PIN)) {
#else
    if(GPIO_HIGH == honyar_gpio_get_input(XO1008_KEY_LOGIC_PIN)) {
#endif
        return PRESS_UP;
    }
    return PRESS_DOWN;
}

static void ICACHE_FLASH_ATTR
xo1008_key_task(void *arg)
{
    static uint8_t key_state_his = PRESS_UP;
    uint8_t key_state_cur = xo1008_key_state();
    static uint32_t press_time = 0;
    uint32_t cur_time = system_get_time();
    
    if(honyar_global_timer_is_disable()) {
        return;
    }
    if(key_state_his == key_state_cur) {
        if(PRESS_DOWN == key_state_his && cur_time - press_time >= LONGLONG_PRESS_TIME_MIN) {
            hy_info("long long press\r\n");
            honyar_wifi_set_router_ssid(WIFI_SSID_DEF);
            honyar_wifi_set_router_passwd(WIFI_PWD_DEF);
            honyar_wifi_set_work_status(WIFI_STA_STATUS);
            dl_config_commit_later();
            dl_config_commit(0);
            honyar_sys_reboot(0);
        }
    } else if(PRESS_DOWN == key_state_cur) {
        hy_info("press event\r\n");
        press_time = system_get_time();
    } else {
        if(cur_time - press_time < SHORT_PRESS_TIME_MIN) {
            hy_info("too short\r\n");
            //ignore; maybe incorrect operation 
        } else if(cur_time - press_time < SHORT_PRESS_TIME_MAX) {
            // short press;
            hy_info("short press\r\n");
#ifdef DL2106F
            dl2106f_set_socket_power_reverse();
#endif
        } else if((cur_time - press_time < LONG_PRESS_TIME_MAX)
                && (cur_time - press_time >= LONG_PRESS_TIME_MIN)) {
            // long press;
            hy_info("long press\r\n");
            honyar_wifi_set_work_status(WIFI_SMARTCONFIG_STATUS);
            dl_config_commit_later();
            dl_config_commit(1);
            honyar_sys_reboot(0);
            return;
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

