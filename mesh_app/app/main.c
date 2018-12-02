#include "honyar_common.h"
#include "mesh_app.h"
#include "uart_app.h"


#define UPGRADE_FILE_NAME "XO1007_OTA_FROM_V1.0.0_TO_V1.0.0.bin"

static void ICACHE_FLASH_ATTR http_upgrade_try(void)
{
    static uint8_t flag = 0;
    http_upgrade_info_t info;
    
    if(flag) {
        return;
    }
    
    memset(&info, 0, sizeof(info));
    memcpy(info.host, "192.168.0.3", NET_IP_ADDR_LEN);
    info.port = 80;
    memcpy(info.file, UPGRADE_FILE_NAME, strlen(UPGRADE_FILE_NAME));

    http_upgrade_init(&info);
}

static void wifi_station_cb(uint8_t status)
{
    if(status == STATION_GOT_IP){
        hy_info("wifi connected\r\n");
        http_upgrade_try();
    } else {
        hy_info("wifi status: %d\r\n", status);
    }
}

void ICACHE_FLASH_ATTR user_init(void)
{    
    honyar_platform_init();

    uart_app_init();

    honyar_wifi_init();
    honyar_wifi_station_regist_statuscb(wifi_station_cb);

    mesh_app_init();
}

