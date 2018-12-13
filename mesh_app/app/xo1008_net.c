

#include "xo1008_net.h"
#include "honyar_common.h"
#include "mesh_app.h"
#include "app_config.h"
#include "xo1008_uart.h"
#include "xo1008_upgrade.h"


#define XO1008_HEARTBEART_TIME 30000
#define MACSTR2 "%02X:%02X:%02X:%02X:%02X:%02X"

typedef enum {
    MESH_HEARTBEAT = 0x00,
    MESH_HEARTBEAT_RESP = 0x8000,
    MESH_DATA_UPLOAD = 0x01,
    MESH_DATA_UPLOAD_RESP = 0x8001,
    MESH_DATA_DOWNLOAD = 0x02,
    MESH_DATA_DOWNLOAD_RESP = 0x8002,
    MESH_DEVICE_UPGRADE = 0x03,
    MESH_DEVICE_UPGRADE_RESP = 0x8003,
    //add more cmd below
}mesh_net_cmd_t;


static uint32_t g_xo1008_net_seq;
static uint32_t g_xo1008_hearbeat_flag;

static void ICACHE_FLASH_ATTR
cjson_add_protocol(cJSON *root)
{
    cJSON_AddItemToObject(root, "protocol", cJSON_CreateString(XO1008_PROTOCOL_VERSION));
}

static void ICACHE_FLASH_ATTR
cjson_add_cmd(cJSON *root, uint32_t cmd)
{
    cJSON_AddItemToObject(root, "cmd", cJSON_CreateNumber(cmd));
}

static void ICACHE_FLASH_ATTR
cjson_add_seq(cJSON *root, uint32_t seq)
{
    cJSON_AddItemToObject(root, "seq", cJSON_CreateNumber(seq));
}

static void ICACHE_FLASH_ATTR
cjson_add_model(cJSON *root)
{
    cJSON_AddItemToObject(root, "model", cJSON_CreateString(honyar_device_get_model()));
}

static void ICACHE_FLASH_ATTR
cjson_add_sw(cJSON *root)
{
    cJSON_AddItemToObject(root, "sw", cJSON_CreateString(APP_VERSION));
}

static void ICACHE_FLASH_ATTR
cjson_add_sn(cJSON *root)
{
    cJSON_AddItemToObject(root, "sn", cJSON_CreateString(honyar_device_get_sn()));
}

static void ICACHE_FLASH_ATTR
cjson_add_mac(cJSON *root)
{
    uint8_t mac[MAC_ADDR_LEN] = {0};
    uint8_t macstr[MAC_ADDR_LEN * 3] = {0};
    honyar_wifi_get_macaddr(mac);
    os_sprintf(macstr, MACSTR2, MAC2STR(mac));
    cJSON_AddItemToObject(root, "mac", cJSON_CreateString(macstr));
}

static void ICACHE_FLASH_ATTR
cjson_add_mesh_gid(cJSON *root)
{
    uint8_t gid[MESH_GROUP_ID_SIZE] = {0};
    uint8_t gidstr[MESH_GROUP_ID_SIZE * 3] = {0};
    honyar_mesh_get_gid(gid);
    os_sprintf(gidstr, MACSTR2, MAC2STR(gid));
    cJSON_AddItemToObject(root, "mesh_gid", cJSON_CreateString(gidstr));
}

static void ICACHE_FLASH_ATTR
cjson_add_wifi_ssid(cJSON *root)
{
    cJSON_AddItemToObject(root, "ssid", cJSON_CreateString(honyar_wifi_get_router_ssid()));
}

static void ICACHE_FLASH_ATTR
cjson_add_hexdata(cJSON *root, uint8_t *data, uint32_t len)
{
    uint8_t *hexdata = honyar_malloc(len * 2 + 4);
    memset(hexdata, 0, len * 2 + 4);
    hy_byte2hex(hexdata, len * 2, data, len);
    cJSON_AddItemToObject(root, "data", cJSON_CreateString(hexdata));
    honyar_free(hexdata);
}

static void ICACHE_FLASH_ATTR
xo1008_data_download_resp(int32_t err)
{
    cJSON *root = cJSON_CreateObject();
    char *fmt = NULL;

    if(!root) {
        hy_error("cJSON_CreateObject failed.\r\n");
        goto end;
    }
    
    cjson_add_protocol(root);
    cjson_add_cmd(root, MESH_DATA_DOWNLOAD_RESP);
    cjson_add_seq(root, g_xo1008_net_seq++);
    cjson_add_model(root);
    cjson_add_sw(root);
    cjson_add_sn(root);
    cjson_add_mac(root);
    cjson_add_mesh_gid(root);
    cjson_add_wifi_ssid(root);
    cJSON_AddItemToObject(root, "data", cJSON_CreateString(""));
    cJSON_AddItemToObject(root, "errcode", cJSON_CreateNumber(err));
    
    fmt = cJSON_PrintUnformatted(root);
    if(!fmt) {
        goto end;
    }

    mesh_packet_send(fmt, os_strlen(fmt));
    
end:
    if(root) {
        cJSON_Delete(root);
    }
    if(fmt) {
        honyar_free(fmt);
    }
}

static void ICACHE_FLASH_ATTR
xo1008_device_upgrade_resp(int32_t err)
{
    cJSON *root = cJSON_CreateObject();
    char *fmt = NULL;

    if(!root) {
        hy_error("cJSON_CreateObject failed.\r\n");
        goto end;
    }
    
    cjson_add_protocol(root);
    cjson_add_cmd(root, MESH_DEVICE_UPGRADE_RESP);
    cjson_add_seq(root, g_xo1008_net_seq++);
    cjson_add_model(root);
    cjson_add_sw(root);
    cjson_add_sn(root);
    cjson_add_mac(root);
    cjson_add_mesh_gid(root);
    cjson_add_wifi_ssid(root);
    cJSON_AddItemToObject(root, "data", cJSON_CreateString(""));
    cJSON_AddItemToObject(root, "errcode", cJSON_CreateNumber(err));
    
    fmt = cJSON_PrintUnformatted(root);
    if(!fmt) {
        goto end;
    }

    mesh_packet_send(fmt, os_strlen(fmt));
    
end:
    if(root) {
        cJSON_Delete(root);
    }
    if(fmt) {
        honyar_free(fmt);
    }
}

static void ICACHE_FLASH_ATTR
xo1008_net_recv(uint8_t *data, uint32_t len)
{
    cJSON *root = cJSON_Parse(data);
    cJSON *item = NULL;
    uint32_t cmd = 0;
    int32_t err = 0;
    uint8_t *ddata = NULL;
    uint32_t ddata_len = 0;
    
    if(!root) {
        hy_error("cJSON_Parse failed\r\n");
        return;
    }
    
    item = cJSON_GetObjectItem(root, "cmd");
    if(NULL == item) {
        hy_error("err json data\r\n");
        goto end;
    }
    cmd = item->valueint;
    switch(cmd) {
    case MESH_HEARTBEAT_RESP:
        hy_info("heart beat.\r\n");
        g_xo1008_hearbeat_flag = 0;
        break;
    
    case MESH_DATA_UPLOAD_RESP:
        //do nothing;
        break;
        
    case MESH_DATA_DOWNLOAD:
        item = cJSON_GetObjectItem(root, "data");
        if(NULL == item) {
            hy_error("err get json data\r\n");
            goto end;
        }
        ddata_len = os_strlen(item->valuestring) / 2;
        ddata = (uint8_t *)os_malloc(ddata_len);
        if(hy_hex2byte(ddata, ddata_len, item->valuestring, os_strlen(item->valuestring))) {
            os_free(ddata);
            break;
        }
        
        err = xo1008_uart_download(ddata, ddata_len);
        xo1008_data_download_resp(err);
        os_free(ddata);
        break;
    
    case MESH_DEVICE_UPGRADE:
        item = cJSON_GetObjectItem(root, "data");
        if(NULL == item) {
            hy_error("err get json data\r\n");
            break;
        }
        err = xo1008_upgrade_set_url(item->valuestring);
        xo1008_device_upgrade_resp(err);
        if(0 == err) {
            honyar_wifi_set_work_status(WIFI_STA_STATUS);
            dl_config_commit(1);
            honyar_sys_reboot(0);
        }
        break;
        
    default:
        hy_error("net recv cmd: %d\r\n", cmd);
        break;
    }

end:
    cJSON_Delete(root);
}


static void ICACHE_FLASH_ATTR
xo1008_heartbeat(void)
{
    cJSON *root = cJSON_CreateObject();
    char *fmt = NULL;
    if(!root) {
        hy_error("cJSON_CreateObject failed.\r\n");
        goto end;
    }

    cjson_add_protocol(root);
    cjson_add_cmd(root, MESH_HEARTBEAT);
    cjson_add_seq(root, g_xo1008_net_seq++);
    cjson_add_model(root);
    cjson_add_sw(root);
    cjson_add_sn(root);
    cjson_add_mac(root);
    cjson_add_mesh_gid(root);
    cjson_add_wifi_ssid(root);
    
    fmt = cJSON_PrintUnformatted(root);
    if(!fmt) {
        goto end;
    }

    mesh_packet_send(fmt, os_strlen(fmt));
    hy_info("heart send.\r\n");
end:
    if(root) {
        cJSON_Delete(root);
    }
    if(fmt) {
        honyar_free(fmt);
    }
}

void ICACHE_FLASH_ATTR
xo1008_net_upload(uint8_t *data, uint32_t len)
{
    cJSON *root = cJSON_CreateObject();
    char *fmt = NULL;

    if(!data || !len) {
        hy_error("none data\r\n");
        goto end;
    }
    
    if(!root) {
        hy_error("cJSON_CreateObject failed.\r\n");
        goto end;
    }
    
    cjson_add_protocol(root);
    cjson_add_cmd(root, MESH_DATA_UPLOAD);
    cjson_add_seq(root, g_xo1008_net_seq++);
    cjson_add_model(root);
    cjson_add_sw(root);
    cjson_add_sn(root);
    cjson_add_mac(root);
    cjson_add_mesh_gid(root);
    cjson_add_wifi_ssid(root);
    cjson_add_hexdata(root, data, len);
    
    fmt = cJSON_PrintUnformatted(root);
    if(!fmt) {
        goto end;
    }

    mesh_packet_send(fmt, os_strlen(fmt));
    
end:
    if(root) {
        cJSON_Delete(root);
    }
    if(fmt) {
        honyar_free(fmt);
    }
}

static void ICACHE_FLASH_ATTR
xo1008_heart(void *parm)
{
    static uint32_t l_tm = 0;
    uint32_t c_tm;

    if(!mesh_network_isconnected()) {
        return;
    }

    if(g_xo1008_hearbeat_flag > 3) {
        //timeout;
        mesh_app_reconnect();
        g_xo1008_hearbeat_flag = 0;
        return;
    }

    c_tm = system_get_time();
    if(c_tm - l_tm > XO1008_HEARTBEART_TIME * 1000) {
        g_xo1008_hearbeat_flag++;
        xo1008_heartbeat();
        l_tm = c_tm;
    }
    
}

void ICACHE_FLASH_ATTR
xo1008_net_init(void)
{
    mesh_regist_packet_recv_cb(xo1008_net_recv);
    honyar_add_task(xo1008_heart, NULL, 1000 / TASK_CYCLE_TM_MS);
    mesh_app_init();
}

