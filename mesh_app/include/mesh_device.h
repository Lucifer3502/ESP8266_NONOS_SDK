#ifndef _MESH_DEVICE_H_
#define _MESH_DEVICE_H_

#include "honyar_types.h"


struct mesh_device_mac_type {
    uint8_t mac[6];
} __packed;

typedef struct mesh_device_list_type {
    uint16_t size;
    uint16_t scale;  // include root
    struct mesh_device_mac_type root;
    struct mesh_device_mac_type *list;
}mesh_device_list_type_t;

mesh_device_list_type_t *mesh_device_get_all(void);

mesh_device_list_type_t *mesh_device_get_child(void);

void mesh_device_list_init(void);

void mesh_device_disp_mac_list(void);

void mesh_device_set_root(struct mesh_device_mac_type *root);

void mesh_device_set_parent(struct mesh_device_mac_type *parent);

int32_t mesh_search_device(mesh_device_list_type_t *node_list, const struct mesh_device_mac_type *node);

int32_t mesh_device_add(struct mesh_device_mac_type *nodes, uint8_t node_type);

int32_t mesh_device_del(struct mesh_device_mac_type *nodes, uint16_t count);

int32_t mesh_device_get_root(const struct mesh_device_mac_type **root);

int32_t mesh_device_get_mac_list(const struct mesh_device_mac_type **list, uint16_t *count);


#endif

