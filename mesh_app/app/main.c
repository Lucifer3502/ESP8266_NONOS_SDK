
#include "honyar_common.h"
#include "app_config.h"
#include "mesh_app.h"
#include "xo1008_net.h"
#include "xo1008_uart.h"
#include "xo1008_key.h"
#include "xo1008_led.h"
#include "xo1008_device.h"
#include "xo1008_upgrade.h"


static void ICACHE_FLASH_ATTR
wifi_station_cb(uint8_t status)
{
    if(status == STATION_GOT_IP){
        hy_info("wifi connected\r\n");
    } else {
        hy_info("wifi status: %d\r\n", status);
    }
}

static void ICACHE_FLASH_ATTR
user_config_regist(void)
{
    xo1008_upgrade_config_init();
}

static void ICACHE_FLASH_ATTR
user_wifi_init(void *parm)
{
    uint8_t status = honyar_wifi_get_work_status();
    honyar_wifi_station_regist_statuscb(wifi_station_cb);
    honyar_wifi_init();
    
    if(WIFI_MESH_STATUS == status) {
        xo1008_net_init();
        xo1008_led_set_work_mode(WIFI_MESH_STATUS);
    } else if(WIFI_STA_STATUS == status) {
        honyar_wifi_station_start(honyar_wifi_get_router_ssid(), honyar_wifi_get_router_passwd());
        xo1008_upgrade_init();
        xo1008_led_set_work_mode(WIFI_STA_STATUS);
    } else if(WIFI_SMARTCONFIG_STATUS == status) {
        honyar_ilink_init();
        honyar_wifi_set_work_status(WIFI_MESH_STATUS);
        dl_config_commit_later();
        dl_config_commit(0);
        xo1008_led_set_work_mode(WIFI_SMARTCONFIG_STATUS);
    } else {
        hy_error("Invalid wifi work status, status = %d\r\n", status);
        honyar_wifi_set_work_status(WIFI_MESH_STATUS);
        dl_config_commit(1);
        honyar_sys_reboot(0);
    }

    honyar_del_task(user_wifi_init);
}

void ICACHE_FLASH_ATTR
user_init(void)
{
    honyar_platform_init();

    hy_info("app version: [%s], compile time: [%s - %s]\r\n", APP_VERSION, __DATE__, __TIME__);
    
    user_config_regist();
    dl_config_init();
#if 0
    xo1008_led_init();

    xo1008_key_init();
#endif
    xo1008_uart_init();
    
    honyar_add_task(user_wifi_init, NULL, 0);
}

