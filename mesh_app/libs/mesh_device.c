
#include "mesh_device.h"
#include "honyar_common.h"


static uint8_t g_mesh_device_init = 0;
static struct mesh_device_list_type g_node_list;

void ICACHE_FLASH_ATTR
mesh_device_disp_mac_list(void)
{
    uint16_t idx = 0;
    uint8_t mac[MAC_ADDR_LEN] = {0};
    if (honyar_wifi_get_macaddr(mac)) {
        hy_error("get mac fail\r\n");
        return;
    }
    if (g_node_list.scale < 1)
        return;

    hy_printf("=====mac list info=====\r\n");
    hy_printf("self: " MACSTR "\r\n", MAC2STR(mac));
    hy_printf("root: " MACSTR "\r\n", MAC2STR(g_node_list.root.mac));

    for (idx = 0; idx < g_node_list.scale - 1; idx ++)
        hy_printf("idx:%d, " MACSTR "\r\n", idx, MAC2STR(g_node_list.list[idx].mac));
    hy_printf("=====mac list end======\r\n");
}

int32_t ICACHE_FLASH_ATTR
mesh_device_get_mac_list(const struct mesh_device_mac_type **list,
                         uint16_t *count)
{
    if (!g_mesh_device_init) {
        hy_error("please init mesh device list firstly\n");
        return -1;
    }

    if (!list || !count)
        return -1;

    if (g_node_list.scale < 2) {
        *list = NULL;
        *count = 0;
        return 0;
    }

    *list = g_node_list.list;
    *count = g_node_list.scale - 1;
    return 0;
}

int32_t ICACHE_FLASH_ATTR
mesh_device_get_root(const struct mesh_device_mac_type **root)
{
    if (!g_mesh_device_init) {
        hy_error("please init mesh device list firstly\n");
        return -1;
    }

    if (g_node_list.scale == 0) {
        hy_info("no mac in current mac list\n");
        return -1;
    }

    if (!root)
        return -1;

    *root = &g_node_list.root;

    return 0;
}

void ICACHE_FLASH_ATTR mesh_device_list_release(void)
{
    if (!g_mesh_device_init)
        return;

    if (g_node_list.list != NULL) {
        os_free(g_node_list.list);
        g_node_list.list = NULL;
    }
    os_memset(&g_node_list, 0, sizeof(g_node_list));
}

void ICACHE_FLASH_ATTR mesh_device_list_init(void)
{
    if (g_mesh_device_init)
        return;
    
    os_memset(&g_node_list, 0, sizeof(g_node_list));
    g_mesh_device_init = 1;
}

void ICACHE_FLASH_ATTR
mesh_device_set_root(struct mesh_device_mac_type *root)
{
    if (!g_mesh_device_init)
        mesh_device_list_init();
    /*
     * the first time to set root
     */
    if (g_node_list.scale == 0) {
        hy_info("new root:" MACSTR "\n", MAC2STR((uint8_t *)root));
        os_memcpy(&g_node_list.root, root, sizeof(*root));
        g_node_list.scale = 1;
        return;
    }
    
    /*
     * root device is the same to the current node,
     * we don't need to modify anything
     */
    if (!os_memcmp(&g_node_list.root, root, sizeof(*root)))
        return;

    /*
     * switch root device, so the mac address list is stale
     * we need to free the stale the mac list
     */
    hy_info("switch root:" MACSTR "to root:" MACSTR "\n",
            MAC2STR((uint8_t *)&g_node_list.root), MAC2STR((uint8_t *)root));
    mesh_device_list_release();
    os_memcpy(&g_node_list.root, root, sizeof(*root));
    g_node_list.scale = 1;
}

int32_t ICACHE_FLASH_ATTR
mesh_search_device(const struct mesh_device_mac_type *node)
{
    uint16_t idx = 0;
    uint16_t scale = 0, i = 0;
    struct mesh_device_mac_type *list = NULL;

    if (g_node_list.scale == 0)
        return -1;
    if (!os_memcmp(&g_node_list.root, node, sizeof(*node)))
        return 0;
    if (g_node_list.list == NULL)
        return -1;

    scale = g_node_list.scale - 1;
    list = g_node_list.list;

    for (i = 0; i < scale; i ++) {
        if (!os_memcmp(list, node, sizeof(*node)))
            return 0;
        list ++;
    }
    return -1;
}

int32_t ICACHE_FLASH_ATTR
mesh_device_add(struct mesh_device_mac_type *nodes)
{
#define MESH_DEV_STEP (10)
    uint16_t idx = 0;
    uint16_t rest = g_node_list.size + 1 - g_node_list.scale;

    if (!g_mesh_device_init)
        mesh_device_list_init();

    if(NULL == nodes) {
        return -1;
    }
    if (g_node_list.size == 0)
        rest = 0;

    if (0 == rest) {
        /*
         * current list is limited
         * we need to re-allocate buffer for mac list
         */
        uint16_t size = g_node_list.size + rest + MESH_DEV_STEP;
        uint8_t *buf = (uint8_t *)os_zalloc(size * sizeof(*nodes));
        if (!buf) {
            hy_error("mesh add alloc buf fail\n");
            return -1;
        }
        /*
         * move the current list to new buffer
         */
        if (g_node_list.list && g_node_list.scale > 1)
            os_memcpy(buf, g_node_list.list,
                    (g_node_list.scale - 1) * sizeof(*nodes));
        if (g_node_list.list)
            os_free(g_node_list.list);
        g_node_list.list = (struct mesh_device_mac_type *)buf;
        g_node_list.size = size;
    }

    if (mesh_search_device(nodes)) {  // not in list, add it into list
        os_memcpy(g_node_list.list + g_node_list.scale - 1,
                nodes, sizeof(*nodes));
        g_node_list.scale ++;
    }

    return 0;
}

int32_t ICACHE_FLASH_ATTR
mesh_device_del(struct mesh_device_mac_type *nodes, uint16_t count)
{
    uint16_t idx = 0, i = 0;
    uint16_t sub_count = g_node_list.scale - 1;

    if (!nodes || count == 0)
        return 0;

    if (!g_mesh_device_init)
        mesh_device_list_init();

    if (g_node_list.scale == 0)
        return -1;

    while (idx < count) {
        /*
         * node is not in list, do nothing
         */
        if (!mesh_search_device(nodes + idx)) {
            idx ++;
            continue;
        }

        /*
         * root will be delete, so current mac list is stale
         */
        if (!os_memcmp(nodes + idx, &g_node_list.root, sizeof(*nodes))) {
            mesh_device_list_release();
            return 0;
        }

        /*
         * delete node from mac list
         */
        for (i = 0; i < sub_count; i ++) {
            if (!os_memcmp(nodes + idx, &g_node_list.list[i], sizeof(*nodes))) {
                if (sub_count - i  > 1)
                    os_memcpy(&g_node_list.list[i], &g_node_list.list[i + 1],
                            (sub_count - i - 1) * sizeof(*nodes));
                sub_count --;
                g_node_list.scale --;
                i --;
                os_memset(&g_node_list.list[g_node_list.scale], 0, sizeof(*nodes));
                break;
            }
        }
        idx ++;
    }
    return 0;
}

