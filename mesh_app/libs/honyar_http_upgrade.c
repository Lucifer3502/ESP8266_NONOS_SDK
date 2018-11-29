
#include "honyar_common.h"

#define HTTP_UPGRADE_TIMEOUT  20000

static esp_tcp g_http_iface;
static struct espconn g_http_network;
static uint32_t g_network_connected;


static os_timer_t g_http_timer;
static uint8_t g_http_reconnect_enable;


static int32_t http_upgrade_reconnect(void)
{
    if(g_http_reconnect_enable) {
        espconn_connect(&g_http_network);
    }
}

// notify at module that espconn has received data
static void ICACHE_FLASH_ATTR http_upgrade_recv(void *arg, char *data, unsigned short len)
{
	os_timer_disarm(&g_http_timer);
    os_timer_arm(&g_http_timer, HTTP_UPGRADE_TIMEOUT, false);
    hex_printf((uint8_t *)"http recv:", (uint8_t *)data, (uint32_t)len);
}

static void ICACHE_FLASH_ATTR http_upgrade_send_cb(void *arg)
{

}

static void ICACHE_FLASH_ATTR http_upgrade_discon_cb(void *arg)
{
  struct espconn *espconn_ptr = (struct espconn *)arg;

  hy_info("http_upgrade espconn disconnected\r\n");
  http_upgrade_reconnect();
}

static void ICACHE_FLASH_ATTR http_upgrade_connect_cb(void *arg)
{
	hy_info("http_upgrade espconn connected\r\n");
	espconn_set_opt((struct espconn*)arg, ESPCONN_COPY);
    g_network_connected = 1;
}

static void ICACHE_FLASH_ATTR http_upgrade_recon_cb(void *arg, sint8 err)
{
	struct espconn *espconn_ptr = (struct espconn *)arg;

	hy_info("at demo espconn reconnect\r\n");
	g_network_connected = 0;
    http_upgrade_reconnect();
}


static void ICACHE_FLASH_ATTR http_upgrade_destroy(void *arg)
{
    hy_info("http upgrade over\r\n");
    os_timer_disarm(&g_http_timer);

    g_http_reconnect_enable = 0;
    espconn_disconnect(&g_http_network);

    upgrading_unlock();
}

int32_t ICACHE_FLASH_ATTR http_upgrade_init(http_upgrade_info_t *info)
{
    uint32 ip = 0;
    
    if(try_upgrading_lock()) {
        return -1;
    }

    ip = ipaddr_addr(info->host);
    memset(&g_http_network, 0, sizeof(g_http_network));
    memset(&g_http_iface, 0, sizeof(g_http_iface));
    
    g_http_network.type = ESPCONN_TCP;
    g_http_network.state = ESPCONN_NONE;
    memcpy(g_http_iface.remote_ip, &ip, sizeof(ip));
    g_http_iface.remote_port = info->port;
    g_http_iface.local_port = espconn_port();
    g_http_network.proto.tcp = &g_http_iface;

    espconn_regist_connectcb(&g_http_network, http_upgrade_connect_cb);
    espconn_regist_reconcb(&g_http_network, http_upgrade_recon_cb);
    espconn_regist_disconcb(&g_http_network, http_upgrade_discon_cb);
    espconn_regist_recvcb(&g_http_network, http_upgrade_recv);
    espconn_regist_sentcb(&g_http_network, http_upgrade_send_cb);

    os_timer_disarm(&g_http_timer);
    os_timer_setfn(&g_http_timer, (os_timer_func_t *)http_upgrade_destroy, NULL);
    os_timer_arm(&g_http_timer, HTTP_UPGRADE_TIMEOUT, false);

    g_network_connected = 0;
    g_http_reconnect_enable = 1;
    espconn_connect(&g_http_network);
    
}

