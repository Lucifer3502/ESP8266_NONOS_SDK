
#include "honyar_common.h"
#include "xo1008_uart.h"
#include "xo1008_device.h"
#include "dl2106f.h"

#define PROTOCOL_HEAD_MAGIC 0xAA
#define PRO_BUF_SIZE 32

enum {
    TOTAL_ENERGE = 0,
    CUR_POWER = 118,
    ELEC_VOLTAGE = 100,
    ELEC_CURRENT = 106,
}modbus_fun_t;

static os_timer_t g_xo1008_modbus_timer;

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
#ifdef DL2106F
            dl2106f_set_socket_power(frame[11]);
#else 
            hy_error("not support power on or off\r\n");
            return -1;
#endif
        }else if(0x02 == child_cmd) {
            if(dl_irda_send(&frame[12], frame_len - 12 - 1, frame[11])) {
            	hy_error("irda send err\r\n");
                return -1;
            }
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
    hex_printf("download:", data, len);
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
        if(checksum(data + offset + 1, frame_len - 1)){
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

static int32_t ICACHE_FLASH_ATTR
xo1008_upload_elec(uint32_t energy, uint32_t power, uint32_t voltage, uint32_t current)
{
    uint8_t buf[PRO_BUF_SIZE] = {0};
    uint32_t len = 0;
    buf[len++] = PROTOCOL_HEAD_MAGIC;
    len += 8;
    buf[len++] = 0x05;
    buf[len++] = 0x01;
    buf[len++] = xo1008_device_get_power_state();
    buf[len++] = power & 0xff;
    buf[len++] = (power >> 8) & 0xff;
    buf[len++] = (power >> 16) & 0xff;
    buf[len++] = (power >> 24) & 0xff;
    buf[len++] = energy & 0xff;
    buf[len++] = (energy >> 8) & 0xff;
    buf[len++] = (energy >> 16) & 0xff;
    buf[len++] = (energy >> 24) & 0xff;
    buf[len++] = voltage & 0xff;
    buf[len++] = (voltage >> 8) & 0xff;
    buf[len++] = (voltage >> 16) & 0xff;
    buf[len++] = (voltage >> 24) & 0xff;
    buf[len++] = current & 0xff;
    buf[len++] = (current >> 8) & 0xff;
    buf[len++] = (current >> 16) & 0xff;
    buf[len++] = (current >> 24) & 0xff;
    buf[1] = len;
    buf[len++] = checksum(buf + 1, len - 1);
    xo1008_net_upload(buf, len);
}

static int32_t ICACHE_FLASH_ATTR
xo1008_modbus_read_elec_parm(uint32_t *value, uint16_t func)
{
    uint8_t buf[PRO_BUF_SIZE];
    uint32_t len = PRO_BUF_SIZE;
    uint32_t rbytes = 0;
    uint16_t crc = 0;
    float f_val = 0;
    uint32_t i_val = 0;
    //clear buf;
    while(honyar_uart_read(buf, len, 50, 0));
    
    len = 0;
    buf[len++] = 0xff;//broadcast addr
    buf[len++] = 0x03;//cmd
    buf[len++] = (func >> 8) & 0xff;//high
    buf[len++] = func & 0xff;//low
    buf[len++] = 0x00;
    buf[len++] = 0x02;
    crc = modbus_crc16(buf, len);
    buf[len++] = crc & 0xff;
    buf[len++] = (crc >> 8) & 0xff;
    honyar_uart_write(buf, len);

    if(0 == honyar_uart_read(buf, 1, 150, 1)) {
        return -1;
    }
    rbytes = honyar_uart_read(buf + 1, 8, 50, 1);
    if(rbytes != 8) {
        return -1;
    }
    if(0x03 != buf[1] || 0x04 != buf[2]) {
        return -1;
    }
    crc = buf[7] | (buf[8] >> 8);
    if(crc != modbus_crc16(buf, 7)) {
        hy_error("modbus crc check failed.\r\n");
        return -1;
    }
#if 0
    *value = bcd_to_hex(buf[3]) * 1000000 
                + bcd_to_hex(buf[4]) * 10000
                + bcd_to_hex(buf[5]) * 100
                + bcd_to_hex(buf[6]);
#else
    i_val = (buf[3] << 24) + 
            (buf[4] << 16) +
            (buf[5] << 8) +
            buf[6];
    memcpy(&f_val, &i_val, sizeof(uint32_t));
    *value = f_val * 100;
#endif
    return 0;
}

static int32_t ICACHE_FLASH_ATTR
xo1008_modbus_read_elec_energy(uint32_t *value)
{
    return xo1008_modbus_read_elec_parm(value, TOTAL_ENERGE);
}

static int32_t ICACHE_FLASH_ATTR
xo1008_modbus_read_elec_power(uint32_t *value)
{
    return xo1008_modbus_read_elec_parm(value, CUR_POWER);
}

static int32_t ICACHE_FLASH_ATTR
xo1008_modbus_read_elec_voltage(uint32_t *value)
{
    return xo1008_modbus_read_elec_parm(value, ELEC_VOLTAGE);
}

static int32_t ICACHE_FLASH_ATTR
xo1008_modbus_read_elec_current(uint32_t *value)
{
    return xo1008_modbus_read_elec_parm(value, ELEC_CURRENT);
}


static void ICACHE_FLASH_ATTR
xo1008_modbus_task(uint32_t *value)
{
    uint32_t energe = 0;
    uint32_t power = 0;
    uint32_t voltage = 0;
    uint32_t current = 0;
    int32_t err = 0;
    
    if(honyar_global_timer_is_disable()) {
        return;
    }
    
    err += xo1008_modbus_read_elec_energy(&energe);
    err += xo1008_modbus_read_elec_power(&power);
    err += xo1008_modbus_read_elec_voltage(&power);
    err += xo1008_modbus_read_elec_current(&power);

    if(0 == err) {
        hy_info("energy: %u, power: %u, voltage: %u, current: %u\r\n", energe, power, voltage, current);
        xo1008_upload_elec(energe, power, voltage, current);
    } else {
        hy_error("read elec info failed, err = %d\r\n", err);
        xo1008_upload_elec(energe, power, voltage, current);
    }

    os_timer_arm(&g_xo1008_modbus_timer, 60000, false);
}


int32_t ICACHE_FLASH_ATTR
xo1008_uart_init(void)
{
    honyar_uart_init(BIT_RATE_9600);
    UART_SetParity(UART0, EVEN_BITS);

    os_timer_disarm(&g_xo1008_modbus_timer);
    os_timer_setfn(&g_xo1008_modbus_timer, (os_timer_func_t *)xo1008_modbus_task, NULL);
    os_timer_arm(&g_xo1008_modbus_timer, 60000, false);
}

