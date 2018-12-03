

#include "honyar_config.h"
#include "honyar_common.h"

#define DL_CONFIG_RO_FLAG (0x77557755)
#define DL_CONFIG_RW_FLAG (0x77667766)

#define ERASE_ONCE_SIZE (SPI_FLASH_SEC_SIZE)
#define WRITE_ONCE_SIZE (256)
#define CONFIG_IMAGE_SIZE (ERASE_ONCE_SIZE)

#define DL_FLASH_CONFIG_TXT_ADDR	(0x0007c000U)
#define DL_FLASH_CONFIG_TXTBAK_ADDR (0x0007e000U)

#define CONFIG_MAX_NUM  128


typedef int32_t (*CONFIG_FOR_EACH_FUNC)(const char *pname, const char *pvalue);


const char cfg_head[] = "CFG_HEAD";
const char cfg_tail[] = "CFG_TAIL";

/* 主配置区地址 */
static uint32_t g_config_image_addr = DL_FLASH_CONFIG_TXT_ADDR;

/* 备份配置区地址 */
static uint32_t g_config_bak_image_addr = DL_FLASH_CONFIG_TXTBAK_ADDR;

/* 配置修改地址 */
static uint32_t g_config_write_addr = 0;

/* 配置修改线程互斥锁 */
static uint32_t g_config_lock = 0;

/* 配置提交 */
uint32_t g_config_commit_flag = 0;

/* 配置操作权限等级 0---系统等级，3---用户等级 */
uint32_t g_config_right_level = 0;
uint32_t g_config_clear_flag = 0;
uint32_t g_config_commit_disable = 0;
static DL_CONFIG_ITEM_S g_config_item_reg[CONFIG_MAX_NUM];




static void _dl_config_commit_inter(DL_CONFIG_ITEM_S *pitem, char *pvalue, uint32_t buf_len);

static int32_t dl_config_item(const char *name, const char *value);

static void ICACHE_FLASH_ATTR dl_config_commit_task(void *ppara)
{
    dl_config_commit(0);
}

//查询配置信息
int32_t ICACHE_FLASH_ATTR dl_config_query_item(const char *name, char *pvalue, uint32_t buf_len)
{
	uint32_t i;
	for (i = 0; i < sizeof(g_config_item_reg)/sizeof(g_config_item_reg[0]); i++)
	{
		if (NULL != g_config_item_reg[i].name)
		{
			if (!strcmp(g_config_item_reg[i].name, name))
			{
				_dl_config_commit_inter(&g_config_item_reg[i], pvalue, buf_len);
				return 0;
			}
		}
	}

	return -1;	
}

int32_t ICACHE_FLASH_ATTR dl_config_items_register_by_user(DL_CONFIG_ITEM_S *p_cfg)
{
	if (p_cfg->read_only_flag == 0)
	{
		p_cfg->read_only_flag = DL_CONFIG_RW_FLAG;
	}
	else
	{
		p_cfg->read_only_flag = DL_CONFIG_RO_FLAG;
	}
	
	return dl_config_items_register(p_cfg->name, p_cfg->type, p_cfg->pvalue, p_cfg->value_len, p_cfg->read_only_flag);
}

int32_t ICACHE_FLASH_ATTR dl_config_items_register(const char *pname,
								DL_CONFIG_ITEM_TYPE_E type,
								void *pvalue,
								uint32_t value_len,
								uint32_t read_only_flag)
{
	if (g_config_clear_flag == 0)
	{
		memset(g_config_item_reg, 0, sizeof(g_config_item_reg));
		g_config_clear_flag = 1;
	}

	if (read_only_flag != DL_CONFIG_RO_FLAG && read_only_flag != DL_CONFIG_RW_FLAG)
	{	
		return -1;
	}
	
	uint32_t i;
	for (i = 0; i < sizeof(g_config_item_reg)/sizeof(g_config_item_reg[0]); i++)
	{
		if (NULL == g_config_item_reg[i].name)
		{
			g_config_item_reg[i].name = pname;
			g_config_item_reg[i].value_len = value_len;
			g_config_item_reg[i].pvalue = pvalue;
			g_config_item_reg[i].type = type;
			g_config_item_reg[i].read_only_flag = 0;
			return 0;
		}
	}

	/* 没有足够的空间存放配置项了 */
	return -2;
}

static void ICACHE_FLASH_ATTR _dl_config_commit_inter(DL_CONFIG_ITEM_S *pitem, char *pvalue, uint32_t buf_len)
{
	switch (pitem->type)
	{
	case DL_CFG_ITEM_TYPE_STRING:
		strncpy(pvalue, pitem->pvalue, buf_len - 1);
		break;

	case DL_CFG_ITEM_TYPE_DEC32:
		{	
			uint32_t *pu32temp = (uint32_t *)pitem->pvalue;
			uint32_t u32temp = *pu32temp;
			os_sprintf(pvalue, "%d", u32temp);
		}
		break;

	case DL_CFG_ITEM_TYPE_DEC16:
		{
			uint16_t *pu16temp = (uint16_t *)pitem->pvalue;
			uint32_t u32temp = *pu16temp;
			os_sprintf(pvalue, "%d", u32temp);
		}
		break;

	case DL_CFG_ITEM_TYPE_DEC8:
		{	
			uint8_t *pu8temp = (uint8_t *)pitem->pvalue;
			uint32_t u32temp = *pu8temp;
			os_sprintf(pvalue, "%d", u32temp);
		}
		break;

	case DL_CFG_ITEM_TYPE_HEX_STR:
		{
			if (buf_len >= (pitem->value_len * 2)) {
                hy_byte2hex(pvalue, buf_len, pitem->pvalue, pitem->value_len);
			}
		}
		break;

	default:
		break;
	}
}

int32_t ICACHE_FLASH_ATTR _dl_config_modify_inter(DL_CONFIG_ITEM_S *pitem, const char *name, const char *value)
{
	int32_t ret = -1;
	
	if (!strcmp(pitem->name, name))
	{
		hy_printf(">%s=%s\n", name, value);
		
		ret = 0;

		if (g_config_right_level == 3 && pitem->read_only_flag == DL_CONFIG_RO_FLAG)
		{
			/* 参数被保护，不得修改 */
			return 0;
		}
		
		switch (pitem->type)
		{
		case DL_CFG_ITEM_TYPE_STRING:
			if (strcmp(pitem->pvalue, value))
			{
				strncpy(pitem->pvalue, value, (pitem->value_len - 1));
				g_config_commit_flag = 1;
			}
			break;

		case DL_CFG_ITEM_TYPE_DEC32:
			{
				uint32_t u32temp = atoi(value);
				uint32_t *pu32temp = (uint32_t *)pitem->pvalue;

				if (*pu32temp != u32temp)
				{
					*pu32temp = u32temp;
					g_config_commit_flag = 1;
				}
			}
			break;

		case DL_CFG_ITEM_TYPE_DEC16:
			{
				uint16_t u16temp = atoi(value);
				uint16_t *pu16temp = (uint16_t *)pitem->pvalue;

				if (u16temp != *pu16temp)
				{
					*pu16temp = u16temp;
					g_config_commit_flag = 1;
				}
			}
			break;

		case DL_CFG_ITEM_TYPE_DEC8:
			{	
				uint8_t u8temp = atoi(value); 
				uint8_t *pu8temp = (uint8_t *)pitem->pvalue;

				if (u8temp != *pu8temp)
				{
					*pu8temp = u8temp;
					g_config_commit_flag = 1;
				}
			}
			break;

		case DL_CFG_ITEM_TYPE_HEX_STR:
			{	
				uint8_t *phex_temp = (uint8_t *)honyar_malloc(CONFIG_VALUE_MAX_LEN);
				memset(phex_temp, 0, CONFIG_VALUE_MAX_LEN);

				if(0 == hy_hex2byte(phex_temp, pitem->value_len, (uint8_t *)value, strlen(value))) {
                    memcpy(pitem->pvalue, phex_temp, pitem->value_len);
					g_config_commit_flag = 1;
                }
				
				honyar_free(phex_temp);
			}
			break;

		default:
			break;
		}
	}

	return ret;
}

int32_t ICACHE_FLASH_ATTR dl_config_modify(const char *name, const char *value)
{
	uint32_t i;
	int32_t ret = -1;
	
	for (i = 0; i < sizeof(g_config_item_reg)/sizeof(g_config_item_reg[0]); i++)
	{
		if (g_config_item_reg[i].name == NULL)
		{
			continue;
		}
	
		if (0 == _dl_config_modify_inter(&(g_config_item_reg[i]), name, value))
		{
			ret = 0;
			break;
		}
	}
    
	return ret;
}

static int32_t ICACHE_FLASH_ATTR _dl_config_show_value(const char *pname, const char *pvalue)
{
	hy_printf("%s=%s\n", pname, pvalue);
	return 0;
}

void ICACHE_FLASH_ATTR dl_config_for_each_in_ram(CONFIG_FOR_EACH_FUNC func)
{
	char *pvalue = (char *)honyar_malloc(256);
	uint32_t i;
    uint32_t item_num = sizeof(g_config_item_reg)/sizeof(g_config_item_reg[0]);
	for (i = 0; i < item_num; i++)
	{
		if (NULL != g_config_item_reg[i].name)
		{
			memset(pvalue, 0, 256);
			_dl_config_commit_inter(&g_config_item_reg[i], pvalue, 256);
			func(g_config_item_reg[i].name, pvalue);
		}
	}

	honyar_free(pvalue);
}

int32_t ICACHE_FLASH_ATTR dl_config_commit(uint32_t force)
{
	if (g_config_commit_disable)
	{
		/* 禁止提交 */
		return -1;
	}

	if (force != 0)
	{
		g_config_commit_flag = 1;
	}

	if (g_config_commit_flag == 0)
	{
		return 0;
	}

	honyar_flash_erase(g_config_bak_image_addr, CONFIG_IMAGE_SIZE);

	g_config_write_addr = g_config_bak_image_addr + 12;

	dl_config_for_each_in_ram(dl_config_item);

	honyar_flash_write(g_config_write_addr, (uint8_t *)cfg_tail, os_strlen(cfg_tail) + 1);
	honyar_flash_write(g_config_write_addr + os_strlen(cfg_tail) + 1, (uint8_t *)"OK", os_strlen("OK") + 1);
	honyar_flash_write(g_config_bak_image_addr, (uint8_t *)cfg_head, os_strlen(cfg_head) + 1);	
	honyar_flash_write(g_config_bak_image_addr + os_strlen(cfg_head) + 1, (uint8_t *)"OK", os_strlen("OK") + 1);

	honyar_flash_erase(g_config_image_addr, CONFIG_IMAGE_SIZE);

	g_config_write_addr = 0;
	uint32_t tmp = g_config_image_addr;
	g_config_image_addr = g_config_bak_image_addr;
	g_config_bak_image_addr = tmp;
    
    g_config_commit_flag = 0;
	return 0;
}

int32_t ICACHE_FLASH_ATTR dl_config_commit_patch(const char *patch, uint32_t patch_len)
{
	if (g_config_commit_disable)
	{
		/* 禁止提交 */
		return -1;
	}

	honyar_flash_erase(g_config_bak_image_addr, CONFIG_IMAGE_SIZE);

	g_config_write_addr = g_config_bak_image_addr + 12;

	dl_config_for_each_in_ram(dl_config_item);

	honyar_flash_write(g_config_write_addr, (uint8_t *)patch, patch_len);
	g_config_write_addr += patch_len;
	honyar_flash_write(g_config_write_addr, (uint8_t *)cfg_tail, os_strlen(cfg_tail) + 1);
	honyar_flash_write(g_config_write_addr + os_strlen(cfg_tail) + 1, (uint8_t *)"OK", os_strlen("OK") + 1);
	honyar_flash_write(g_config_bak_image_addr, (uint8_t *)cfg_head, os_strlen(cfg_head) + 1);	
	honyar_flash_write(g_config_bak_image_addr + os_strlen(cfg_head) + 1, (uint8_t *)"OK", os_strlen("OK") + 1);

	honyar_flash_erase(g_config_image_addr, CONFIG_IMAGE_SIZE);

	g_config_write_addr = 0;
	uint32_t tmp = g_config_image_addr;
	g_config_image_addr = g_config_bak_image_addr;
	g_config_bak_image_addr = tmp;
    
    g_config_commit_flag = 0;
	return 0;
}

int32_t ICACHE_FLASH_ATTR dl_config_commit_later(void)
{
	if (g_config_commit_disable)
	{
		/* 禁止提交 */
		return -1;
	}

    g_config_commit_flag = 1;
    return 0;
}

int32_t ICACHE_FLASH_ATTR dl_config_commit_clear(void)
{
    g_config_commit_flag = 0;
    return 0;
}

int32_t ICACHE_FLASH_ATTR dl_config_commit_disable(int32_t disable)
{
    g_config_commit_disable = disable;
    return 0;
}

void ICACHE_FLASH_ATTR dl_config_for_each_in_flash(CONFIG_FOR_EACH_FUNC func)
{
	char *cfg_item = (char *)honyar_malloc(CONFIG_ITEM_LEN);
	memset(cfg_item, 0, CONFIG_ITEM_LEN);

	honyar_flash_read(g_config_image_addr, (uint8_t *)cfg_item, CONFIG_ITEM_LEN);		

	if (os_memcmp(cfg_item, cfg_head, os_strlen(cfg_head) + 1))
	{
		uint32_t tmpaddr = g_config_image_addr;
        g_config_image_addr = g_config_bak_image_addr;
        g_config_bak_image_addr = tmpaddr;
	}

	honyar_flash_read(g_config_image_addr, (uint8_t *)cfg_item, CONFIG_ITEM_LEN);
	
	if (os_memcmp(cfg_item, cfg_head, os_strlen(cfg_head) + 1))//配置区未被写过
	{
        /* 两个配置区的配置都无效，将当前的默认配置设置为配置 */
		honyar_free(cfg_item);
		hy_printf("--No config data!--!\n");
		return;
	}	

	/* 配置从FLASH中获取的值 */
	uint32_t index = g_config_image_addr;
	
	while (index < (g_config_image_addr + CONFIG_IMAGE_SIZE - CONFIG_ITEM_LEN))
	{	
		honyar_flash_read(index, (uint8_t *)cfg_item, CONFIG_ITEM_LEN);

		int i = 0;
		char *item_name = cfg_item;
		char *item_value = NULL;
		
		i += os_strlen(item_name) + 1;

		if (!strcmp(item_name, cfg_tail))
		{
			/* 已经到结尾了 */
			hy_printf("--No more config data!--!\n");
			break;
		}

		item_value = &cfg_item[i];

		i += os_strlen(item_value) + 1;

		index += i;

		func(item_name, item_value);
	}	
	
	honyar_free(cfg_item);
}


int32_t ICACHE_FLASH_ATTR dl_config_init(void)
{
	static uint32_t config_inited = 0;

	if (config_inited == 1) {
		return -1;
	}
	config_inited = 1;
	
	g_config_right_level = 0; /* 系统等级 */
	dl_config_for_each_in_flash(dl_config_modify);
	g_config_commit_flag = 0;
	g_config_right_level = 3;
	honyar_add_task(dl_config_commit_task, NULL, 0);
    
	return 0;
}

void ICACHE_FLASH_ATTR dl_config_show_all_items_in_flash(void)
{
	dl_config_for_each_in_flash(_dl_config_show_value);
}

void ICACHE_FLASH_ATTR dl_config_show_all_items_in_ram(void)
{
	dl_config_for_each_in_ram(_dl_config_show_value);
}

//used to save config from ram to rom
static int32_t ICACHE_FLASH_ATTR dl_config_item(const char *name, const char *value)
{
	if (g_config_write_addr == 0) {
		return -1;
	}
	hy_printf("<%s=%s\n", name, value);
	honyar_flash_write(g_config_write_addr, (uint8_t *)name, os_strlen(name) + 1);
	g_config_write_addr += os_strlen(name) + 1;
	honyar_flash_write(g_config_write_addr, (uint8_t *)value, os_strlen(value) + 1);
	g_config_write_addr += os_strlen(value) + 1;
	return 0;
}



