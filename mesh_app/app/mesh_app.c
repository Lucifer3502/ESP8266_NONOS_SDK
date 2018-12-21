#include "honyar_common.h"
#include "mesh_app.h"


static esp_tcp g_mesh_iface;
static struct espconn g_mesh_network;
static uint32_t g_mesh_network_connected;

static mesh_packet_recv_handle_t g_mesh_packet_recv_handle;

static int32_t mesh_send(uint8_t *data, uint16_t len);

void ICACHE_FLASH_ATTR 
mesh_regist_packet_recv_cb(mesh_packet_recv_handle_t cb)
{
    g_mesh_packet_recv_handle = cb;
}

uint32_t ICACHE_FLASH_ATTR
mesh_network_isconnected(void)
{
    return g_mesh_network_connected;
}

int32_t ICACHE_FLASH_ATTR
mesh_packet_send(uint8_t *data, uint32_t len)
{
    struct mesh_header_format *header = NULL;
    uint8_t dst[ESP_MESH_ADDR_LEN] = {0};//broadcast
    uint8_t src[ESP_MESH_ADDR_LEN];
    int32_t ret = -1;
    uint8_t ipaddr[NET_IP_ADDR_LEN] = {0};
    uint32_t addr = 0;
    uint16_t port = honyar_mesh_get_server_port();
    honyar_mesh_get_server_ipaddr(ipaddr);
    addr = ipaddr_addr(ipaddr);

    if(NULL == data) {
        return -1;
    }
    if(!mesh_network_isconnected()) {
        return -1;
    }
    if (honyar_wifi_get_macaddr(src)) {
        return -1;
    }

    os_memcpy(dst, &addr, sizeof(addr));
    os_memcpy(dst + sizeof(addr), &port, sizeof(port));
    
    header = (struct mesh_header_format *)espconn_mesh_create_packet(
                    dst,   // destiny address
                    src,   // source address
                    false, // not p2p packet
                    true,  // piggyback congest request
                    M_PROTO_BIN,  // packe with JSON format
                    len,  // data length
                    false, // no option
                    0,     // option len
                    false, // no frag
                    0,     // frag type, this packet doesn't use frag
                    false, // more frag
                    0,     // frag index
                    0);    // frag length
    if (!header) {
        hy_error("create packet fail\n");
        return -1;
    }

    if (!espconn_mesh_set_usr_data(header, data, len)) {
        hy_error("set user data fail\n");
        goto end;
    }

    ret = mesh_send((void *)header, header->len);
    
end:
    os_free(header);
    return ret;
}


int32_t ICACHE_FLASH_ATTR
mesh_app_reconnect(void)
{
    espconn_mesh_connect(&g_mesh_network);
}

// notify at module that espconn has received data
static void ICACHE_FLASH_ATTR 
mesh_recv(const void *arg, uint8_t *data, uint16_t len)
{
    const struct mesh_header_format *header = arg;
    hex_printf("mesh recv", data, len);
    if(g_mesh_packet_recv_handle) {
        g_mesh_packet_recv_handle(data, len);
    }
}

static int32_t ICACHE_FLASH_ATTR 
mesh_send(uint8_t *data, uint16_t len)
{
    hy_info("mesh sending data ...\r\n");
    if (espconn_mesh_sent(&g_mesh_network, data, len)) {
        hy_error("mesh is busy\n");
        espconn_mesh_disconnect(&g_mesh_network);
        return -1;
    }
    return 0;
}

static void ICACHE_FLASH_ATTR 
mesh_send_cb(void *arg)
{
    hy_info("mesh send ok\r\n");
}

static void ICACHE_FLASH_ATTR 
mesh_discon_cb(void *arg)
{
  struct espconn *espconn_ptr = (struct espconn *)arg;

  hy_info("mesh espconn disconnected\r\n");
  g_mesh_network_connected = 0;
  mesh_app_reconnect();
}

static void ICACHE_FLASH_ATTR 
mesh_connect_cb(void *arg)
{
	hy_info("mesh espconn connected\r\n");
	espconn_set_opt((struct espconn*)arg, ESPCONN_COPY);
    g_mesh_network_connected = 1;
}

static void ICACHE_FLASH_ATTR 
mesh_recon_cb(void *arg, sint8 err)
{
	struct espconn *espconn_ptr = (struct espconn *)arg;

	hy_info("mesh espconn reconnect\r\n");
	g_mesh_network_connected = 0;
    mesh_app_reconnect();
}


static void ICACHE_FLASH_ATTR 
mesh_client_init(void)
{
    uint8_t ipaddr[NET_IP_ADDR_LEN] = {0};
    uint32_t addr = 0;
    honyar_mesh_get_server_ipaddr(ipaddr);
    addr = ipaddr_addr(ipaddr);
    memset(&g_mesh_network, 0, sizeof(g_mesh_network));
    memset(&g_mesh_iface, 0, sizeof(g_mesh_iface));
    
    g_mesh_network.type = ESPCONN_TCP;
    g_mesh_network.state = ESPCONN_NONE;
    memcpy(g_mesh_iface.remote_ip, &addr, sizeof(addr));
    g_mesh_iface.remote_port = honyar_mesh_get_server_port();
    g_mesh_iface.local_port = espconn_port();
    g_mesh_network.proto.tcp = &g_mesh_iface;

    espconn_regist_connectcb(&g_mesh_network, mesh_connect_cb);
    espconn_regist_reconcb(&g_mesh_network, mesh_recon_cb);
    espconn_regist_disconcb(&g_mesh_network, mesh_discon_cb);
    espconn_regist_recvcb(&g_mesh_network, honyar_mesh_recv);
    espconn_regist_sentcb(&g_mesh_network, mesh_send_cb);
    honyar_mesh_regist_recv_cb(mesh_recv);
    
    espconn_mesh_connect(&g_mesh_network);
}

static void ICACHE_FLASH_ATTR 
mesh_app_task(void *parm)
{
    if(honyar_mesh_is_valid()) {
        hy_debug("mesh_client_init start\r\n");
        mesh_client_init();
        hy_debug("mesh_client_init over\r\n");

        honyar_del_task(mesh_app_task);
    }
}

static void ICACHE_FLASH_ATTR 
mesh_app_topo_task(void *parm)
{
    hy_info("mesh status: %d - %d\r\n", honyar_mesh_is_valid(), espconn_mesh_get_status());
    if(honyar_mesh_is_valid()) {
        honyar_mesh_topo_query(&g_mesh_network);
    }
}

int32_t ICACHE_FLASH_ATTR 
mesh_app_init(void)
{
    if(honyar_mesh_init()){
        return -1;
    }
    
    honyar_add_task(mesh_app_task, NULL, 1000 / TASK_CYCLE_TM_MS);

    honyar_add_task(mesh_app_topo_task, NULL, 10000 / TASK_CYCLE_TM_MS);
    
    return 0;
}


