
#include "at_app.h"
#include "honyar_common.h"

static int32_t ICACHE_FLASH_ATTR
self_mac_check(uint8_t *addr)
{
    uint8_t mac[MAC_ADDR_LEN] = {0};
    uint8_t macstr1[MAC_ADDR_LEN * 3] = {0};
    uint8_t macstr2[MAC_ADDR_LEN * 3] = {0};
    honyar_wifi_get_macaddr(mac);
    os_sprintf(macstr1, MACSTR, MAC2STR(mac));
    os_sprintf(macstr2, MACSTR2, MAC2STR(mac));
    if(os_strcmp(addr, macstr1) && os_strcmp(addr, macstr2)) {
        hy_info("mac check failed\r\n");
        return -1;
    }
    
    return 0;
}

static int32_t ICACHE_FLASH_ATTR
at_reboot(uint32_t argc, uint8_t *argv[])
{
    honyar_sys_reboot(0);
    return 0;
}

static int32_t ICACHE_FLASH_ATTR
at_upgrade(uint32_t argc, uint8_t *argv[])
{
    if(argc < 3) {
        hy_printf("usage:\r\n");
        hy_printf("AT#UPGRADE [OPTION1] [OPTION2]\r\n");
        hy_printf(" [OPTION1]: The Device MAC\r\n");
        hy_printf(" [OPTION2]: download url.\r\n");
        return -1;
    }

    if(self_mac_check(argv[1])) {
        return -1;
    }
    
    return http_upgrade_init2(argv[2]);
}

static int32_t ICACHE_FLASH_ATTR
at_scan(uint32_t argc, uint8_t *argv[])
{
    uint8_t mac[MAC_ADDR_LEN] = {0};
    honyar_wifi_get_macaddr(mac);
    AT_PRINTF(MACSTR"\r\n", MAC2STR(mac));
    return 0;
}

static int32_t ICACHE_FLASH_ATTR
at_write_config(uint32_t argc, uint8_t *argv[])
{
    if(argc < 4) {
        hy_printf("usage:\r\n");
        hy_printf("AT#WRCONF [OPTION1] [OPTION2] [OPTION3]\r\n");
        hy_printf(" [OPTION1]: The Device MAC\r\n");
        hy_printf(" [OPTION2]: The config name.\r\n");
        hy_printf(" [OPTION3]: The config value.\r\n");
        return -1;
    }

    if(self_mac_check(argv[1])) {
        return -1;
    }
    if(dl_config_modify(argv[2], argv[3])) {
        return -1;
    }
    return 0;
}

static int32_t ICACHE_FLASH_ATTR
at_read_config(uint32_t argc, uint8_t *argv[])
{
    uint8_t value[CONFIG_VALUE_MAX_LEN] = {0};
    if(argc < 3) {
        hy_printf("usage:\r\n");
        hy_printf("AT#WRCONF [OPTION1] [OPTION2]\r\n");
        hy_printf(" [OPTION1]: The Device MAC\r\n");
        hy_printf(" [OPTION2]: The config name.\r\n");
        return -1;
    }

    if(self_mac_check(argv[1])) {
        return -1;
    }

    if(dl_config_query_item(argv[2], value, CONFIG_VALUE_MAX_LEN)) {
        return -1;
    }
    AT_PRINTF("%s\r\n", value);
    return 0;
}

static int32_t ICACHE_FLASH_ATTR
at_mesh_topology_show(uint32_t argc, uint8_t *argv[])
{
    mesh_device_disp_mac_list();
    return 0;
}

static int32_t ICACHE_FLASH_ATTR
at_app_output(uint8_t *buf, uint32_t buf_len)
{
    return at_udp_send(buf, buf_len);
}

static void ICACHE_FLASH_ATTR
at_app_task(void *args)
{
    if(STATION_GOT_IP == wifi_station_get_connect_status()) {
        at_output_regist(at_app_output);
        at_udp_init();
        honyar_del_task(at_app_task);
    }
}

static at_table_t at_tables[] = {
    {"REBOOT", at_reboot},
    {"UPGRADE", at_upgrade},
    {"SCAN", at_scan},
    {"WRCONF", at_write_config},
    {"RDCONF", at_read_config},
    {"MESHTOP", at_read_config},
};

void ICACHE_FLASH_ATTR
at_app_init(void)
{
    at_table_init();
    at_table_regist(at_tables, HONYAR_ARRAY_SIZE(at_tables));
    honyar_add_task(at_app_task, NULL, 1000 / TASK_CYCLE_TM_MS);
}

