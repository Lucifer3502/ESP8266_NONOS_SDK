
#ifndef _HONYAR_WIFI_H_
#define _HONYAR_WIFI_H_

#include "c_types.h"

#define WIFI_SSID_DEF "X1pP@7ua*gavRV$*YYJb9w"
#define WIFI_PWD_DEF "00144dev*"

#define WIFI_SSID_LEN 32
#define WIFI_PASSWD_LEN 64
#define WIFI_SCAN_AP_MAX_NUM 32
#define MAC_ADDR_LEN  6

typedef enum {
    WIFI_STA_STATUS = 0,
    WIFI_MESH_STATUS = 1,
    WIFI_SMARTCONFIG_STATUS = 2,
    WIFI_INVALID_STATUS,
}wifi_work_status_t;

typedef struct wifi_scan_result_info_tag{
	uint8_t ssid[WIFI_SSID_LEN + 4];
	uint8_t ssid_len;
	uint8_t channel;
	uint8_t mac[6];
	uint8_t signal;
	uint8_t sec;
    uint8_t reverse[3];
}wifi_scan_result_info_t;


typedef void (*wifi_station_status_cb_t)(uint8_t status);

int32_t honyar_wifi_get_macaddr(uint8_t *mac);

int32_t honyar_wifi_station_regist_statuscb(wifi_station_status_cb_t cb);

int32_t honyar_wifi_station_start(uint8_t *ssid, uint8_t *passwd);

uint8_t honyar_wifi_get_work_status(void);

int32_t honyar_wifi_set_work_status(uint8_t status);

uint8_t *honyar_wifi_get_router_ssid(void);

uint8_t *honyar_wifi_get_router_passwd(void);

int32_t honyar_wifi_set_router_ssid(uint8_t *ssid);

int32_t honyar_wifi_set_router_passwd(uint8_t *passwd);

int32_t honyar_wifi_init(void);

uint32_t honyar_wifi_scan_isover(void);

int32_t honyar_wifi_scan(void);

int32_t honyar_wifi_get_list( wifi_scan_result_info_t **list, uint32_t *num);

#endif
