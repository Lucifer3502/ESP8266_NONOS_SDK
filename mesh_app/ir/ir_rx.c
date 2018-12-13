

#include "ir_rx.h"
#include "infared/honyar_ir.h"
#include "honyar_common.h"


static uint32_t g_last_time_us;
static uint32_t g_wave_num = 0;
static os_timer_t g_ir_rx_timer;
static uint8_t g_time_unit_us;
static uint8_t *g_ir_rx_buf = NULL;
static uint32_t g_ir_rx_buf_len = 0;
static uint32_t g_ir_rx_disable;

static void ICACHE_FLASH_ATTR
dl_ir_rx_disable(void)
{
    uint8_t pin_num = honyar_gpio_find(DELAN_IR_RX_PIN_NUM);
    if(PIN_IO_NONE == pin_num)
    {
        return;
    }
    gpio_pin_intr_state_set(pin_num, GPIO_PIN_INTR_DISABLE);
    g_ir_rx_disable = 1;
}

static void ICACHE_FLASH_ATTR
dl_ir_rx_enable()
{
    uint8_t pin_num = honyar_gpio_find(DELAN_IR_RX_PIN_NUM);
    if(PIN_IO_NONE == pin_num)
    {
        return;
    }
    gpio_pin_intr_state_set(pin_num, GPIO_PIN_INTR_ANYEDGE);
    g_ir_rx_disable = 0;
}


static void ICACHE_FLASH_ATTR
dl_ir_rx_timer_cb(void *parm)
{
#if 0
    uint32_t cur_time_us = system_get_time();
    if(cur_time_us - g_last_time_us < 100)
    {
        os_timer_arm(&g_ir_rx_timer, 100, FALSE);
    }
    else
    {
        os_timer_disarm(&g_ir_rx_timer);
        dl_ir_rx_disable();
    }
#else
    os_timer_disarm(&g_ir_rx_timer);
    dl_ir_rx_disable();
#endif
}

static void ICACHE_FLASH_ATTR
dl_ir_rx_handler(void)
{
    uint8_t pin_num = honyar_gpio_find(DELAN_IR_RX_PIN_NUM);
    if(PIN_IO_NONE == pin_num)
    {
        return;
    }

    uint32_t cur_time_us = system_get_time();
    uint8_t cur_value = honyar_gpio_get_input(DELAN_IR_RX_PIN_NUM);
    if(0 == g_wave_num)
    {
        if(GPIO_LOW != cur_value)
        {
            //强制结束
            dl_ir_rx_disable();
            return;
        }
        //采样时间100ms
        os_timer_arm(&g_ir_rx_timer, 200, FALSE);
        g_last_time_us = cur_time_us;
        g_wave_num++;
        return;
    }

    uint32_t pos = g_wave_num * 2;
    pos -= 2;
    uint32_t int_time = (cur_time_us - g_last_time_us) / g_time_unit_us;
    g_last_time_us = cur_time_us;
    g_ir_rx_buf[pos] = int_time & 0xff;
    g_ir_rx_buf[pos + 1] = (int_time >> 8) & 0xff;
    g_wave_num++;

    if(g_wave_num * 2 > g_ir_rx_buf_len)
    {
        //buf full, force end;
        dl_ir_rx_disable();
    }
}

static void ICACHE_FLASH_ATTR
dl_ir_rx_intr_event(uint32 intr_mask, void *arg)
{
    uint8_t pin_num = honyar_gpio_find(DELAN_IR_RX_PIN_NUM);
    uint32_t gpio_status;

    if(PIN_IO_NONE == pin_num)
    {
        return;
    }
    
    gpio_status = GPIO_REG_READ(GPIO_STATUS_ADDRESS);
    //clear interrupt status 
    GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, gpio_status);
    if( (gpio_status >> pin_num) & BIT0 )
    {
        //dl_printf("ir rx intr.\r\n");
        dl_ir_rx_handler();
    }  
    //add else if for other gpio intr task
    else
    {
        hy_error("gpio num mismached.\r\n");
    }
}

void ICACHE_FLASH_ATTR
dl_ir_rx_init()
{
    uint8_t pin_num = honyar_gpio_find(DELAN_IR_RX_PIN_NUM);
    if(PIN_IO_NONE == pin_num)
    {
        return;
    }
    
    honyar_gpio_config(DELAN_IR_RX_PIN_NUM, GPIO_INTPUT, PULL_NONE);
    GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, BIT(pin_num));
    gpio_pin_intr_state_set(pin_num, GPIO_PIN_INTR_DISABLE);
    gpio_intr_handler_register(dl_ir_rx_intr_event, NULL);
    ETS_GPIO_INTR_ENABLE();

    os_timer_disarm(&g_ir_rx_timer);
    os_timer_setfn(&g_ir_rx_timer, dl_ir_rx_timer_cb, NULL);
}

//返回波形电平数量
uint16_t ICACHE_FLASH_ATTR
dl_ir_rx_start(uint8_t *buf, uint32_t *plen, uint8_t time_unit, uint32_t time_out)
{
    if(NULL == buf | 2 > *plen | 0 == time_unit)
    {
        return;
    }
    g_ir_rx_buf = buf;
    g_ir_rx_buf_len = *plen;
    g_wave_num = 0;
    g_last_time_us = 0;
    g_time_unit_us = time_unit;
    dl_ir_rx_enable();

    *plen = 0;
    
    uint32_t start_time;
    uint32_t cur_time = system_get_time();
    start_time = cur_time;
    while(!g_ir_rx_disable)
    {
        honyar_msleep(10);
        if(cur_time - start_time > time_out * 1000)
        {
            if(0 == g_wave_num)
            {
                dl_ir_rx_disable();
                break;
            }
        }
        cur_time = system_get_time();
    }
    if(g_wave_num)
    {
        g_wave_num--;
    }
    *plen = g_wave_num * 2;
    return g_wave_num;
}

