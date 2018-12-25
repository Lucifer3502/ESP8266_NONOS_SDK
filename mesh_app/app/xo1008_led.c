
#include "honyar_common.h"
#include "xo1008_led.h"
#include "dl2106f.h"
#include "xo1008_device.h"

static uint8_t g_xo1008_led1_state;
static uint8_t g_xo1008_led2_state;
static os_timer_t g_xo1008_led_timer;

static uint8_t g_xo1008_led_work_mode;

static void ICACHE_FLASH_ATTR
xo1008_led1_on(void)
{
    honyar_gpio_set_output(XO1008_LED1_LOGIC_PIN, GPIO_LOW);
    g_xo1008_led1_state = LED_ON;
}

static void ICACHE_FLASH_ATTR
xo1008_led1_off(void)
{
    honyar_gpio_set_output(XO1008_LED1_LOGIC_PIN, GPIO_HIGH);
    g_xo1008_led1_state = LED_OFF;
}

static void ICACHE_FLASH_ATTR
xo1008_led1_reverse(void)
{
    if(LED_OFF == g_xo1008_led1_state) {
        xo1008_led1_on();
    } else {
        xo1008_led1_off();
    }
}

static void ICACHE_FLASH_ATTR
xo1008_led2_on(void)
{
    honyar_gpio_set_output(XO1008_LED2_LOGIC_PIN, GPIO_LOW);
    g_xo1008_led2_state = LED_ON;
}

static void ICACHE_FLASH_ATTR
xo1008_led2_off(void)
{
    honyar_gpio_set_output(XO1008_LED2_LOGIC_PIN, GPIO_HIGH);
    g_xo1008_led2_state = LED_OFF;
}

static void ICACHE_FLASH_ATTR
xo1008_led2_reverse(void)
{
    if(LED_OFF == g_xo1008_led2_state) {
        xo1008_led2_on();
    } else {
        xo1008_led2_off();
    }
}

#if 0
static void ICACHE_FLASH_ATTR
xo1008_led_sta_mode(void)
{
#define STA_NOT_CONNECT_INT  2000000
#define STA_GOT_IP_INT       200000

    static uint32_t last_run_time = 0;
    uint32_t cur_time = system_get_time();
    
    if(LED_ON == g_xo1008_led1_state) {
        xo1008_led1_off();
    }
    
    if(STATION_GOT_IP == wifi_station_get_connect_status()) {
        if(cur_time - last_run_time > STA_GOT_IP_INT) {
            xo1008_led2_reverse();
            last_run_time = cur_time;
        }
    } else {
        if(cur_time - last_run_time > STA_NOT_CONNECT_INT) {
            xo1008_led2_reverse();
            last_run_time = cur_time;
        }
    }
}
#else
static void ICACHE_FLASH_ATTR
xo1008_led_sta_mode(void)
{
#define STA_INT  2000000
    static uint32_t last_run_time = 0;
    uint32_t cur_time = system_get_time();
    if(cur_time - last_run_time > STA_INT) {
        last_run_time = cur_time;
        if(LED_OFF == g_xo1008_led1_state) {
            xo1008_led1_on();
            xo1008_led2_off();
        } else {
            xo1008_led1_off();
            xo1008_led2_on();
        }
    }
}

#endif

static void ICACHE_FLASH_ATTR
xo1008_led_mesh_mode(void)
{
#define MESH_IS_INVALID_INT 2000000
#define MESH_NETWORK_NOT_CONNET_INT  200000
    static uint32_t last_run_time = 0;
    uint32_t cur_time = system_get_time();
    #ifdef DL2106F
    uint8_t state = dl2106f_get_socket_power_state();
    #else
    uint8_t state = xo1008_device_get_power_state();
    #endif
    
    if(mesh_network_isconnected()) {
        if(POWER_ON == state) {
            xo1008_led1_on();
            if(espconn_mesh_is_root()) {
                xo1008_led2_on();
            }else {
                xo1008_led2_off();
            }
        } else {
            xo1008_led2_on();
            if(espconn_mesh_is_root()) {
                xo1008_led1_on();
            }else {
                xo1008_led1_off();
            }
        }
    } else if(honyar_mesh_is_valid()) {
        if(POWER_ON == state) {
            if(espconn_mesh_is_root()) {
                xo1008_led2_on();
            } else {
                xo1008_led2_off();
            }
            if(cur_time - last_run_time > MESH_NETWORK_NOT_CONNET_INT) {
                xo1008_led1_reverse();
                last_run_time = cur_time;
            }
        } else {
            if(espconn_mesh_is_root()) {
                xo1008_led1_on();
            }
            else {
                xo1008_led1_off();
            }
            if(cur_time - last_run_time > MESH_NETWORK_NOT_CONNET_INT) {
                xo1008_led2_reverse();
                last_run_time = cur_time;
            }
        }
    } else {
        if(POWER_ON == state) {
            xo1008_led2_off();
            if(cur_time - last_run_time > MESH_IS_INVALID_INT) {
                xo1008_led1_reverse();
                last_run_time = cur_time;
                
            }
        } else {
            xo1008_led1_off();
            if(cur_time - last_run_time > MESH_IS_INVALID_INT) {
                xo1008_led2_reverse();
                last_run_time = cur_time;
            }
        }
    }
}

static void ICACHE_FLASH_ATTR
xo1008_led_smartconfig_mode(void)
{
#define SMARTCONFIG_INT 200000
    static uint32_t last_run_time = 0;
    uint32_t cur_time = system_get_time();
    if(cur_time - last_run_time > SMARTCONFIG_INT) {
        last_run_time = cur_time;
        if(LED_OFF == g_xo1008_led1_state) {
            xo1008_led1_on();
            xo1008_led2_off();
        } else {
            xo1008_led1_off();
            xo1008_led2_on();
        }
    }
}

static void ICACHE_FLASH_ATTR
xo1008_led_task(void *arg)
{
    if(honyar_global_timer_is_disable()) {
        return;
    }
    
    switch(g_xo1008_led_work_mode) {
    case WIFI_STA_STATUS:
        xo1008_led_sta_mode();
        break;
    case WIFI_MESH_STATUS:
        xo1008_led_mesh_mode();
        break;
    case WIFI_SMARTCONFIG_STATUS:
        xo1008_led_smartconfig_mode();
        break;
    default:
        xo1008_led1_off();
        xo1008_led2_off();
        hy_error("unkown led mode.\r\n");
        return;
    }
    os_timer_arm(&g_xo1008_led_timer, 10, false);
}

void ICACHE_FLASH_ATTR
xo1008_led_set_work_mode(uint8_t mode)
{
    g_xo1008_led_work_mode = mode;
}

void ICACHE_FLASH_ATTR
xo1008_led_init(void)
{
    honyar_gpio_config(XO1008_LED1_LOGIC_PIN, GPIO_OUTPUT, PULL_NONE);
    honyar_gpio_config(XO1008_LED2_LOGIC_PIN, GPIO_OUTPUT, PULL_NONE);
    xo1008_led1_on();
    xo1008_led2_on();

    os_timer_disarm(&g_xo1008_led_timer);
    os_timer_setfn(&g_xo1008_led_timer, (os_timer_func_t *)xo1008_led_task, NULL);
    os_timer_arm(&g_xo1008_led_timer, 10, false);
}

