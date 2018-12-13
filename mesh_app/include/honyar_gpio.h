
#ifndef _HONYAR_GPIO_H_
#define _HONYAR_GPIO_H_

#include "c_types.h"

#define LOGICAL_PIN_MAX_NUM 26
#define PIN_IO_NONE		(255)

/** 
 * GPIO管脚方向属性值
 */
typedef enum
{
	/** 输入管脚 */
    GPIO_INTPUT,
	
	/** 输出管脚 */
    GPIO_OUTPUT
}gpio_dir_t;

/** 
 * IO口硬件配置特性   
 */
typedef enum
{
	/** 无上下拉 */
    PULL_NONE,
	
	/** 上拉 */
    PULL_UP,
	
	/** 下拉 */
    PULL_DOWN,
	
	/** 推挽 */
    PULL_PUSH
} gpio_pull_t;

/** 
 * IO口输入电平状态  
 */
typedef enum
{
	/** 低电平 */
    GPIO_LOW = 0,
	
	/** 高电平 */
    GPIO_HIGH = 1,
	
	/** 未知 */
    GPIO_UNKNOWN = 255,
}gpio_state_t;//高低电平 

uint8_t honyar_gpio_find(uint8_t pin_num);

/** 
 * GPIO口初始化
 * 配置GPIO口的方向和管脚设置。
 * @param[in]   pin 逻辑管脚号。
 * @param[in]   dir 方向。详细参考DL_GPIO_DIR定义
 * @param[in]   pull 管脚设置。详细参考DL_GPIO_PULL定义
 * @retval  0   成功
 * @retval  <0  失败
 */
int32_t honyar_gpio_config(uint8_t pin, uint8_t dir, uint8_t pull);

/** 
 * 设置IO口输出电平
 * 设置IO口输出高或者低电平
 * @param[in]   pin 逻辑管脚号。
 * @param[in]   state 管脚电平状态。详细参考DL_GPIO_STATE定义
 * @retval  0   成功
 * @retval  <0  失败
 */
int32_t honyar_gpio_set_output(uint8_t pin, uint8_t state);

/** 
 * 获取当前IO口输入电平
 * 获取当前IO口输入电平
 * @param[in]   pin 逻辑管脚号。
 * @retval  state 管脚电平状态。详细参考DL_GPIO_STATE定义
 */
uint8_t honyar_gpio_get_input(uint8_t pin);

/**********************************************************/
/** 
 * GPIO初始化工作。
 */
 void honyar_gpio_init(void);


#endif

