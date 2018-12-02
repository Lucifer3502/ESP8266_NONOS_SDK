#include "honyar_common.h"

#define WIFI_SSID_DEF "zhihe_test"
#define WIFI_PWD_DEF "123456zhihe"

static wifi_station_status_cb_t g_wifi_station_status_cb;

int32_t honyar_wifi_get_macaddr(uint8_t *mac)
{
    if (wifi_get_macaddr(STATION_IF, mac)) {
        return 0;
    }

    hy_error("wifi_get_macaddr failed\r\n");
    return -1;
}

int32_t ICACHE_FLASH_ATTR honyar_wifi_station_regist_statuscb(wifi_station_status_cb_t cb)
{
    g_wifi_station_status_cb = cb;
    return 0;
}

static void ICACHE_FLASH_ATTR honyar_wifi_sta_workstation(void *parm)
{
    struct ip_info ip;
    uint8_t status;
    static uint8_t last_status = STATION_IDLE;
    
    wifi_get_ip_info(STATION_IF, &ip);
	status = wifi_station_get_connect_status();
    if(status == last_status) {
        return;
    }

    last_status = status;
    if(g_wifi_station_status_cb) {
        g_wifi_station_status_cb(status);
    }
}

static int32_t ICACHE_FLASH_ATTR honyar_wifi_station_start(uint8_t *ssid, uint8_t *passwd)
{
    struct station_config sta_conf;
	os_memset(&sta_conf, 0, sizeof(struct station_config));
	os_sprintf(sta_conf.ssid, "%s", ssid);
	os_sprintf(sta_conf.password, "%s", passwd);

    wifi_set_opmode_current(STATION_MODE);
	wifi_station_set_config_current(&sta_conf);
	wifi_station_connect();

    honyar_add_task(honyar_wifi_sta_workstation, NULL, 1000 / TASK_CYCLE_TM_MS);
}

int32_t ICACHE_FLASH_ATTR honyar_wifi_init(void)
{
    honyar_wifi_station_start(WIFI_SSID_DEF, WIFI_PWD_DEF);
}

