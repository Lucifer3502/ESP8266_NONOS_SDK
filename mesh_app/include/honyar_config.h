
#ifndef _HONYAR_CONFIG_H_
#define _HONYAR_CONFIG_H_

#include "honyar_types.h"

/** 
 * ���õ��ַ���ֵ����󳤶�
 */
#define CONFIG_VALUE_MAX_LEN (128)


/** 
 * �������������� 
 */
typedef enum
{
	/** �ַ������� */
	DL_CFG_ITEM_TYPE_STRING,
	
	/** HEX�����ַ��� */
	DL_CFG_ITEM_TYPE_HEX_STR,
	
	/** 10����32λ���� */
	DL_CFG_ITEM_TYPE_DEC32,
	
	/** 10����16λ���� */
	DL_CFG_ITEM_TYPE_DEC16,
	
	/** 10����8λ���� */
	DL_CFG_ITEM_TYPE_DEC8,
}DL_CONFIG_ITEM_TYPE_E;

/** 
 * ����ע������ṹ 
 */
typedef struct
{
	/** �������� */
	const char *name;
	
	/** �������� */
	DL_CONFIG_ITEM_TYPE_E type;
	
	/** ���ñ�����ַ */
	void *pvalue;
	
	/** ���ñ������� */
	uint32_t value_len;
	
	/** ֻ����ʶ */
	uint32_t read_only_flag;
}DL_CONFIG_ITEM_S;

/** 
 * ����������Ŀ�ڴ洢ʱ����󳤶� 
 */
#define CONFIG_ITEM_LEN		(512)

/** 
 * �����޸��ύ
 * ���޸�ĳ�����ñ�����ֵ��Ϊ�������õ�FLASH�У���Ҫ���øú����ύ�����
 * @param[in]   force �Ƿ�ǿ���ύ�����֮ǰû�е��ù�dl_config_commit_later�ύ�޸ģ�force=0���������ʵ���ύ��force=1ʱǿ���ύ��
 * @retval  0   �ύ�ɹ�
 * @retval  <0  �ύʧ�� 
 */
int32_t dl_config_commit(uint32_t force);

/** 
 * �����޸��Ӻ��ύ����
 * ���޸�ĳ�����ñ�����ֵ��Ϊ�������õ�FLASH�У���Ҫ���øú����ύ������ύ����SDK�ں˶�ʱ���л�����ʱ�ύ���룬���������ͻ�ִ���ύ��
 * @retval  0   �ύ�ɹ�
 * @retval  <0  �ύʧ�� 
 */
int32_t dl_config_commit_later(void);

/** 
 * �����ַ���ֵ�޸�
 * �޸�ĳ����������ַ���ֵ���޸ĵ�ͬʱ�ᴥ�����ñ���ֵ�ı仯�������ֵ��������Զ�������ʱ�ύ��
 * @param[in]   name �������� 
 * @param[in]   value ���õ��ַ���ֵ
 * @retval  0   �޸ĳɹ�
 * @retval  <0  �޸�ʧ�� 
 */
int32_t dl_config_modify(const char *name, const char *value);

/** 
 * �û�������ע��
 * 
 * @param[in]   p_cfg ���ò���
 * @retval  0   ע��ɹ�
 * @retval  <0  ע��ʧ�� 
 */
int32_t dl_config_items_register_by_user(DL_CONFIG_ITEM_S *p_cfg);

int32_t dl_config_init(void);

#endif /* _HONYAR_CONFIG_H_ */

