#ifndef _XO1008_LED_H_
#define _XO1008_LED_H_

#include "honyar_types.h"

#define XO1008_LED1_LOGIC_PIN  21
#define XO1008_LED2_LOGIC_PIN  19

enum {
    LED_OFF = 0,
    LED_ON = 1,
}xo1008_led_state_t;

void xo1008_led_set_work_mode(uint8_t mode);

void xo1008_led_init(void);

#endif

