
#ifndef _HONYAR_WIFI_H_
#define _HONYAR_WIFI_H_

#include "c_types.h"

#define WIFI_SSID_LEN 32
#define wifi_PASSWD_LEN 64

typedef enum {
    WIFI_STA_STATUS = 0,
    WIFI_MESH_STATUS = 1,
    WIFI_SMARTCONFIG_STATUS = 2,
    WIFI_INVALID_STATUS,
}wifi_work_status_t;

typedef void (*wifi_station_status_cb_t)(uint8_t status);

int32_t honyar_wifi_get_macaddr(uint8_t *mac);

int32_t honyar_wifi_station_regist_statuscb(wifi_station_status_cb_t cb);

int32_t honyar_wifi_station_start(uint8_t *ssid, uint8_t *passwd);

uint8_t honyar_wifi_work_status(void);

int32_t honyar_wifi_init(void);
#endif
