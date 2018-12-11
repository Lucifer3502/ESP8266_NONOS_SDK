
#include "xo1008_upgrade.h"
#include "honyar_common.h"

#define UPGRADE_URL_MAX_LEN CONFIG_VALUE_MAX_LEN


static uint8_t g_upgrade_http_url[UPGRADE_URL_MAX_LEN];


int32_t ICACHE_FLASH_ATTR
xo1008_upgrade_set_url(uint8_t *url)
{
    if(NULL == url) {
        return -1;
    }
    memset(g_upgrade_http_url, 0, UPGRADE_URL_MAX_LEN);
    os_strncpy(g_upgrade_http_url, url, UPGRADE_URL_MAX_LEN - 1);
    return 0;
}

void ICACHE_FLASH_ATTR
xo1008_upgrade_config_init(void)
{
     DL_CONFIG_ITEM_S config_items[] = 
	{
        {"USER_UPGRADE_HTTP_URL", DL_CFG_ITEM_TYPE_STRING, g_upgrade_http_url, sizeof(g_upgrade_http_url), 0},
	};

	uint32_t i;
	for (i = 0; i < sizeof(config_items)/sizeof(config_items[0]); i++) {
        dl_config_items_register_by_user(&config_items[i]);
    }
}

static void ICACHE_FLASH_ATTR
xo1008_upgrade_failed(void *arg)
{
    honyar_sys_reboot(0);
}

static void ICACHE_FLASH_ATTR
xo1008_upgrade_task(void *arg)
{
    if(STATION_GOT_IP == wifi_station_get_connect_status()) {
        honyar_del_task(xo1008_upgrade_task);
        http_upgrade_init2(g_upgrade_http_url);
    }
}

void ICACHE_FLASH_ATTR
xo1008_upgrade_init(void)
{
    honyar_add_task(xo1008_upgrade_task, NULL, 1000 / TASK_CYCLE_TM_MS);
    http_upgrade_regist_faild_cb(xo1008_upgrade_failed);
}

