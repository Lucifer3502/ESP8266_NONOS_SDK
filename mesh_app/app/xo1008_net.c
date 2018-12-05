

#include "xo1008_net.h"
#include "honyar_common.h"
#include "mesh_app.h"

#define XO1007_HEARTBEART_TIME 30000

static uint32_t g_xo1008_net_seq;

static void ICACHE_FLASH_ATTR
xo1008_net_recv(uint8_t *data, uint32_t len)
{
    cJSON *root = cJSON_Parse(data);
    if(!root) {
        hy_error("cJSON_Parse failed\r\n");
        return;
    }

    cJSON *item = cJSON_GetObjectItem(root, "Cmd");
    if(NULL == item) {
        hy_error("err json data\r\n");
        goto end;
    }
    
    if(!os_strcmp(item->valuestring, "echo")) {
        hy_info("xo1007 heart beat.\r\n");
        goto end;
    } else if (os_strcmp(item->valuestring, "data")) {
        hy_error("unknow cmd: %s\r\n", item->valuestring);
    }
    item = cJSON_GetObjectItem(root, "Data");
    if(NULL == item) {
        hy_error("err get json data\r\n");
        goto end;
    }

end:
    cJSON_Delete(root);
}


static void ICACHE_FLASH_ATTR
xo1007_heartbeat(void)
{
    cJSON *root = cJSON_CreateObject();
    char *fmt = NULL;
    if(!root) {
        hy_error("cJSON_CreateObject failed.\r\n");
        goto end;
    }
    cJSON_AddItemToObject(root, "Cmd", cJSON_CreateString("echo"));
    cJSON_AddItemToObject(root, "Dir", cJSON_CreateString("up"));
    cJSON_AddItemToObject(root, "Frame", cJSON_CreateNumber(g_xo1008_net_seq++));

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

void ICACHE_FLASH_ATTR
xo1008_net_upload(uint8_t *buf, uint32_t len)
{

}

static void ICACHE_FLASH_ATTR
xo1008_heart(void *parm)
{
    static uint32_t l_tm = 0;
    uint32_t c_tm;

    if(!mesh_network_isconnected()) {
        return;
    }

    c_tm = system_get_time();
    if(c_tm - l_tm > XO1007_HEARTBEART_TIME * 1000) {
        xo1007_heartbeat();
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

