
#ifndef _HONYAR_CONFIG_H_
#define _HONYAR_CONFIG_H_

#include "honyar_types.h"

/** 
 * 配置的字符串值的最大长度
 */
#define CONFIG_VALUE_MAX_LEN (128)


/** 
 * 配置项数据类型 
 */
typedef enum
{
	/** 字符串类型 */
	DL_CFG_ITEM_TYPE_STRING,
	
	/** HEX化的字符串 */
	DL_CFG_ITEM_TYPE_HEX_STR,
	
	/** 10进制32位整数 */
	DL_CFG_ITEM_TYPE_DEC32,
	
	/** 10进制16位整数 */
	DL_CFG_ITEM_TYPE_DEC16,
	
	/** 10进制8位整数 */
	DL_CFG_ITEM_TYPE_DEC8,
}DL_CONFIG_ITEM_TYPE_E;

/** 
 * 配置注册参数结构 
 */
typedef struct
{
	/** 配置名称 */
	const char *name;
	
	/** 配置类型 */
	DL_CONFIG_ITEM_TYPE_E type;
	
	/** 配置变量地址 */
	void *pvalue;
	
	/** 配置变量长度 */
	uint32_t value_len;
	
	/** 只读标识 */
	uint32_t read_only_flag;
}DL_CONFIG_ITEM_S;

/** 
 * 单个配置项目在存储时的最大长度 
 */
#define CONFIG_ITEM_LEN		(512)

/** 
 * 配置修改提交
 * 当修改某个配置变量的值后，为保存配置到FLASH中，需要调用该函数提交变更。
 * @param[in]   force 是否强制提交。如果之前没有调用过dl_config_commit_later提交修改，force=0的情况将不实际提交。force=1时强制提交。
 * @retval  0   提交成功
 * @retval  <0  提交失败 
 */
int32_t dl_config_commit(uint32_t force);

/** 
 * 配置修改延后提交申请
 * 当修改某个配置变量的值后，为保存配置到FLASH中，需要调用该函数提交变更。提交后，在SDK内核定时器中会检查延时提交申请，如果有申请就会执行提交。
 * @retval  0   提交成功
 * @retval  <0  提交失败 
 */
int32_t dl_config_commit_later(void);

/** 
 * 配置字符串值修改
 * 修改某个配置项的字符串值。修改的同时会触发配置变量值的变化。如果有值变更，会自动请求延时提交。
 * @param[in]   name 配置名称 
 * @param[in]   value 配置的字符串值
 * @retval  0   修改成功
 * @retval  <0  修改失败 
 */
int32_t dl_config_modify(const char *name, const char *value);

/** 
 * 用户配置项注册
 * 
 * @param[in]   p_cfg 配置参数
 * @retval  0   注册成功
 * @retval  <0  注册失败 
 */
int32_t dl_config_items_register_by_user(DL_CONFIG_ITEM_S *p_cfg);

int32_t dl_config_init(void);

#endif /* _HONYAR_CONFIG_H_ */

