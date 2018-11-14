#include "honyar_common.h"


void ICACHE_FLASH_ATTR user_init(void)
{
    uint8_t buf[125];
    memset(buf, 0, 125);
    honyar_platform_init();
}

