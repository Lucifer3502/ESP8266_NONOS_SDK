
#ifndef _HONYAR_GPIO_H_
#define _HONYAR_GPIO_H_

#include "c_types.h"

#define LOGICAL_PIN_MAX_NUM 26
#define PIN_IO_NONE		(255)

/** 
 * GPIO�ܽŷ�������ֵ
 */
typedef enum
{
	/** ����ܽ� */
    GPIO_INTPUT,
	
	/** ����ܽ� */
    GPIO_OUTPUT
}gpio_dir_t;

/** 
 * IO��Ӳ����������   
 */
typedef enum
{
	/** �������� */
    PULL_NONE,
	
	/** ���� */
    PULL_UP,
	
	/** ���� */
    PULL_DOWN,
	
	/** ���� */
    PULL_PUSH
} gpio_pull_t;

/** 
 * IO�������ƽ״̬  
 */
typedef enum
{
	/** �͵�ƽ */
    GPIO_LOW = 0,
	
	/** �ߵ�ƽ */
    GPIO_HIGH = 1,
	
	/** δ֪ */
    GPIO_UNKNOWN = 255,
}gpio_state_t;//�ߵ͵�ƽ 

uint8_t honyar_gpio_find(uint8_t pin_num);

/** 
 * GPIO�ڳ�ʼ��
 * ����GPIO�ڵķ���͹ܽ����á�
 * @param[in]   pin �߼��ܽźš�
 * @param[in]   dir ������ϸ�ο�DL_GPIO_DIR����
 * @param[in]   pull �ܽ����á���ϸ�ο�DL_GPIO_PULL����
 * @retval  0   �ɹ�
 * @retval  <0  ʧ��
 */
int32_t honyar_gpio_config(uint8_t pin, uint8_t dir, uint8_t pull);

/** 
 * ����IO�������ƽ
 * ����IO������߻��ߵ͵�ƽ
 * @param[in]   pin �߼��ܽźš�
 * @param[in]   state �ܽŵ�ƽ״̬����ϸ�ο�DL_GPIO_STATE����
 * @retval  0   �ɹ�
 * @retval  <0  ʧ��
 */
int32_t honyar_gpio_set_output(uint8_t pin, uint8_t state);

/** 
 * ��ȡ��ǰIO�������ƽ
 * ��ȡ��ǰIO�������ƽ
 * @param[in]   pin �߼��ܽźš�
 * @retval  state �ܽŵ�ƽ״̬����ϸ�ο�DL_GPIO_STATE����
 */
uint8_t honyar_gpio_get_input(uint8_t pin);

/**********************************************************/
/** 
 * GPIO��ʼ��������
 */
 void honyar_gpio_init(void);


#endif

