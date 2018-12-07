
#ifndef _HONYAR_MESH_H_
#define _HONYAR_MESH_H_

#include "honyar_types.h"
#include "ip_addr.h"
#include "espconn.h"
#include "mesh.h"

#define MESH_GROUP_ID_SIZE  ESP_MESH_ADDR_LEN

typedef struct honyar_mesh_info{
    uint8_t *ssid_prefix;
    uint32_t ssid_len;
    uint8_t *pwd;
    uint32_t pwd_len;
    struct ip_addr server;
    uint16_t port;
}honyar_mesh_info_t;

typedef void (*honyar_mesh_recv_handle_t)(const void *mesh_header, uint8_t *pdata, uint16_t len);

void honyar_mesh_recv(void *arg, char *data, unsigned short len);

int32_t honyar_mesh_init(honyar_mesh_info_t *info);

void honyar_mesh_regist_recv_cb(honyar_mesh_recv_handle_t cb);

void honyar_mesh_topo_query(struct espconn *network);

void honyar_mesh_get_gid(uint8_t gid[MESH_GROUP_ID_SIZE]);

#endif
