
#ifndef _MESH_APP_H_
#define _MESH_APP_H_

#include "honyar_types.h"

typedef void (*mesh_packet_recv_handle_t)( uint8_t *pdata, uint32_t len);

void mesh_regist_packet_recv_cb(mesh_packet_recv_handle_t cb);

uint32_t mesh_network_isconnected(void);

int32_t mesh_packet_send(uint8_t *data, uint32_t len);

int32_t mesh_app_init(void);

#endif

