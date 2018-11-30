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
static void ICACHE_FLASH_ATTR mesh_recv(void *arg, char *data, unsigned short len)
{
    hex_printf((uint8_t *)"mesh recv:", (uint8_t *)data, (uint32_t)len);
}

static void ICACHE_FLASH_ATTR mesh_send_cb(void *arg)
{

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

	hy_info("at demo espconn reconnect\r\n");
	g_mesh_network_connected = 0;
    mesh_app_reconnect();
}


static void mesh_client_init(void)
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
    espconn_regist_recvcb(&g_mesh_network, mesh_recv);
    espconn_regist_sentcb(&g_mesh_network, mesh_send_cb);

    espconn_mesh_connect(&g_mesh_network);
}

static void mesh_app_task(void *parm)
{
    uint8_t status = espconn_mesh_get_status();
    //fix me;
    if(MESH_DISABLE == status) {
        return;
    }
    hy_debug("status = %d\r\n", status);
    hy_debug("mesh_client_init start\r\n");
    //mesh_client_init();
    hy_debug("mesh_client_init over\r\n");

    honyar_del_task(mesh_app_task);
}

static void mesh_app_callback(void)
{
    uint8_t status = espconn_mesh_get_status();
    hy_info("mesh status: %d\r\n", status);
}


int32_t mesh_app_init(void)
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
    espconn_mesh_enable(mesh_app_callback, MESH_LOCAL);
    honyar_add_task(mesh_app_task, NULL, 1000 / TASK_CYCLE_TM_MS);

    return 0;
}


