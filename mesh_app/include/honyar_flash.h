
#ifndef _HONYAR_FLASH_H_
#define _HONYAR_FLASH_H_

#include "c_types.h"

void honyar_flash_erase(uint32_t addr,uint32_t len);

void honyar_flash_write(uint32_t addr, uint8_t *buf,uint32_t len);

void honyar_flash_read(uint32_t addr, uint8_t *buf,uint32_t len);

#endif

