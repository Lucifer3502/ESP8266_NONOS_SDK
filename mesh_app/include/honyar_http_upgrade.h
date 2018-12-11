
#ifndef _HONYAR_HTTP_UPGRADE_H_
#define _HONYAR_HTTP_UPGRADE_H_

#include "c_types.h"
#include "honyar_network.h"


#define UPGRADE_FILENAME_LEN  128

typedef struct http_upgrade_info_tag{
    uint8_t host[NET_IP_ADDR_LEN];
    uint16_t port;
    uint8_t file[UPGRADE_FILENAME_LEN];
}http_upgrade_info_t;


typedef void (*http_upgrade_failed_cb_t)(void *arg);

void http_upgrade_regist_faild_cb(http_upgrade_failed_cb_t cb);

int32_t http_upgrade_init(http_upgrade_info_t *info);

int32_t http_upgrade_init2(uint8_t *url);

#endif
