
#ifndef _DELAN_IR_RX_TX_H_
#define _DELAN_IR_RX_TX_H_

#include "c_types.h"

#define DELAN_IR_TX_PIN_NUM 6
#define DELAN_IR_RX_PIN_NUM 23

//#define IR_DATA_MAX_LEN 1024

typedef enum{
    IR_UNKNOWN_MODE = 0,
    IR_DATA_MODE = 1,
    IR_WAVE_MODE = 2
}IR_DATA_MODE_E;


void dl_ir_tx_init();

int32_t dl_irda_send(uint8_t *data, uint32_t data_len, IR_DATA_MODE_E ir_mode);

void dl_ir_rx_init();

//返回波形电平数量
uint16_t dl_ir_rx_start(uint8_t *buf, uint32_t *plen, uint8_t time_unit, uint32_t time_out);

#endif
