
#include "honyar_device.h"
#include "honyar_common.h"

static uint8_t g_device_sn[HONYAR_DEVICE_SN_MAX_SIZE + 4];
static uint8_t g_device_model[HONYAR_DEVICE_MODEL_MAX_SIZE + 4];


void ICACHE_FLASH_ATTR
honyar_device_config_regist(void)
{
    DL_CONFIG_ITEM_S config_items[] = 
	{
        {"CFG_DEVICE_SN", DL_CFG_ITEM_TYPE_STRING, g_device_sn, sizeof(g_device_sn), 0},
        {"CFG_DEVICE_MODEL", DL_CFG_ITEM_TYPE_STRING, g_device_model, sizeof(g_device_model), 0},
	};

	uint32_t i;
	for (i = 0; i < sizeof(config_items)/sizeof(config_items[0]); i++) {
        dl_config_items_register_by_user(&config_items[i]);
    }
}


uint8_t ICACHE_FLASH_ATTR
*honyar_device_get_sn(void)
{
    return g_device_sn;
}

int32_t ICACHE_FLASH_ATTR
honyar_device_set_sn(uint8_t *sn)
{
    os_strncpy(g_device_sn, sn, HONYAR_DEVICE_SN_MAX_SIZE);
    return 0;
}

uint8_t ICACHE_FLASH_ATTR
*honyar_device_get_model(void)
{
    return g_device_model;
}

int32_t ICACHE_FLASH_ATTR
honyar_device_set_model(uint8_t *model)
{
    os_strncpy(g_device_model, model, HONYAR_DEVICE_MODEL_MAX_SIZE);
    return 0;
}


