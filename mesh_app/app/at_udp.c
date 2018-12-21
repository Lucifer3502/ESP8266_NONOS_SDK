
#include "honyar_common.h"
#include "at_udp.h"

#define AT_UDP_PORT  20000

static esp_udp g_at_udp_iface;
static struct espconn g_at_udp_network;

int32_t ICACHE_FLASH_ATTR
at_udp_send(uint8_t *data, uint16_t len)
{
    remot_info *premote = NULL;
    
    if (espconn_get_connection_info(&g_at_udp_network, &premote, 0)) {
        return -1;
    }
    os_memcpy(g_at_udp_network.proto.udp->remote_ip, premote->remote_ip, 4);
    g_at_udp_network.proto.udp->remote_port = premote->remote_port;
    return espconn_sent(&g_at_udp_network, data, len);
}

static void ICACHE_FLASH_ATTR
at_udp_recv(void *arg, char *data, unsigned short len)
{
    at_recv_handle(data, len);
}


void ICACHE_FLASH_ATTR
at_udp_init(void)
{
    memset(&g_at_udp_network, 0, sizeof(g_at_udp_network));
    memset(&g_at_udp_iface, 0, sizeof(g_at_udp_iface));

    g_at_udp_network.type = ESPCONN_UDP;
    g_at_udp_network.state = ESPCONN_NONE;
    g_at_udp_iface.local_port = AT_UDP_PORT;
    g_at_udp_network.proto.udp = &g_at_udp_iface;

    espconn_regist_recvcb(&g_at_udp_network, at_udp_recv);
    if(espconn_create(&g_at_udp_network)) {
        hy_error("udp create failed.\r\n");
    }
}
