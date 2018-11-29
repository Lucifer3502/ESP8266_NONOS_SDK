
#ifndef _HONYAR_MESH_H_
#define _HONYAR_MESH_H_

#include "honyar_types.h"
#include "ip_addr.h"

typedef struct honyar_mesh_info{
    uint8_t *ssid_prefix;
    uint32_t ssid_len;
    uint8_t *pwd;
    uint32_t pwd_len;
    struct ip_addr server;
    uint16_t port;
}honyar_mesh_info_t;

int32_t honyar_mesh_init(honyar_mesh_info_t *info);

#endif
