
#ifndef _HONYAR_UPGRADE_H_
#define _HONYAR_UPGRADE_H_

#include "c_types.h"

/* FLASH ������ */
#define USER1_ADDR (0x00001000U)
#define USER2_ADDR (0x00081000U)
#define MAX_IMAGE_LEN (1024UL * 400)


/** �����ļ���ʽ */
typedef struct t_ota_bin_s
{
    uint8_t model[16];
	uint8_t hw_version[16];
	uint8_t sw_version[16]; /* ����汾 */
	uint32_t utc_time;

	uint32_t user1_bin_offset;   //bin ��ʼλ��
	uint32_t user1_bin_len;
	uint8_t user1_md5[16]; /* MD5ֵ */
	
	uint32_t user2_bin_offset;
	uint32_t user2_bin_len;
	uint8_t user2_md5[16];

    uint32_t cfg_len;
    uint8_t cfg_md5[16];
    
    uint32_t reserve[8];
}T_OTA_BIN_S;

int32_t try_upgrading_lock(void);

int32_t upgrading_unlock(void);

void wait_upgrade_reboot(void);

void hy_update_init(void);

#endif

