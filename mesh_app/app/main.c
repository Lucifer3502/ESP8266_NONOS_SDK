#include "honyar_common.h"

static void ICACHE_FLASH_ATTR network_test(void)
{
    http_upgrade_info_t info;
    memset(&info, 0, sizeof(info));
    memcpy(info.host, "192.168.100.169", NET_IP_ADDR_LEN);
    info.port = 8080;

    http_upgrade_init(&info);
}

static void ICACHE_FLASH_ATTR uart_recv_test(void *parm)
{
    uint8_t buf[128];
    uint32_t len = 128;
    uint32_t rbytes = honyar_uart_read(buf, len, 30);
    if(rbytes) {
        hy_info("uart read %d bytes\r\n", rbytes);
        honyar_uart_write(buf, rbytes);
    }
}

static void ICACHE_FLASH_ATTR uart_test(void)
{
    //honyar_uart_init(115200);
    honyar_add_task(uart_recv_test, NULL, 0);
}

static void wifi_station_cb(uint8_t status)
{
    if(status == STATION_GOT_IP){
        hy_info("wifi connected\r\n");
        network_test();
    } else {
        hy_info("wifi status: %d\r\n", status);
    }
}

void ICACHE_FLASH_ATTR user_init(void)
{    
    honyar_platform_init();

    uart_test();

    honyar_wifi_init();
    honyar_wifi_station_regist_statuscb(wifi_station_cb);
}

