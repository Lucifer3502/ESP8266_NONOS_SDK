
#include "honyar_common.h"
#include "xo1008_uart.h"
#include "xo1008_device.h"

#define PROTOCOL_HEAD_MAGIC 0xAA

static uint8_t ICACHE_FLASH_ATTR
checksum(uint8_t *buf,   uint32_t len)
{
	uint32_t i;
	uint8_t sum = 0;
	for(i = 0;i < len; i++) {
		sum += buf[i];
	}
	sum = ~sum;
	sum++;
	return sum;
}

static int32_t ICACHE_FLASH_ATTR
xo1008_uart_protocol_parse(uint8_t *frame, uint32_t frame_len)
{
    uint8_t cmd = frame[9];
    uint8_t child_cmd = frame[10];
    switch(cmd) {
    case 0x02:
        if(frame_len < 13) {
            return -1;
        }
        if(0x01 == child_cmd) {
            xo1008_device_set_power(frame[11]);
        }else if(0x02 == child_cmd) {

        } else {
            return -1;
        }
        break;
    
    default:
        //unkown cmd;
        return -1;
        break;
    }

    return 0;
}

int32_t ICACHE_FLASH_ATTR
xo1008_uart_download(uint8_t *data, uint32_t len)
{
    uint8_t *frame = NULL;
    uint32_t frame_len = 0;
    uint32_t offset = 0;
    uint32_t flag = 0;
    while(offset + 11 < len) {
        frame = data + offset;
        frame_len = frame[1] + (frame[2] << 8) + 1;
        if(PROTOCOL_HEAD_MAGIC != data[offset]) {
            offset++;
            continue;
        }
        if((frame_len < 10) || (frame_len > (len - offset))) {
            hy_error("uart len err\n");
            offset++;
            continue;
        }
        if(checksum(data + offset, frame_len - 1)){
            hy_error("check sum err\n");
			offset++;
			continue;
        }
        offset += frame_len;
        flag = 1;
        if(xo1008_uart_protocol_parse(frame, frame_len)) {
            return -1;
        }
    }
    if(!flag) {
        return -1;
    }
    return 0;
}

static void ICACHE_FLASH_ATTR
xo1008_uart_recv_task(void *parm)
{
    uint8_t buf[128];
    uint32_t len = 128;
    uint32_t rbytes = honyar_uart_read(buf, len, 50);
    if(0 == rbytes) {
        return;
    }

    
}

int32_t ICACHE_FLASH_ATTR
xo1008_uart_init(void)
{
    honyar_uart_init(BIT_RATE_9600);
    UART_SetParity(UART0, EVEN_BITS);
    honyar_add_task(xo1008_uart_recv_task, NULL, 0);
}

