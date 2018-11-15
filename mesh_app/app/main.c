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
    uint32_t rbytes = honyar_uart_read(buf, len, 50);
    if(rbytes) {
        hy_info("uart read %d bytes\r\n", rbytes);
        honyar_uart_write("hello world\r\n", strlen("hello world\r\n"));
    }
    
}

static void ICACHE_FLASH_ATTR uart_test(void)
{
    honyar_uart_init(9600);
    honyar_add_task(uart_recv_test, NULL, 0);
}

void ICACHE_FLASH_ATTR user_init(void)
{    
    honyar_platform_init();

    network_test();
}

