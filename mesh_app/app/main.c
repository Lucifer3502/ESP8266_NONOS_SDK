#include "honyar_common.h"
#include "mesh_app.h"
#include "uart_app.h"


#define UPGRADE_FILE_NAME "XO1007_OTA_FROM_V1.0.0_TO_V1.0.0.bin"


static void ICACHE_FLASH_ATTR http_upgrade_try(void)
{
    static uint8_t flag = 1;
    http_upgrade_info_t info;
    
    if(flag) {
        return;
    }
    flag = 1;
    memset(&info, 0, sizeof(info));
    memcpy(info.host, "192.168.0.3", NET_IP_ADDR_LEN);
    info.port = 80;
    memcpy(info.file, UPGRADE_FILE_NAME, strlen(UPGRADE_FILE_NAME));

    http_upgrade_init(&info);
}

static void ICACHE_FLASH_ATTR wifi_station_cb(uint8_t status)
{
    if(status == STATION_GOT_IP){
        hy_info("wifi connected\r\n");
        if(WIFI_STA_STATUS == honyar_wifi_get_work_status()) {
            http_upgrade_try();
        }
    } else {
        hy_info("wifi status: %d\r\n", status);
    }
}

static void ICACHE_FLASH_ATTR user_config_regist(void)
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

static void ICACHE_FLASH_ATTR user_wifi_init(void *parm)
{
    uint8_t status = honyar_wifi_get_work_status();
    honyar_wifi_init();
    honyar_wifi_station_regist_statuscb(wifi_station_cb);

    if(WIFI_MESH_STATUS == status) {
        mesh_app_init();
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

void ICACHE_FLASH_ATTR user_init(void)
{
    honyar_platform_init();
    
    user_config_regist();
    dl_config_init();

    uart_app_init();
    
    honyar_add_task(user_wifi_init, NULL, 0);

}

