
#include "honyar_mesh.h"
#include "honyar_common.h"

#define HY_MESH_AUTH  AUTH_WPA2_PSK
#define HY_MESH_MAX_HOP  4

//"C8:3A:35:5F:18:D8"
static const uint8_t MESH_GROUP_ID[6] = {0x18,0xfe,0x34,0x00,0x00,0x50};

int32_t ICACHE_FLASH_ATTR honyar_mesh_init(honyar_mesh_info_t *info)
{
    espconn_mesh_print_ver();
    if(NULL == info) {
        hy_error("no mesh info\r\n");
    }
    if (!espconn_mesh_encrypt_init(HY_MESH_AUTH, info->pwd, info->pwd_len)) {
        hy_error("set pw fail\r\n");
        return -1;
    }

    if (!espconn_mesh_set_max_hops(HY_MESH_MAX_HOP)) {
        hy_error("fail, max_hop:%d\r\n", espconn_mesh_get_max_hops());
        return -1;
    }

    if (!espconn_mesh_set_ssid_prefix(info->ssid_prefix, info->ssid_len)) {
        hy_error("set prefix fail\r\n");
        return -1;
    }

    /*
     * mesh_group_id
     * mesh_group_id and mesh_ssid_prefix represent mesh network
     */
    if (!espconn_mesh_group_id_init((uint8_t *)MESH_GROUP_ID, sizeof(MESH_GROUP_ID))) {
        hy_error("set grp id fail\n");
        return false;
    }

    /*
     * set cloud server ip and port for mesh node
     */
    if (!espconn_mesh_server_init((struct ip_addr *)&info->server, info->port)) {
        hy_error("server_init fail\n");
        return false;
    }
    
}

