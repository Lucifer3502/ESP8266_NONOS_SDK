
#ifndef _HONYAR_MESH_H_
#define _HONYAR_MESH_H_

#include "honyar_types.h"
#include "ip_addr.h"
#include "espconn.h"
#include "mesh.h"
#include "honyar_network.h"

#define MESH_GROUP_ID_SIZE  ESP_MESH_ADDR_LEN


typedef void (*honyar_mesh_recv_handle_t)(const void *mesh_header, uint8_t *pdata, uint16_t len);

void honyar_mesh_recv(void *arg, char *data, unsigned short len);

int32_t honyar_mesh_init(void);

void honyar_mesh_regist_recv_cb(honyar_mesh_recv_handle_t cb);

void honyar_mesh_topo_query(struct espconn *network);

void honyar_mesh_child_query(void);

void honyar_mesh_parent_query(void);

void honyar_mesh_get_gid(uint8_t gid[MESH_GROUP_ID_SIZE]);

void honyar_mesh_get_server_ipaddr(uint8_t ip[NET_IP_ADDR_LEN]);

uint16_t honyar_mesh_get_server_port(void);

#endif
