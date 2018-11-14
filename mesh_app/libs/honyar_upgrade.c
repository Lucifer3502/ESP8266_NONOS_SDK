
#include "honyar_common.h"


static T_OTA_BIN_S *_g_pota_head = NULL;
static uint32_t _g_update_total_len = 0;
static uint32_t _g_update_cfg_total_len = 0;
static char *_gp_update_cfg_patch = NULL;
static uint32_t _g_update_need_len = 0;
static uint32_t _g_update_start_recv_len = 0;

uint32_t get_max_image_size(void)
{
    return MAX_IMAGE_LEN;
}

uint32_t get_upgrade_bin_addr(void)
{
    uint8_t user_bin = system_upgrade_userbin_check();
    if(UPGRADE_FW_BIN1 == user_bin) {
        return USER2_ADDR;
    } else if(UPGRADE_FW_BIN2 == user_bin) {
        return USER1_ADDR;
    } else {
        return 0;
    }
}

LOCAL int32_t check_update_user_bin(T_OTA_BIN_S *pota_head, uint32_t addr)
{
    if(USER2_ADDR == addr) {
        _g_update_start_recv_len = pota_head->user2_bin_offset;
        return 0;
    }
    else if(USER1_ADDR == addr) {
        _g_update_start_recv_len = pota_head->user1_bin_offset;
        return 0;
    }

    return -1;
}

static void get_update_bin_md5(T_OTA_BIN_S *pota_head, uint8_t md5[16])
{
    uint8_t user_bin = system_upgrade_userbin_check();
    if(UPGRADE_FW_BIN1 == user_bin) {
        memcpy(md5, pota_head->user2_md5, MD5_LEN);
    } else if(UPGRADE_FW_BIN2 == user_bin)
    {
        memcpy(md5, pota_head->user1_md5, MD5_LEN);
    }
}


static int32_t update_download_process(uint8_t *buf, uint32_t buf_len);

static int32_t update_download_start(uint8_t *buf, uint32_t buf_len, char *pmodel)
{
    uint32_t update_addr = get_upgrade_bin_addr();
    uint32_t write_len = 0;
    if(!update_addr) {
        hy_error("system error, cannt update\r\n");
        return -2;
    }
    
    if(buf_len < sizeof(T_OTA_BIN_S)){
        hy_error("Buf Len less than OTA_HEAD_LEN! buf_len = %d.\r\n", buf_len);
        return -2;
    }
        
    if (_g_pota_head == NULL) {
        _g_pota_head = (T_OTA_BIN_S *)honyar_malloc(sizeof(T_OTA_BIN_S));
    }

    memcpy(_g_pota_head, buf, sizeof(T_OTA_BIN_S));
    if(check_update_user_bin(_g_pota_head, update_addr)){
        hy_error("UPDATE BIN SELECT FAILED. \r\n");
        honyar_free(_g_pota_head);
        _g_pota_head = NULL;
        return -2;
    }

    _g_pota_head->model[15] = 0;

    if (pmodel && strcmp(_g_pota_head->model, pmodel)) {
            /* 型号不匹配 */
        hy_error("MODEL INVALID!\n");
        return -1;
    }

    if(_g_pota_head->user1_bin_len != _g_pota_head->user2_bin_len){
        //升级文件错误，user1和user2 bin大小不一致
        hy_error("BIN LEN INVALID!\n");
        return -2;
    }
    if (_g_pota_head->user1_bin_len > MAX_IMAGE_LEN  || _g_pota_head->user2_bin_len > MAX_IMAGE_LEN){
        /* 升级文件过大，不予升级 */
        hy_error("BIN LEN TOO BIG!\n");
        return -2;
    }

    if (_g_pota_head->cfg_len > 0){
        if (_gp_update_cfg_patch){
            honyar_free(_gp_update_cfg_patch);
        }
        _gp_update_cfg_patch = (char *)honyar_malloc(_g_pota_head->cfg_len);
    }

    _g_update_need_len = _g_pota_head->user2_bin_len + _g_pota_head->user2_bin_offset;
    _g_update_total_len = buf_len;
    _g_update_cfg_total_len = 0;

    //程序不为空
    if(_g_pota_head->user1_bin_len)
    {
        hy_debug("erase addr 0x%x\r\n", update_addr);
        honyar_flash_erase(update_addr, MAX_IMAGE_LEN);

        if(buf_len > _g_update_start_recv_len){
            write_len = buf_len - _g_update_start_recv_len;
            honyar_flash_write(update_addr, buf + _g_update_start_recv_len, write_len);
        }
    } else if(0 == _g_pota_head->cfg_len) {
        //空文件
        return -2;
    } else {
        //程序为空
        if(buf_len >= _g_update_need_len){
            //接收到的文件包含配置文件
            _g_update_total_len = _g_update_need_len;
            return update_download_process(buf + _g_update_need_len, buf_len - _g_update_need_len);
        }
    }
    
    return 0;
}

static int32_t update_download_process(uint8_t *buf, uint32_t buf_len)
{
    uint32_t update_addr = get_upgrade_bin_addr();
    if(!update_addr)
    {
        hy_error("system error, cannt update\r\n");
        return -2;
    }

    uint32_t bin_size = _g_pota_head->user1_bin_len;
    uint32_t recv_bin_len = 0;
    uint32_t binadd_len = buf_len;
    uint32_t cfgadd_len = 0;
    if(_g_update_total_len + buf_len <= _g_update_start_recv_len)
    {
        _g_update_total_len += buf_len;
        return 0;
    }
    else if(_g_update_total_len < _g_update_start_recv_len)
    {
        binadd_len = _g_update_total_len + buf_len - _g_update_start_recv_len;
        honyar_flash_write(update_addr + recv_bin_len, &buf[_g_update_start_recv_len - _g_update_total_len], binadd_len);
        _g_update_total_len += buf_len;
        recv_bin_len = binadd_len;
    }
    else
    {
        recv_bin_len = _g_update_total_len - _g_update_start_recv_len;
        uint32_t buf_offset = 0;
        if(_g_update_total_len > _g_update_need_len)
        {
            recv_bin_len = bin_size;
            binadd_len = 0;
            cfgadd_len = buf_len;
            buf_offset = 0;
            //dl_printf("%d--%d--%d--%d--%d\r\n",_g_update_need_len, _g_update_total_len, cfgadd_len, binadd_len, _g_update_cfg_total_len);
        }
        else if (_g_update_need_len <  _g_update_total_len + buf_len)
        {
            if(recv_bin_len < bin_size)
            {
                binadd_len = _g_update_need_len - _g_update_total_len;
            }
            else
            {
                recv_bin_len = bin_size;
                binadd_len = 0;
            }

            buf_offset = _g_update_need_len - _g_update_total_len;
            cfgadd_len = _g_update_total_len + buf_len - _g_update_need_len; 
        }
        else
        {
            cfgadd_len = 0;
            if(recv_bin_len >= bin_size)
            {
                recv_bin_len = bin_size;
                binadd_len = 0;
            }
            else
            {
                binadd_len = buf_len;
            }
        }
        
        if(cfgadd_len && _g_pota_head->cfg_len)
        {
            //dl_printf("%d--%d--%d--%d--%d\r\n",_g_update_need_len, _g_update_total_len, cfgadd_len, binadd_len, _g_update_cfg_total_len);
            if ((_g_update_cfg_total_len + cfgadd_len) > _g_pota_head->cfg_len)
            {
                cfgadd_len = _g_pota_head->cfg_len - _g_update_cfg_total_len;
            }
            memcpy(&_gp_update_cfg_patch[_g_update_cfg_total_len], &buf[buf_offset], cfgadd_len);
            _g_update_cfg_total_len += cfgadd_len;
        }

        if (binadd_len > 0)
        {
            honyar_flash_write(update_addr + recv_bin_len, buf, binadd_len);
            
            recv_bin_len += binadd_len;
        }
        _g_update_total_len += buf_len;
        
        
    }

    hy_printf("RECV: %d/%d -- %d/%d\n", recv_bin_len, bin_size, _g_update_cfg_total_len, _g_pota_head->cfg_len);
    uint8_t percent = (uint8_t)((recv_bin_len + _g_update_cfg_total_len) * 100 / (bin_size + _g_pota_head->cfg_len));
    //_g_update_check_state = percent;

    if ((recv_bin_len >= bin_size) && (_g_update_cfg_total_len >= _g_pota_head->cfg_len))
    {
        honyar_msleep(10);
        
        md5_context_t handle;
        uint32_t read_len = 0;
        uint8_t md5_ret[MD5_LEN] = {0};
        uint8_t md5_cmp[MD5_LEN] = {0};
        get_update_bin_md5(_g_pota_head, md5_cmp);
        honyar_md5_start(&handle);

        uint8_t *read_buf = (uint8_t *)honyar_malloc(1024);
        while(read_len < bin_size)
        {
            if(bin_size > read_len + 1024)
            {
                honyar_flash_read(read_len + update_addr, read_buf, 1024);
                honyar_md5_update(&handle, read_buf, 1024);
                read_len += 1024;
            }
            else
            {
                honyar_flash_read(read_len + update_addr, read_buf, bin_size - read_len);
                honyar_md5_update(&handle, read_buf, bin_size - read_len);
                read_len = bin_size;
            }
        }
        honyar_free(read_buf);
        honyar_md5_finish(&handle, md5_ret);
        if (memcmp(md5_cmp, md5_ret, MD5_LEN))
        {
            hy_error("BIN MD5 invalid!\n");
            hex_printf("_g_pota_head->md5:", md5_cmp, MD5_LEN);
            hex_printf("md5_ret:", md5_ret, MD5_LEN);
            if(_gp_update_cfg_patch)
            {
                honyar_free(_gp_update_cfg_patch);
                _gp_update_cfg_patch = NULL;
            }
            honyar_free(_g_pota_head);
            _g_pota_head = NULL;
            return -2;
        }
        if(_gp_update_cfg_patch)
        {
            honyar_md5_start(&handle);
            honyar_md5_update(&handle, (uint8_t *)_gp_update_cfg_patch, _g_pota_head->cfg_len);
            honyar_md5_finish(&handle, md5_ret);
            if (memcmp(_g_pota_head->cfg_md5, md5_ret, MD5_LEN)){
                hy_error("CFG MD5 invalid!\n");
                honyar_free(_gp_update_cfg_patch);
                _gp_update_cfg_patch = NULL;
                honyar_free(_g_pota_head);
                _g_pota_head = NULL;
                return -2;
            }
        }
        
        if(_gp_update_cfg_patch)
        {
            //dl_printf("Add patch config.\r\n");
            //dl_config_commit_patch(_gp_update_cfg_patch, _g_pota_head->cfg_len);
            honyar_free(_gp_update_cfg_patch);
            _gp_update_cfg_patch = NULL;
            //dl_config_commit_disable(1);
        } else {
            hy_info("No patch config.\r\n");
        }
        honyar_free(_g_pota_head);
        _g_pota_head = NULL;

        return 1;
    }
    
    return 0;
}



/*
    return :
    0: update in process;
    <0: update error;
    1: update done;
*/
int32_t ICACHE_FLASH_ATTR hy_update_download(uint8_t *buf, uint32_t buf_len, uint8_t *pmodel)
{

    if (_g_update_total_len == 0)
    {   
        return update_download_start(buf, buf_len, pmodel);  
    }
    else
    {
        return update_download_process(buf, buf_len);
    }
}


