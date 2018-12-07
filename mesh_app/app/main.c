#include "honyar_common.h"
#include "mesh_app.h"
#include "xo1008_net.h"
#include "xo1008_uart.h"



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
    DL_CONFIG_ITEM_S config_items[] = 
	{
        
	};

	uint32_t i;
	for (i = 0; i < sizeof(config_items)/sizeof(config_items[0]); i++)
	{
        dl_config_items_register_by_user(&config_items[i]);
    }
}

static void ICACHE_FLASH_ATTR
user_wifi_init(void *parm)
{
    uint8_t status = honyar_wifi_get_work_status();
    honyar_wifi_init();
    honyar_wifi_station_regist_statuscb(wifi_station_cb);

    if(WIFI_MESH_STATUS == status) {
        xo1008_net_init();
    } else if(WIFI_STA_STATUS == status) {
        honyar_wifi_station_start(honyar_wifi_get_router_ssid(), honyar_wifi_get_router_passwd());
    } else if(WIFI_SMARTCONFIG_STATUS == status) {
        honyar_ilink_init();
        honyar_wifi_set_work_status(WIFI_MESH_STATUS);
        dl_config_commit_later();
        dl_config_commit(0);
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
    
    user_config_regist();
    dl_config_init();

    xo1008_uart_init();
    
    honyar_add_task(user_wifi_init, NULL, 0);

}

