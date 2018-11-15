
#ifndef _HONYAR_HTTP_UPGRADE_H_
#define _HONYAR_HTTP_UPGRADE_H_

#include "c_types.h"
#include "honyar_network.h"

#define UPGRADE_PATHNAME_LEN  64
#define UPGRADE_FILENAME_LEN  64

typedef struct http_upgrade_info_tag{
    uint8_t host[NET_IP_ADDR_LEN];
    uint16_t port;
    uint8_t path[UPGRADE_PATHNAME_LEN];
    uint8_t file[UPGRADE_FILENAME_LEN];
}http_upgrade_info_t;

int32_t http_upgrade_init(http_upgrade_info_t *info);

#endif
