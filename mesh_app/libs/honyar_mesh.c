
#include "honyar_mesh.h"
#include "honyar_common.h"

#define HY_MESH_AUTH  AUTH_WPA2_PSK
#define HY_MESH_MAX_HOP  4
#define HY_MESH_SSID_PREFIX_DEF "_HONYAR"
#define HY_MESH_AP_PASSWD_DEF   "zWG9kvcCFS"



//MESH CONFIG LIST START:
static uint8_t g_mesh_groupid[MESH_GROUP_ID_SIZE] = {0x18,0xfe,0x34,0x00,0x00,0x50};
static uint8_t g_mesh_ssid_prefix[WIFI_SSID_LEN] = HY_MESH_SSID_PREFIX_DEF;
static uint8_t g_mesh_server_ipaddr[NET_IP_ADDR_LEN] = "58.214.239.34";
static uint16_t g_mesh_server_port = 9002;
//MESH CONFIG LIST END.


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
    hy_info("root's mac:" MACSTR "\r\n", MAC2STR(header->src_addr));
    mesh_device_set_root((struct mesh_device_mac_type *)header->src_addr);

    while (espconn_mesh_get_option(header, M_O_TOPO_RESP, op_idx++, &option)) {
        dev_count = option->olen / mac_len;
        dev_mac = option->ovalue;
        list = (struct mesh_device_mac_type *)dev_mac;
        for(i = 0; i < dev_count; i++) {
            mesh_device_add(list + i, MESH_NODE_ALL);
        }
    }

    mesh_device_disp_mac_list();
}

static void ICACHE_FLASH_ATTR
honyar_mesh_packet_parser(void *arg, uint8_t *pdata, uint16_t len)
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
        hy_error("get sta mac fail\r\n");
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

void ICACHE_FLASH_ATTR
honyar_mesh_recv(void *arg, char *data, unsigned short len)
{
    //hy_info("mesh recv total len: %d\r\n", len);
    honyar_mesh_packet_parser(arg, data, len);
}

static void ICACHE_FLASH_ATTR 
honyar_mesh_callback(int8_t res)
{
    uint8_t status = espconn_mesh_get_status();
    hy_info("mesh status: %d\r\n", status);
}

int32_t ICACHE_FLASH_ATTR
honyar_mesh_init(void)
{
    struct ip_addr server;
    struct station_config sta_conf;
    
    espconn_mesh_print_ver();
    
    server.addr = ipaddr_addr(g_mesh_server_ipaddr);
    os_memset(&sta_conf, 0, sizeof(struct station_config));
	os_sprintf(sta_conf.ssid, "%s", honyar_wifi_get_router_ssid());
	os_sprintf(sta_conf.password, "%s", honyar_wifi_get_router_passwd());

    //wifi_set_opmode_current(STATIONAP_MODE);
    if (!espconn_mesh_encrypt_init(HY_MESH_AUTH, HY_MESH_AP_PASSWD_DEF, os_strlen(HY_MESH_AP_PASSWD_DEF))) {
        hy_error("mesh set pw fail\r\n");
        return -1;
    }

    if (!espconn_mesh_set_max_hops(HY_MESH_MAX_HOP)) {
        hy_error("mesh fail, max_hop:%d\r\n", espconn_mesh_get_max_hops());
        return -1;
    }

    if (!espconn_mesh_set_ssid_prefix(g_mesh_ssid_prefix, os_strlen(g_mesh_ssid_prefix))) {
        hy_error("mesh set prefix fail\r\n");
        return -1;
    }

    /*
     * mesh_group_id
     * mesh_group_id and mesh_ssid_prefix represent mesh network
     */
    if (!espconn_mesh_group_id_init(g_mesh_groupid, MESH_GROUP_ID_SIZE)) {
        hy_error("mesh set grp id fail\n");
        return -1;
    }

    /*
     * set cloud server ip and port for mesh node
     */
    if (!espconn_mesh_server_init((struct ip_addr *)&server, g_mesh_server_port)) {
        hy_error("mesh server_init fail\n");
        return -1;
    }

    if(!espconn_mesh_set_router(&sta_conf)) {
        hy_error("mesh set router fail\n");
        return -1;
    }
    
    espconn_mesh_enable(honyar_mesh_callback, MESH_ONLINE);

    return 0;
}

void ICACHE_FLASH_ATTR
honyar_mesh_regist_recv_cb(honyar_mesh_recv_handle_t cb)
{
    g_mesh_recv_handle = cb;
}

void ICACHE_FLASH_ATTR
honyar_mesh_topo_query(struct espconn *network)
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
        if (!espconn_mesh_get_node_info(MESH_NODE_ALL, &sub_dev_mac, &sub_dev_count)) {
            hy_error("get mesh all node failed\r\n");
            return;
        }
        // the first one is mac address of root
        //mesh_disp_sub_dev_mac(sub_dev_mac, sub_dev_count);
        mesh_device_set_root((struct mesh_device_mac_type *)src);
        list = (struct mesh_device_mac_type *)sub_dev_mac;
        for(i = 0; i < sub_dev_count - 1; i++) {
            mesh_device_add(list + 1 + i, MESH_NODE_ALL);
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

void ICACHE_FLASH_ATTR
honyar_mesh_child_query(struct espconn *network)
{
    uint8_t *sub_dev_mac = NULL;
    uint16_t sub_dev_count = espconn_mesh_get_sub_dev_count();
    uint16_t i = 0;
    struct mesh_sub_node_info *list = NULL;
    if (!espconn_mesh_get_node_info(MESH_NODE_CHILD, &sub_dev_mac, &sub_dev_count)) {
        hy_error("get mesh child node failed\r\n");
        return;
    }
    hy_info("child count: %d\r\n", sub_dev_count);
    list = (struct mesh_sub_node_info *)sub_dev_mac;
    for(i = 0; i < sub_dev_count; i++) {
        mesh_device_add((struct mesh_device_mac_type *)((list + i)->mac), MESH_NODE_CHILD);
    }

    mesh_device_disp_child_list();

    // release memory occupied by mac address.
    espconn_mesh_get_node_info(MESH_NODE_CHILD, NULL, NULL); 
    return;
}

void ICACHE_FLASH_ATTR
honyar_mesh_parent_query(struct espconn *network)
{
    uint8_t *sub_dev_mac = NULL;
    uint16_t sub_dev_count = 0;
    struct mesh_device_mac_type *list = NULL;

    if (espconn_mesh_is_root()) {
        return;
    }
    if (!espconn_mesh_get_node_info(MESH_NODE_PARENT, &sub_dev_mac, &sub_dev_count)) {
        hy_error("get mesh parent node failed\r\n");
        return;
    }
    hy_info("parent count: %d\r\n", sub_dev_count);
    list = (struct mesh_device_mac_type *)sub_dev_mac;
    mesh_device_set_parent(list);

    mesh_device_disp_child_list();

    // release memory occupied by mac address.
    espconn_mesh_get_node_info(MESH_NODE_PARENT, NULL, NULL); 
    return;
}


void ICACHE_FLASH_ATTR
honyar_mesh_config_regist(void)
{
    DL_CONFIG_ITEM_S config_items[] = 
	{
        {"CFG_MESH_GROUP_ID", DL_CFG_ITEM_TYPE_HEX_STR, g_mesh_groupid, sizeof(g_mesh_groupid), 0},
        {"CFG_MESH_SSID_PREFIX", DL_CFG_ITEM_TYPE_STRING, g_mesh_ssid_prefix, sizeof(g_mesh_ssid_prefix), 0},
        {"CFG_MESH_SERVER_IPADDR", DL_CFG_ITEM_TYPE_STRING, g_mesh_server_ipaddr, sizeof(g_mesh_server_ipaddr), 0},
        {"CFG_MESH_SERVER_PORT", DL_CFG_ITEM_TYPE_DEC16, &g_mesh_server_port, sizeof(g_mesh_server_port), 0},
	};

	uint32_t i;
	for (i = 0; i < sizeof(config_items)/sizeof(config_items[0]); i++)
	{
        dl_config_items_register_by_user(&config_items[i]);
    }
}

uint8_t ICACHE_FLASH_ATTR
honyar_mesh_is_valid(void)
{
    uint8_t status = espconn_mesh_get_status();
    if(MESH_LOCAL_AVAIL != status && MESH_ONLINE_AVAIL != status) {
        return 0;
    }

    return 1;
}

void ICACHE_FLASH_ATTR
honyar_mesh_get_gid(uint8_t gid[MESH_GROUP_ID_SIZE])
{
    os_memcpy(gid, g_mesh_groupid, MESH_GROUP_ID_SIZE);
}

void ICACHE_FLASH_ATTR
honyar_mesh_get_server_ipaddr(uint8_t ip[NET_IP_ADDR_LEN])
{
    os_memcpy(ip, g_mesh_server_ipaddr, NET_IP_ADDR_LEN);
}

uint16_t ICACHE_FLASH_ATTR
honyar_mesh_get_server_port(void)
{
    return g_mesh_server_port;
}


