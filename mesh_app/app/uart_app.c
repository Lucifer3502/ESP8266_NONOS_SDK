
#include "honyar_common.h"
#include "uart_app.h"

static void ICACHE_FLASH_ATTR uart_recv_task(void *parm)
{
    uint8_t buf[128];
    uint32_t len = 128;
    uint32_t rbytes = honyar_uart_read(buf, len, 50);
    if(rbytes) {
        hy_info("uart read %d bytes\r\n", rbytes);
        honyar_uart_write(buf, rbytes);
    }
}

int32_t ICACHE_FLASH_ATTR uart_app_init(void)
{
    honyar_uart_init(BIT_RATE_9600);
    honyar_add_task(uart_recv_task, NULL, 0);
}

