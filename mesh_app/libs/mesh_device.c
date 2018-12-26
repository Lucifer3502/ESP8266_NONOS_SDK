
#include "mesh_device.h"
#include "honyar_common.h"


static uint8_t g_mesh_device_init = 0;
static struct mesh_device_list_type g_node_list;
static struct mesh_device_list_type g_child_list;
static struct mesh_device_mac_type g_parent;

mesh_device_list_type_t *ICACHE_FLASH_ATTR
mesh_device_get_all(void)
{
    return &g_node_list;
}

mesh_device_list_type_t *ICACHE_FLASH_ATTR
mesh_device_get_child(void)
{
    return &g_child_list;
}

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

void ICACHE_FLASH_ATTR
mesh_device_disp_child_list(void)
{
    uint16_t idx = 0;
    uint8_t mac[MAC_ADDR_LEN] = {0};
    if (honyar_wifi_get_macaddr(mac)) {
        hy_error("get mac fail\r\n");
        return;
    }
    if (g_child_list.scale < 1)
        return;

    hy_printf("=====child list info=====\r\n");
    //hy_printf("self: " MACSTR "\r\n", MAC2STR(mac));
    hy_printf("self: " MACSTR "\r\n", MAC2STR(g_child_list.root.mac));

    for (idx = 0; idx < g_child_list.scale - 1; idx ++)
        hy_printf("idx:%d, " MACSTR "\r\n", idx, MAC2STR(g_child_list.list[idx].mac));
    hy_printf("=====child list end======\r\n");
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

void ICACHE_FLASH_ATTR
mesh_device_list_release(void)
{
    if (!g_mesh_device_init)
        return;

    if (g_node_list.list != NULL) {
        os_free(g_node_list.list);
        g_node_list.list = NULL;
    }
    os_memset(&g_node_list, 0, sizeof(g_node_list));
}

void ICACHE_FLASH_ATTR
mesh_device_list_init(void)
{
    uint8_t mac[MAC_ADDR_LEN] = {0};
    honyar_wifi_get_macaddr(mac);

    if (g_mesh_device_init)
        return;
    
    os_memset(&g_node_list, 0, sizeof(g_node_list));
    os_memset(&g_child_list, 0, sizeof(g_child_list));
    os_memcpy(&g_child_list.root, mac, MAC_ADDR_LEN);
    g_child_list.scale = 1;
    
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
    if (!os_memcmp(g_node_list.root.mac, root->mac, ESP_MESH_ADDR_LEN))
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

void ICACHE_FLASH_ATTR
mesh_device_set_parent(struct mesh_device_mac_type *parent)
{
    if (!os_memcmp(g_parent.mac, parent->mac, ESP_MESH_ADDR_LEN)) {
        return;
    }
    hy_info("set parent from:" MACSTR "to parent:" MACSTR "\n",
            MAC2STR(g_parent.mac), MAC2STR(parent->mac));
    os_memcpy(&g_parent, parent, sizeof(*parent));
}

int32_t ICACHE_FLASH_ATTR
mesh_search_device(mesh_device_list_type_t *node_list, const struct mesh_device_mac_type *node)
{
    uint16_t idx = 0;
    uint16_t scale = 0, i = 0;
    struct mesh_device_mac_type *list = NULL;

    if (node_list->scale == 0)
        return -1;
    if (!os_memcmp(node_list->root.mac, node->mac, ESP_MESH_ADDR_LEN))
        return 0;
    if (node_list->list == NULL)
        return -1;

    scale = node_list->scale - 1;
    list = node_list->list;

    for (i = 0; i < scale; i ++) {
        if (!os_memcmp(list->mac, node->mac, ESP_MESH_ADDR_LEN))
            return 0;
        list++;
    }
    return -1;
}

int32_t ICACHE_FLASH_ATTR
mesh_device_add(struct mesh_device_mac_type *nodes, uint8_t node_type)
{
#define MESH_DEV_STEP (10)
    uint16_t idx = 0;
    mesh_device_list_type_t *node_list = NULL;

    if(MESH_NODE_CHILD == node_type ) {
        node_list = mesh_device_get_child();
    } else {
        node_list = mesh_device_get_all();
    }
    
    uint16_t rest = node_list->size + 1 - node_list->scale;

    if (!g_mesh_device_init)
        mesh_device_list_init();

    if(NULL == nodes) {
        return -1;
    }
    if (node_list->size == 0)
        rest = 0;

    if (0 == rest) {
        /*
         * current list is limited
         * we need to re-allocate buffer for mac list
         */
        uint16_t size = node_list->size + rest + MESH_DEV_STEP;
        uint8_t *buf = (uint8_t *)os_zalloc(size * sizeof(*nodes));
        if (!buf) {
            hy_error("mesh add alloc buf fail\n");
            return -1;
        }
        /*
         * move the current list to new buffer
         */
        if (node_list->list && node_list->scale > 1)
            os_memcpy(buf, node_list->list,
                    (node_list->scale - 1) * sizeof(*nodes));
        if (node_list->list)
            os_free(node_list->list);
        node_list->list = (struct mesh_device_mac_type *)buf;
        node_list->size = size;
    }

    if (mesh_search_device(node_list, nodes)) {  // not in list, add it into list
        os_memcpy(node_list->list + node_list->scale - 1,
                nodes, sizeof(*nodes));
        node_list->scale ++;
    }

    return 0;
}

int32_t ICACHE_FLASH_ATTR
mesh_device_del(struct mesh_device_mac_type *nodes, uint16_t count, uint8_t node_type)
{
    uint16_t idx = 0, i = 0;
    mesh_device_list_type_t *list = NULL;
    uint16_t sub_count = 0;
    
    if(MESH_NODE_CHILD == node_type ) {
        list = mesh_device_get_child();
    } else {
        list = mesh_device_get_all();
    }
    
    if (!nodes || count == 0)
        return 0;

    if (!g_mesh_device_init)
        mesh_device_list_init();

    if (list->scale == 0)
        return -1;

    sub_count = list->scale - 1;
    while (idx < count) {
        /*
         * node is not in list, do nothing
         */
        if (!mesh_search_device(&g_node_list, nodes + idx)) {
            idx ++;
            continue;
        }

        /*
         * root will be delete, so current mac list is stale
         */
        if (!os_memcmp(nodes[idx].mac, list->root.mac, ESP_MESH_ADDR_LEN)) {
            mesh_device_list_release();
            return 0;
        }

        /*
         * delete node from mac list
         */
        for (i = 0; i < sub_count; i ++) {
            if (!os_memcmp(nodes[idx].mac, list->list[i].mac, ESP_MESH_ADDR_LEN)) {
                if (sub_count - i  > 1) {
                    //left shift
                    os_memcpy(&list->list[i], &list->list[i + 1],
                            (sub_count - i - 1) * sizeof(*nodes));
                }
                sub_count--;
                list->scale--;
                i--;
                os_memset(&list->list[list->scale], 0, sizeof(*nodes));
                break;
            }
        }
        idx++;
    }
    return 0;
}

static int32_t ICACHE_FLASH_ATTR
mesh_device_flush_all(void)
{
    mesh_device_list_type_t *list = mesh_device_get_all();
    uint32_t i = 0;
    uint32_t ctm = system_get_time();

    if(list->scale < 1) {
        return 0;
    }
    for(i = 0; i < list->scale - 1; i++) {
        if(ctm - list->list[i].active_time > MESH_DEVICE_FLUSH_TIMEOUT) {
            mesh_device_del(&list->list[i], 1, MESH_NODE_ALL);
            hy_info(MACSTR" offline.\r\n", MAC2STR(list->list[i].mac));
        }
    }
    return 0;
}

static int32_t ICACHE_FLASH_ATTR
mesh_device_flush_child(void)
{
    mesh_device_list_type_t *list = mesh_device_get_child();
    uint32_t i = 0;
    uint32_t ctm = system_get_time();

    if(list->scale < 1) {
        return 0;
    }
    for(i = 0; i < list->scale - 1; i++) {
        if(ctm - list->list[i].active_time > MESH_DEVICE_FLUSH_TIMEOUT) {
            mesh_device_del(&list->list[i], 1, MESH_NODE_CHILD);
            hy_info(MACSTR" offline.\r\n", MAC2STR(list->list[i].mac));
        }
    }
    return 0;
}


int32_t ICACHE_FLASH_ATTR
mesh_device_flush(void)
{
    mesh_device_flush_all();
    mesh_device_flush_child();
    return 0;
}

