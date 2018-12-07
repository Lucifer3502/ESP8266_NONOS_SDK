
#include "honyar_common.h"
#include "xo1008_uart.h"

int32_t ICACHE_FLASH_ATTR
xo1008_uart_download(uint8_t *data, uint32_t len)
{
    return 0;
}

static void ICACHE_FLASH_ATTR
xo1008_uart_recv_task(void *parm)
{
    uint8_t buf[128];
    uint32_t len = 128;
    uint32_t rbytes = honyar_uart_read(buf, len, 50);
    if(rbytes) {
        hy_info("uart read %d bytes\r\n", rbytes);
        honyar_uart_write(buf, rbytes);
    }
}

int32_t ICACHE_FLASH_ATTR
xo1008_uart_init(void)
{
    honyar_uart_init(BIT_RATE_9600);
    UART_SetParity(UART0, EVEN_BITS);
    honyar_add_task(xo1008_uart_recv_task, NULL, 0);
}

