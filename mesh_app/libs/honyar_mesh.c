
#include "honyar_mesh.h"
#include "honyar_common.h"

#define HY_MESH_AUTH  AUTH_WPA2_PSK
#define HY_MESH_MAX_HOP  4

//"C8:3A:35:5F:18:D8"
static const uint8_t MESH_GROUP_ID[6] = {0x18,0xfe,0x34,0x00,0x00,0x50};
static honyar_mesh_recv_handle_t g_mesh_recv_handle;

static void ICACHE_FLASH_ATTR
honyar_mesh_topo_proto_parser(const void *mesh_header, uint8_t *pdata, uint16_t len)
{
    uint16_t op_idx = 1;
    uint16_t dev_count = 0;
    uint8_t *dev_mac = NULL;
    const uint8_t mac_len = ESP_MESH_ADDR_LEN;
    struct mesh_header_format *header = NULL;
    struct mesh_header_option_format *option = NULL;
    uint16_t i = 0;
    struct mesh_device_mac_type *list = NULL;
    
    if (!pdata)
        return;

    header = (struct mesh_header_format *)pdata;
    hy_info("root's mac:" MACSTR "\n", MAC2STR(header->src_addr));
    mesh_device_set_root((struct mesh_device_mac_type *)header->src_addr);

    while (espconn_mesh_get_option(header, M_O_TOPO_RESP, op_idx++, &option)) {
        dev_count = option->olen / mac_len;
        dev_mac = option->ovalue;
        list = (struct mesh_device_mac_type *)dev_mac;
        for(i = 0; i < dev_count; i++) {
            mesh_device_add(list + i);
        }
    }

    mesh_device_disp_mac_list();
}

static void ICACHE_FLASH_ATTR honyar_mesh_packet_parser(void *arg, uint8_t *pdata, uint16_t len)
{
    uint16_t i = 0;
    uint8_t src[ESP_MESH_ADDR_LEN] = {0};
    uint8_t *usr_data = NULL;
    uint16_t usr_data_len = 0;
    enum mesh_usr_proto_type proto;
    struct mesh_header_format *header = (struct mesh_header_format *)pdata;

    if (!espconn_mesh_get_usr_data_proto(header, &proto)) {
        hy_error("get proto failed.\r\n");
        return;
    }
    
    if (!espconn_mesh_get_usr_data(header, &usr_data, &usr_data_len)) {
        // mesh topology packet
        if(M_PROTO_NONE == proto) {
            //topology protocol, fix me;
            honyar_mesh_topo_proto_parser(header, pdata, len);
        }
        return;
    }

    if (honyar_wifi_get_macaddr(src)) {
        hy_error("get sta mac fail\n");
        return;
    }
    if(!os_memcmp(src, header->src_addr, ESP_MESH_ADDR_LEN)) {
        //ignore;
        return;
    }
    
    if(g_mesh_recv_handle) {
        g_mesh_recv_handle(header, usr_data, usr_data_len);
    }
}

void ICACHE_FLASH_ATTR honyar_mesh_recv(void *arg, char *data, unsigned short len)
{
    //hy_info("mesh recv total len: %d\r\n", len);
    honyar_mesh_packet_parser(arg, data, len);
}


int32_t ICACHE_FLASH_ATTR honyar_mesh_init(honyar_mesh_info_t *info)
{
    espconn_mesh_print_ver();

    //wifi_set_opmode_current(STATIONAP_MODE);
    if(NULL == info) {
        hy_error("no mesh info\r\n");
        return -1;
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
        return -1;
    }

    /*
     * set cloud server ip and port for mesh node
     */
    if (!espconn_mesh_server_init((struct ip_addr *)&info->server, info->port)) {
        hy_error("server_init fail\n");
        return -1;
    }

    return 0;
}

void ICACHE_FLASH_ATTR honyar_mesh_regist_recv_cb(honyar_mesh_recv_handle_t cb)
{
    g_mesh_recv_handle = cb;
}

void ICACHE_FLASH_ATTR mesh_topo_query(struct espconn *network)
{
    uint8_t src[ESP_MESH_ADDR_LEN] = {0};
    uint8_t dst[ESP_MESH_ADDR_LEN] = {0};
    struct mesh_header_format *header = NULL;
    struct mesh_header_option_format *option = NULL;
    uint8_t ot_len = sizeof(struct mesh_header_option_header_type) + sizeof(*option) + sizeof(dst); 

    if (honyar_wifi_get_macaddr(src)) {
        hy_error("get sta mac fail\n");
        return;
    }

    /*
     * root device uses espconn_mesh_get_node_info to get mac address of all devices
     */
    if (espconn_mesh_is_root()) {
        uint8_t *sub_dev_mac = NULL;
        uint16_t sub_dev_count = 0;
        uint16_t i = 0;
        struct mesh_device_mac_type *list = NULL;
        if (!espconn_mesh_get_node_info(MESH_NODE_ALL, &sub_dev_mac, &sub_dev_count))
            return;
        // the first one is mac address of router
        //mesh_disp_sub_dev_mac(sub_dev_mac, sub_dev_count);
        mesh_device_set_root((struct mesh_device_mac_type *)src);
        list = (struct mesh_device_mac_type *)sub_dev_mac;
        for(i = 0; i < sub_dev_count - 1; i++) {
            mesh_device_add(list + 1 + i);
        }

        mesh_device_disp_mac_list();

        // release memory occupied by mac address.
        espconn_mesh_get_node_info(MESH_NODE_ALL, NULL, NULL); 
        return;
    }

    /*
     * non-root device uses topology request with bcast to get mac address of all devices
     */
    os_memset(dst, 0, sizeof(dst));  // use bcast to get all the devices working in mesh from root.
    header = (struct mesh_header_format *)espconn_mesh_create_packet(
                            dst,     // destiny address (bcast)
                            src,     // source address
                            false,   // not p2p packet
                            true,    // piggyback congest request
                            M_PROTO_NONE,  // packe with JSON format
                            0,       // data length
                            true,    // no option
                            ot_len,  // option total len
                            false,   // no frag
                            0,       // frag type, this packet doesn't use frag
                            false,   // more frag
                            0,       // frag index
                            0);      // frag length
    if (!header) {
        hy_error("create packet fail\n");
        return;
    }

    option = (struct mesh_header_option_format *)espconn_mesh_create_option(
            M_O_TOPO_REQ, dst, sizeof(dst));
    if (!option) {
        hy_error("create option fail\n");
        goto TOPO_FAIL;
    }

    if (!espconn_mesh_add_option(header, option)) {
        hy_error("set option fail\n");
        goto TOPO_FAIL;
    }

    if (espconn_mesh_sent(network, (uint8_t *)header, header->len)) {
        hy_error("topo mesh is busy\n");
    }
TOPO_FAIL:
    option ? os_free(option) : 0;
    header ? os_free(header) : 0;
}

