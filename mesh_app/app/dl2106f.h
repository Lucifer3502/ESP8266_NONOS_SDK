#ifndef _DL2106F_H_
#define _DL2106F_H_

#ifdef DL2106F

#include "honyar_types.h"

#define DL_ELEC_CAL_PIN_NUM       5
#define DL_SOCKET_POWER_PIN_NUM   4
#define DL_VOLTAGE_PIN_NUM       21

enum {
    POWER_OFF = 0,
    POWER_ON = 1,
}dl2106f_power_state_t;

uint8_t dl2106f_get_socket_power_state(void);

void dl2106f_set_socket_power(uint8_t state);

uint32_t dl2106f_get_electricity_energe(void);

uint32_t dl2106f_get_electricity_cur_power(void);

uint32_t dl2106f_get_socket_cur_voltage(void);

void dl2106f_config_init(void);

void dl2106f_init(void);



#endif //end of DL2106F
#endif // end of _DL2106F_H_
