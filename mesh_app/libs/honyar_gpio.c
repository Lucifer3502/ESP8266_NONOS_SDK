
#include "honyar_common.h"


typedef enum{
    PIN_GPIO_0 = 0,
    PIN_GPIO_1,
    PIN_GPIO_2,
    PIN_GPIO_3,
    PIN_GPIO_4,
    PIN_GPIO_5,
    PIN_GPIO_6,
    PIN_GPIO_7,
    PIN_GPIO_8,
    PIN_GPIO_9,
    PIN_GPIO_10,
    PIN_GPIO_11,
    PIN_GPIO_12,
    PIN_GPIO_13,
    PIN_GPIO_14,
    PIN_GPIO_15,
    PIN_GPIO_16
}pin_gpio_num_t;


static uint8_t g_pin_gpio[LOGICAL_PIN_MAX_NUM + 1];



void ICACHE_FLASH_ATTR
honyar_gpio_init(void)
{
    memset(g_pin_gpio, PIN_IO_NONE, LOGICAL_PIN_MAX_NUM + 1);
    g_pin_gpio[4] = PIN_GPIO_4;
    g_pin_gpio[5] = PIN_GPIO_12;
    g_pin_gpio[6] = PIN_GPIO_14;
    g_pin_gpio[9] = PIN_GPIO_1;
    g_pin_gpio[10] = PIN_GPIO_3;
    g_pin_gpio[13] = PIN_GPIO_3;
    g_pin_gpio[14] = PIN_GPIO_1;
    g_pin_gpio[16] = PIN_GPIO_16;
    g_pin_gpio[18] = PIN_GPIO_2;
    g_pin_gpio[19] = PIN_GPIO_0;
    g_pin_gpio[20] = PIN_GPIO_15;
    g_pin_gpio[21] = PIN_GPIO_13;
    g_pin_gpio[23] = PIN_GPIO_5;
}

static uint8_t ICACHE_FLASH_ATTR
honyar_gpio_find(uint8_t pin_num)
{
    if(pin_num >= 1 && pin_num <=  LOGICAL_PIN_MAX_NUM) {
        return g_pin_gpio[pin_num];
    } else {
        return PIN_IO_NONE;
    }
}

/**********************************************************/
//函数名:dl_gpio_config
//用途：配置GPIO
//参数：1.pin  引脚的编号 2. dir 输入/输出 3. pull上拉/下拉 
//
//返回值：0:成功 其他：失败 
//作者：
/**********************************************************/
int32_t ICACHE_FLASH_ATTR
honyar_gpio_config(uint8_t pin, uint8_t dir, uint8_t pull)
{
    uint8_t key = honyar_gpio_find(pin);

    if (PIN_IO_NONE == key) {
        return -1;
    }
    
    if(PIN_GPIO_16 != key) {
        uint32_t io_reg = GPIO_PIN_REG(key);
        if ((0x1 << key) & (GPIO_Pin_0 | GPIO_Pin_2 | GPIO_Pin_4 | GPIO_Pin_5)) {
             PIN_FUNC_SELECT(io_reg, 0);
        } else {
             PIN_FUNC_SELECT(io_reg, 3);
        }
    }
    
    if(GPIO_INTPUT == dir) {
        if(PIN_GPIO_16 == key) {
            gpio16_input_conf();
        } else {
            uint32_t io_reg = GPIO_PIN_REG(key);
            GPIO_DIS_OUTPUT(key);
            if(PULL_UP == pull) {
                PIN_PULLUP_EN(io_reg);
            } else {
                PIN_PULLUP_DIS(io_reg);
            }
        }
    } else if(GPIO_OUTPUT == dir) {
        if(PIN_GPIO_16 == key) {
            gpio16_output_conf();
        } else {
            uint32_t io_reg = GPIO_PIN_REG(key);
            GPIO_AS_OUTPUT(1 << key);
            if(PULL_UP == pull) {
                PIN_PULLUP_EN(io_reg);
            } else {
                PIN_PULLUP_DIS(io_reg);
            }
        }
    } else {
        return -1;
    }

    return 0;
}

/**********************************************************/
//函数名:dl_gpio_output
//用途：设置GPIO的输出状态 
//参数：1.pin  引脚的编号2.state  输出电平高低 
//
//返回值：0:成功 其他：失败 
//作者：
/**********************************************************/
int32_t ICACHE_FLASH_ATTR
honyar_gpio_set_output(uint8_t pin, uint8_t state)
{
    uint8_t key = honyar_gpio_find(pin);

    if (PIN_IO_NONE == key || GPIO_UNKNOWN == state) {   
        return -1;
    }
    
    if (PIN_GPIO_16 == key){
        gpio16_output_set(state);
    } else {
        GPIO_OUTPUT_SET(key, state);
    }
    return 0;
}

/**********************************************************/
//函数名:dl_gpio_get_input
//用途：设置GPIO的输出状态 
//参数：1.pin  引脚的编号
//
//返回值：DL_GPIO_STATE  LOW/HIGH 
//作者：
/**********************************************************/
uint8_t ICACHE_FLASH_ATTR
honyar_gpio_get_input(uint8_t pin)
{
    uint8_t key = honyar_gpio_find(pin);

    if (PIN_IO_NONE == key) {
        return GPIO_UNKNOWN;
    }

    if(PIN_GPIO_16 == key) {
        return gpio16_input_get();
    }
    
    return GPIO_INPUT_GET(key);
}


