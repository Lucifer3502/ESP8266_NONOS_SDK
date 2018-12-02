
#ifndef _HONYAR_WIFI_H_
#define _HONYAR_WIFI_H_

#include "c_types.h"

typedef void (*wifi_station_status_cb_t)(uint8_t status);

int32_t honyar_wifi_get_macaddr(uint8_t *mac);

int32_t honyar_wifi_station_regist_statuscb(wifi_station_status_cb_t cb);

int32_t honyar_wifi_station_start(uint8_t *ssid, uint8_t *passwd);

int32_t honyar_wifi_init(void);
#endif
