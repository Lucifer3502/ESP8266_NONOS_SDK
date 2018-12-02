#include "honyar_common.h"
#include "mesh_app.h"

#define MESH_SSID_PREFIX "ZHIHE_"
#define MESH_AP_PWD     "zWG9kvcCFS"

#define MESH_ROUTER_SSID "zhihe_test"
#define MESH_ROUTER_PASS "123456zhihe"


#define MESH_SERVER_IP_ADDR  "192.168.0.3"
#define MESH_SERVER_PORT     8088

static esp_tcp g_mesh_iface;
static struct espconn g_mesh_network;
static uint32_t g_mesh_network_connected;

static int32_t mesh_app_reconnect(void)
{
    espconn_mesh_connect(&g_mesh_network);
}

// notify at module that espconn has received data
static void ICACHE_FLASH_ATTR mesh_recv(const void *arg, uint8_t *data, uint16_t len)
{
    const struct mesh_header_format *header = arg;
    hex_printf("mesh recv", data, len);
}

static void ICACHE_FLASH_ATTR mesh_send(uint8_t *data, uint16_t len)
{
    hy_info("mesh sending data ...\r\n");
    if (espconn_mesh_sent(&g_mesh_network, data, len)) {
        hy_error("mesh is busy\n");
        espconn_mesh_disconnect(&g_mesh_network);
        return;
    }
}

static void ICACHE_FLASH_ATTR mesh_send_cb(void *arg)
{
    hy_info("mesh send ok\r\n");
}

static void ICACHE_FLASH_ATTR mesh_discon_cb(void *arg)
{
  struct espconn *espconn_ptr = (struct espconn *)arg;

  hy_info("mesh espconn disconnected\r\n");
  g_mesh_network_connected = 0;
  mesh_app_reconnect();
}

static void ICACHE_FLASH_ATTR mesh_connect_cb(void *arg)
{
	hy_info("mesh espconn connected\r\n");
	espconn_set_opt((struct espconn*)arg, ESPCONN_COPY);
    g_mesh_network_connected = 1;
}

static void ICACHE_FLASH_ATTR mesh_recon_cb(void *arg, sint8 err)
{
	struct espconn *espconn_ptr = (struct espconn *)arg;

	hy_info("mesh espconn reconnect\r\n");
	g_mesh_network_connected = 0;
    mesh_app_reconnect();
}


static void ICACHE_FLASH_ATTR mesh_client_init(void)
{
    uint32_t ip = ipaddr_addr(MESH_SERVER_IP_ADDR);
    memset(&g_mesh_network, 0, sizeof(g_mesh_network));
    memset(&g_mesh_iface, 0, sizeof(g_mesh_iface));
    
    g_mesh_network.type = ESPCONN_TCP;
    g_mesh_network.state = ESPCONN_NONE;
    memcpy(g_mesh_iface.remote_ip, &ip, sizeof(ip));
    g_mesh_iface.remote_port = MESH_SERVER_PORT;
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

static void ICACHE_FLASH_ATTR mesh_app_task(void *parm)
{
    uint8_t status = espconn_mesh_get_status();
    //fix me;
    if(MESH_LOCAL_AVAIL != status && MESH_ONLINE_AVAIL != status) {
        return;
    }
    hy_debug("status = %d\r\n", status);
    hy_debug("mesh_client_init start\r\n");
    mesh_client_init();
    hy_debug("mesh_client_init over\r\n");

    honyar_del_task(mesh_app_task);
}

static void ICACHE_FLASH_ATTR mesh_app_topo_task(void *parm)
{
    uint8_t status = espconn_mesh_get_status();
    if(MESH_LOCAL_AVAIL != status && MESH_ONLINE_AVAIL != status) {
        return;
    }
    mesh_topo_query(&g_mesh_network);
}

static void ICACHE_FLASH_ATTR mesh_app_test(void *parm)
{
#if 0
    hy_info("mesh_app_test\r\n");
#else 
    struct mesh_header_format *header = NULL;
    uint8_t dst[ESP_MESH_ADDR_LEN] = {0};//broadcast
    uint8_t src[ESP_MESH_ADDR_LEN];
    uint8_t buf[16] = "hello world";
    uint32_t server = ipaddr_addr(MESH_SERVER_IP_ADDR);
    uint16_t port = MESH_SERVER_PORT;
    
    hy_info("mesh_app_test\r\n");
    if(espconn_mesh_is_root()) {
        hy_info("I am root node\r\n");
    } else {
        hy_info("I am sub node\r\n");
    }
    if(!g_mesh_network_connected) {
        return;
    }
    if (honyar_wifi_get_macaddr(src)) {
        return;
    }

    os_memcpy(dst, &server, sizeof(server));
    os_memcpy(dst + sizeof(server), &port, sizeof(port));
    
    header = (struct mesh_header_format *)espconn_mesh_create_packet(
                    dst,   // destiny address
                    src,   // source address
                    false, // not p2p packet
                    true,  // piggyback congest request
                    M_PROTO_BIN,  // packe with JSON format
                    os_strlen(buf),  // data length
                    false, // no option
                    0,     // option len
                    false, // no frag
                    0,     // frag type, this packet doesn't use frag
                    false, // more frag
                    0,     // frag index
                    0);    // frag length
    if (!header) {
        hy_error("create packet fail\n");
        return;
    }

    if (!espconn_mesh_set_usr_data(header, buf, os_strlen(buf))) {
        hy_error("set user data fail\n");
        goto end;
    }

    mesh_send((void *)header, header->len);
 
end:
    os_free(header);
#endif
}

static void ICACHE_FLASH_ATTR mesh_app_callback(int8_t res)
{
    uint8_t status = espconn_mesh_get_status();
    hy_info("mesh status: %d\r\n", status);
}


int32_t ICACHE_FLASH_ATTR mesh_app_init(void)
{
    struct station_config sta_conf;
    honyar_mesh_info_t info;

    os_memset(&sta_conf, 0, sizeof(struct station_config));
	os_sprintf(sta_conf.ssid, "%s", MESH_ROUTER_SSID);
	os_sprintf(sta_conf.password, "%s", MESH_ROUTER_PASS);
    
    memset(&info, 0, sizeof(info));
    info.ssid_prefix = MESH_SSID_PREFIX;
    info.ssid_len = strlen(MESH_SSID_PREFIX);
    info.pwd = MESH_AP_PWD;
    info.pwd_len = strlen(MESH_AP_PWD);
    info.server.addr = ipaddr_addr(MESH_SERVER_IP_ADDR);
    info.port = MESH_SERVER_PORT;
    
    if(honyar_mesh_init(&info)){
        return -1;
    }

    espconn_mesh_set_router(&sta_conf);
    espconn_mesh_enable(mesh_app_callback, MESH_ONLINE);
    honyar_add_task(mesh_app_task, NULL, 1000 / TASK_CYCLE_TM_MS);

    honyar_add_task(mesh_app_test, NULL, 20000 / TASK_CYCLE_TM_MS);

    honyar_add_task(mesh_app_topo_task, NULL, 5000 / TASK_CYCLE_TM_MS);
    
    return 0;
}


