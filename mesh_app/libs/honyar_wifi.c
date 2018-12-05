#include "honyar_common.h"

#define WIFI_SSID_DEF "zhihe_test"
#define WIFI_PWD_DEF "123456zhihe"


static wifi_station_status_cb_t g_wifi_station_status_cb;

static uint8_t g_wifi_work_status = WIFI_SMARTCONFIG_STATUS;
static uint8_t g_wifi_router_ssid[WIFI_SSID_LEN + 4] = WIFI_SSID_DEF;
static uint8_t g_wifi_router_passwd[WIFI_PASSWD_LEN + 4] = WIFI_PWD_DEF;

static uint8_t g_wifi_scan_over_flag;
static wifi_scan_result_info_t *g_wifi_scan_info = NULL;
static uint32_t g_wifi_ap_cnt = 0;



int32_t ICACHE_FLASH_ATTR
honyar_wifi_get_macaddr(uint8_t *mac)
{
    if (wifi_get_macaddr(STATION_IF, mac)) {
        return 0;
    }

    hy_error("wifi_get_macaddr failed\r\n");
    return -1;
}

int32_t ICACHE_FLASH_ATTR
honyar_wifi_station_regist_statuscb(wifi_station_status_cb_t cb)
{
    g_wifi_station_status_cb = cb;
    return 0;
}

static void ICACHE_FLASH_ATTR
honyar_wifi_sta_workstation(void *parm)
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

int32_t ICACHE_FLASH_ATTR
honyar_wifi_station_start(uint8_t *ssid, uint8_t *passwd)
{
    struct station_config sta_conf;
	os_memset(&sta_conf, 0, sizeof(struct station_config));
	os_sprintf(sta_conf.ssid, "%s", ssid);
	os_sprintf(sta_conf.password, "%s", passwd);

    wifi_set_opmode_current(STATION_MODE);
	wifi_station_set_config_current(&sta_conf);
	wifi_station_connect();
}

uint8_t ICACHE_FLASH_ATTR 
honyar_wifi_get_work_status(void)
{
    return ((g_wifi_work_status >= WIFI_INVALID_STATUS)
            ? WIFI_INVALID_STATUS : g_wifi_work_status);
}

int32_t ICACHE_FLASH_ATTR 
honyar_wifi_set_work_status(uint8_t status)
{
    g_wifi_work_status =  ((status >= WIFI_INVALID_STATUS)
            ? WIFI_MESH_STATUS : status);

    return 0;
}

uint8_t ICACHE_FLASH_ATTR
*honyar_wifi_get_router_ssid(void)
{
    return g_wifi_router_ssid;
}

uint8_t ICACHE_FLASH_ATTR
*honyar_wifi_get_router_passwd(void)
{
    return g_wifi_router_passwd;
}

int32_t ICACHE_FLASH_ATTR
honyar_wifi_set_router_ssid(uint8_t *ssid)
{
    os_strncpy(g_wifi_router_ssid, ssid, WIFI_SSID_LEN);
    return 0;
}

int32_t ICACHE_FLASH_ATTR
honyar_wifi_set_router_passwd(uint8_t *passwd)
{
    os_strncpy(g_wifi_router_passwd, passwd, WIFI_PASSWD_LEN);
    return 0;
}


void ICACHE_FLASH_ATTR
honyar_wifi_config_regist(void)
{
    DL_CONFIG_ITEM_S config_items[] = 
	{
        {"CFG_WIFI_WORK_STATUS", DL_CFG_ITEM_TYPE_DEC8, &g_wifi_work_status, sizeof(g_wifi_work_status), 0},
        {"CFG_WIFI_ROUTER_SSID", DL_CFG_ITEM_TYPE_STRING, g_wifi_router_ssid, sizeof(g_wifi_router_ssid), 0},
        {"CFG_WIFI_ROUTER_PASSWD", DL_CFG_ITEM_TYPE_STRING, g_wifi_router_passwd, sizeof(g_wifi_router_passwd), 0},
	};

	uint32_t i;
	for (i = 0; i < sizeof(config_items)/sizeof(config_items[0]); i++) {
        dl_config_items_register_by_user(&config_items[i]);
    }
}

int32_t ICACHE_FLASH_ATTR
honyar_wifi_init(void)
{
    //honyar_wifi_station_start(WIFI_SSID_DEF, WIFI_PWD_DEF);
    honyar_add_task(honyar_wifi_sta_workstation, NULL, 1000 / TASK_CYCLE_TM_MS);
}

uint32_t ICACHE_FLASH_ATTR
honyar_wifi_scan_isover(void)
{
    return g_wifi_scan_over_flag;
}

static void ICACHE_FLASH_ATTR 
honyar_wifi_station_scan_cb(void *arg, STATUS status)
{
    uint8_t ssid[WIFI_SSID_LEN + 4] = {0};
    uint8_t ap_num = 0;
    int8_t rssi_high = -40;
    int8_t rssi_low = -50;
    int8_t rssi_min = -100;
    
    if(OK != status) {
        hy_error("station scan done failed. status: %d\r\n", status);
        goto end;
    }

    struct bss_info *bss_link = (struct bss_info *)arg;
    if(!g_wifi_scan_info) {
        goto end;
    }
    
    while(bss_link) {
        if(bss_link->rssi >= rssi_high) {
            memcpy(g_wifi_scan_info[ap_num].ssid, bss_link->ssid, WIFI_SSID_LEN);
            g_wifi_scan_info[ap_num].ssid_len = strlen(g_wifi_scan_info[ap_num].ssid);
            g_wifi_scan_info[ap_num].channel = bss_link->channel;
            memcpy(g_wifi_scan_info[ap_num].mac, bss_link->bssid, 6);
            g_wifi_scan_info[ap_num].signal = bss_link->rssi;
            g_wifi_scan_info[ap_num].sec = bss_link->authmode;
            ap_num++;
        }
        
        bss_link = bss_link->next.stqe_next;
        if(ap_num >= WIFI_SCAN_AP_MAX_NUM) {
            break;
        }
    }

    bss_link = (struct bss_info *)arg;
    while(bss_link) {
        if(ap_num >= WIFI_SCAN_AP_MAX_NUM) {
            break;
        }
        if(bss_link->rssi < rssi_high && bss_link->rssi >= rssi_low) {
            memcpy(g_wifi_scan_info[ap_num].ssid, bss_link->ssid, WIFI_SSID_LEN);
            g_wifi_scan_info[ap_num].ssid_len = strlen(g_wifi_scan_info[ap_num].ssid);
            g_wifi_scan_info[ap_num].channel = bss_link->channel;
            memcpy(g_wifi_scan_info[ap_num].mac, bss_link->bssid, 6);
            g_wifi_scan_info[ap_num].signal = bss_link->rssi;
            g_wifi_scan_info[ap_num].sec = bss_link->authmode;
            ap_num++;
        }
        bss_link = bss_link->next.stqe_next;
        if(NULL == bss_link) {
            bss_link = (struct bss_info *)arg;
            rssi_high -= 10;
            rssi_low -= 10;
            if(rssi_high < rssi_min) {
                //ignore, the signal is too weak
                break;
            }
        }
    }
    g_wifi_ap_cnt = ap_num;

    //debug print
    bss_link = (struct bss_info *)arg;
    while(bss_link) {
        memcpy(ssid, bss_link->ssid, WIFI_SSID_LEN);
        hy_printf("(sec:%d, ssid:\"%s\", rssi:%d, mac:\""MACSTR"\", channel:%d, freq_offset:%d, freqcal_val:%d)\r\n",
                 bss_link->authmode, ssid, bss_link->rssi,
                 MAC2STR(bss_link->bssid),bss_link->channel, 
                 bss_link->freq_offset, bss_link->freqcal_val);
        
        bss_link = bss_link->next.stqe_next;
    }

end:
    hy_info("station scan over.\r\n");
    g_wifi_scan_over_flag = 1;
}


int32_t ICACHE_FLASH_ATTR
honyar_wifi_scan(void)
{
    uint32_t sleep_time = 0;
    
    if (g_wifi_scan_info == NULL) {
        g_wifi_scan_info = honyar_malloc(sizeof(wifi_scan_result_info_t) * WIFI_SCAN_AP_MAX_NUM);
        memset(g_wifi_scan_info, 0, sizeof(wifi_scan_result_info_t) * WIFI_SCAN_AP_MAX_NUM);
    }
    g_wifi_ap_cnt = 0;
    g_wifi_scan_over_flag = 0;
    
    wifi_set_opmode_current(STATION_MODE);
    if(wifi_get_opmode() == SOFTAP_MODE) {
        hy_error("ap mode can't scan !!!\r\n");
        return -1;
    }

#if 1
    if(!wifi_station_scan(NULL, honyar_wifi_station_scan_cb)) {
        hy_error("station scan failed.\r\n");
        return -1;
    }
#else
    struct scan_config sconf;
    memset(&sconf, 0, sizeof(struct scan_config));
    sconf.show_hidden = 1;
    sconf.scan_type = 0;
    sconf.scan_time.active.min = 100;
    sconf.scan_time.active.max = 300;
    if(!wifi_station_scan(&sconf, honyar_wifi_station_scan_cb)) {
        hy_error("station scan failed.\r\n");
        return -1;
    }
#endif
    
    return 0;
}


int32_t ICACHE_FLASH_ATTR
honyar_wifi_get_list( wifi_scan_result_info_t **list, uint32_t *num)
{
    if(g_wifi_scan_info) {
        *num = g_wifi_ap_cnt;
        *list = g_wifi_scan_info;
        return 0;
    }
    return -1;
}

