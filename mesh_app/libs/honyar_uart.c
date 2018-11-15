
#include "honyar_common.h"

#define UART_RECV_BUF_SIZE  1024

typedef struct uart_read_buff_tag{
    uint32_t front;
    uint32_t tail;
    uint8_t buf[UART_RECV_BUF_SIZE];
}uart_read_buff_t;


static uart_read_buff_t g_uart0_buf;

void _uart0_one_byte_enqueue(uint8_t ch)
{
    uint32_t front = g_uart0_buf.front;
    uint32_t tail = g_uart0_buf.tail;
    
    if(front == ((tail + 1) & (UART_RECV_BUF_SIZE - 1))) {
        //full;
        os_printf("buff full\r\n");
        return;
    }
    g_uart0_buf.buf[tail] = ch;
    g_uart0_buf.tail = (tail + 1) & (UART_RECV_BUF_SIZE - 1);
}


uint32_t honyar_uart_read(uint8_t *buf, uint32_t len, uint32_t tm_out)
{
    uint32_t rbytes = 0;
    uint32_t count = 0;
    
    if(NULL == buf || 0 == len)
        return len;
    
    uart_read_buff_t *rx_hd = &g_uart0_buf;
    while(1) {
        if(rx_hd->front == rx_hd->tail) {
            if(0 == rbytes || count >= tm_out) {
                break;
            }
            count++;
            honyar_msleep(1);
        } else {
            buf[rbytes++] = rx_hd->buf[rx_hd->front++];
            rx_hd->front &= (UART_RECV_BUF_SIZE - 1);
            if(rbytes >= len) {
                break;//full
            }
            count = 0;
        }
    }

    return rbytes;
}

uint32_t honyar_uart_write(const uint8_t *buf, uint32_t len)
{  
    uint32_t i;
    for(i = 0; i < len; i++) {
        uart_tx_one_char(UART0, buf[i]);
    }

    return len;
}

int32_t honyar_uart_init(uint32_t baudrate)
{
    UART_SetBaudrate(UART0, baudrate);
    return 0;
}
