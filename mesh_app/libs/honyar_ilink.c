
#include "honyar_common.h"

#define WIFI_SCAN_TIMEOUT  10000
#define iLinkConnectVer     "V1.1.0"
            //0����ȷ����
            //ʹ���޼����ڹ�˾��·��������ô�޷�ʹ��һ�����ã�
            
            //1�����Ż���
            //δ�����Ż������ȫ������frameNo͸���Ĺ���֧�֣����������ӷ��㣻����stepNo���尴��˳��ţ�ֻ���ߴ����￪ʼ���ܹ����ݳ��ȡ�֡��ţ�
            //δ�����Ż���֧����������
            //�����Ϊ�����ݻ���ķ�ʽ���������ã���ʽ�յ����Ͽɷ�������
            //δ�Ż�����������˳����룬����Ч�ʵ�
            //���޴���SNУ���ѯ�������Ƿ���ƥ����о͵���֧��
            
            //2.BUG����
            //bugδ�����ֻ������Ĵ���ʽ��֧������WDS����Ҫ�ٴ���֤
            //bug:iLinkConnectVer=V1.0.1
            //m:239.33.113.250
            //101
            //m:239.33.113.250
            //too long time...


            
            
            //3��������
            //��δ֧�֣�����ʱ����Ҫ���ⷢ�����ʰ�;
            //��δ֧�֣�֧��APģʽ��ʱ����
            //��δ֧�֣�һ��������Ϊ������һ����������
            //δʵ�ֵĹ��ܣ���������ģʽ�������������õ�
            //��δ֧�֣������ļ�FLASH�б���Ҳ����һ�ݱ�֤���ݵ������ԣ�����ʧ��ʵ��һ���ű��������Զ���Ӳ�����ͬʱ�Զ��Ի�������Ϊ�ļ���������һ�������ļ���
            
    
            //V1.0.1---����֤
            //�Ż������յ�ͬ��ͷ�󣬽��з���Դ��ַ��������֤����Դͷ��ͬһ��
            //��ʵ�֣���ֱ�ӽ����������ü�����Ҫ��������
            //��ʵ�֣����Ӱ汾�Ŵ�ӡ
            

            //V1.0.0
            //�Ż���֡�����ʱWIFI_WAIT_FRAME_TIMEOUT_TIME=300ms
            //�Ż�����ʱʱ��15S��LastTimeMs=NowTimeMs+10000;//��Ȼ��Ȼ���ǳ�ʱ��
            //�Ż�����ʱ��+֡������Ȳ���ͬ��ͷ���ҵ�ͬ��ͷ������Ƶ�ʺͷ����ߣ����ǳ�ʱ��־����ʱ�����л�Ƶ�ʣ�
            //��֧�֣����ݷ�����Ҫ�����Ƿ���ܱ�־����Ҫ�����Ƿ���ҪУ������SN�Ĺ��ܣ�
            //����������ʹ��iwpriv wifi0 setCountry CN��������12\13Ƶ��
            //���Ż���������ͬ��ͷ���������Ķ��ӳٴ�������յ�ͬ��ͷһ���ֽ�,���ӳ��л�Ƶ��
            //���Ż���ʹ��֡��������������������������������Զ�������ķ�ʽ���������������0xff����0x00���������ݣ����벢�����ۼӣ��ۼ������ˣ��������֤һ��������
            //���������ݻ���Ҫ��ʱ��գ����ܶ����ݣ����һֱ������������Ƶ��ʱ�����

#define RESP_IP_LEN         (4)
#define RESP_RAND_LEN       (2)
#define RESP_PORT_HI_BYTE   (200)
#define RESP_PORT_LO_BYTE   (87)
#define SMART_RESP_RANDOM_XML_LEN (512)
#define RESP_INFO_DISABLE   (0)
#define RESP_INFO_ENABLE    (1)

//�������ݽṹ
I_LINK_CONNECT_MON_BASE_STR g_MonBaseStr;
//wifi���ü������ݽṹ��
I_LINK_CONNECT_STA_SETTING_MULTICAST_STR g_StaMulticastStr;
//WIFI��������֡
I_LINK_CONNECT_WIFI_FRAME_HEAD_SHORT_STR g_WifiFrameHeadStr;
/* һ�����óɹ���ļ�ʱ */
LOCAL uint32_t _g_smart_over_cnt = 0;
#define WIFI_SWITCH_FREQ_TIME 150 // 2000ms
#define WIFI_WAIT_TIMEOUT_TIME  35000//15*1000ms�ܳ�ʱʱ��
#define WIFI_WAIT_FRAME_TIMEOUT_TIME    1500//15*1000ms֡�䳬ʱ
#define WIFI_MON_DATA_SELECT_TIMOUT_US  80000//50ms
uint32_t g_llLastSyncReceiveTime;//������һ�����ݵ�ʱ��
uint32_t g_llLastReceiveTime;//������һ�����ݵ�ʱ��
int g_ilink_status = I_LINK_CONNECT_STA_SETTING_STATUS_START;//����״̬״̬��

/* �Ѿ���ȡ����SSID���� */
int g_ssid_read_len = 0;

/* �Ѿ���ȡ�������볤�� */
int g_pwd_read_len = 0;

/* �Ѿ���ȡ����SN���� */
int g_sn_read_len = 0;

/* �Ѿ���ȡ������������� */
int g_rdm_read_len = 0;

/* �Ѿ���ȡ����IP���� */
int g_ip_read_len = 0;

int g_resp_info_flag = RESP_INFO_DISABLE;

int g_sum_ready_flag = 0;

uint32_t g_sync_cnt = 0;


/* �߼�ͨ����ӳ��� */
uint8_t ChnGruop[13] = {1, 4, 7, 10, 13, 2, 5, 8, 11, 3, 6, 9, 12};//�Ż�

/* ���һ�ν���ʱ�� */
uint32_t  LastTimeMs = 0;

/* ��ǰʱ�� */
uint32_t NowTimeMs = 0;

/* ��ǰ�߼�ͨ���� */
uint32_t CurrentGroupPoint = 0;

#define ILINK_BUF_CNT   (400)
I_LINK_CONNECT_WIFI_FRAME_HEAD_SHORT_STR *_g_pilink_recv_buf = NULL;

uint32_t _g_ilink_buf_tail = 0;
uint32_t _g_ilink_buf_head = 0;
static honyar_ilink_success_cb_t g_ilink_success_cb;


int InitMonStr( void );
int InitStaMulticastStr( void );

LOCAL void _ilink_change_channel(uint8_t channel)
{
    InitMonStr();
    InitStaMulticastStr();//��ͨ��ʱ��ʼ������
    wifi_set_channel(channel);
    honyar_msleep(10);
    os_printf("channel: %d\r\n", channel);
    LastTimeMs = system_get_time() / 1000;
    _g_ilink_buf_head = _g_ilink_buf_tail;
    g_ilink_status = I_LINK_CONNECT_STA_SETTING_STATUS_START;
}

LOCAL void _ilink_rescan_channel(void)
{
    InitMonStr();
    InitStaMulticastStr();//��ͨ��ʱ��ʼ������
    honyar_msleep(50);
    LastTimeMs = system_get_time() / 1000;
    _g_ilink_buf_head = _g_ilink_buf_tail;
    g_ilink_status = I_LINK_CONNECT_STA_SETTING_STATUS_START;
}

LOCAL void ilink_relock_channel(uint8_t *bssid)
{
    uint32_t num = 0;
	wifi_scan_result_info_t *plist;
    uint32_t i;
    
    if(honyar_wifi_get_list(&plist, &num)) {
        return;
    }

    for(i = 0; i < num; i++) {
        if(!os_memcmp(bssid, plist[i].mac, MAC_ADDR_LEN)) {
            os_printf("relock channel from %d to %d.", ChnGruop[CurrentGroupPoint], plist[i].channel);
            wifi_set_channel(plist[i].channel);
        }
    }
}

LOCAL void _ilink_order(uint8_t *pframe, uint8_t *pdata, uint32_t cnt)
{
    uint32_t i;

    for (i = 0; i < cnt - 1; i++)
    {
        uint32_t j;
        for (j = i; j < cnt; j++)
        {
            if (pframe[i] > pframe[j])
            {
                uint8_t temp;
                temp = pframe[i];
                pframe[i] = pframe[j];
                pframe[j] = temp;

                temp = pdata[i];
                pdata[i] = pdata[j];
                pdata[j] = temp;
            }
        }
    }
}

/* ����Ƿ���ȡ��� */
int32_t _ilink_check_over(void)
{
    /* �ܳ����Ƿ��ȡ�� */
    //if (0xff == g_MonBaseStr.DataLen)
    //{
    //  return -1;
    //}

    /* �������Ƿ��ȡ�� */
    if(0xff == g_MonBaseStr.Cmd)
    {
        return -2;
    }
    
    /* У���ֽ��Ƿ��ȡ�� */
    //if (0 == g_sum_ready_flag)
    //{
    //  return -3;
    //}

    if (g_ssid_read_len != g_StaMulticastStr.SsidLen)
    {
        return -4;
    }

    if (g_pwd_read_len != g_StaMulticastStr.PwdLen)
    {
        return -5;
    }

    if ((g_MonBaseStr.Cmd == I_LINK_CONNECT_CMD_STA_SETTING_SN))
    {       
        /* SN�Ƿ���ȫ */
        if (g_sn_read_len != g_StaMulticastStr.SnLen)
        {
            return -6;
        }

        uint8_t *sn = honyar_device_get_sn();
        if (os_strlen((char *)sn) < g_StaMulticastStr.SnLen)
        {
            return -7;
        }

        /* ��SN���� */
        _ilink_order(g_StaMulticastStr.sn_frame, g_StaMulticastStr.Sn, 36);

        /* ���벻ƥ�� */
        if (os_strcmp((char *)g_StaMulticastStr.Sn, &sn[os_strlen((char *)sn) - g_StaMulticastStr.SnLen]))
        {
            /* �ڵ�ǰͨ���������� */
            os_printf("NOW SN:%s; REAL SN:%s\n", (char *)g_StaMulticastStr.Sn, sn);
            _ilink_rescan_channel();
            return -8;
        }
    }

    if ( g_resp_info_flag == RESP_INFO_DISABLE )
    {
        return -9;
    }
    
    if ( g_rdm_read_len != g_StaMulticastStr.RespRandomIdLen )
    {
        return -10;
    }
    
    if ( g_ip_read_len != g_StaMulticastStr.RespInfoIpLen )
    {
        return -11;
    }
    return 0;   
}

int IsMulticastAddr(uint8_t *addr, I_LINK_CONNECT_MULTICAST_BASE_STR *multicastStr)
{
    if((0x01 == *addr)&&(0x00 == *(addr+1))&&(0x5e == *(addr+2)))//�鲥
    {
        if(
            ((0xff != *(addr+3))&&(0x0 != *(addr+3)))
            &&((0xff != *(addr+4)&&(0x0 != *(addr+4))))
            &&((0xff != *(addr+5))&&(0x0 != *(addr+4)))
            )//���˵���0xFF�鲥��㲥������
        {
            multicastStr->StepNo = *(addr+3)&0xff;
            multicastStr->FrameNo = *(addr+4)&0xff;
            multicastStr->Data = *(addr+5)&0xff;
            return 1;
        }
        else
        {
            return 2;
        }
    }
    return 3;
}

int InitMonStr(void)
{
    int tmpJ=0;
    for(tmpJ=0;tmpJ<sizeof(g_MonBaseStr.Sync);tmpJ++)
    {
        g_MonBaseStr.Sync[tmpJ] = 0xff;
    }
    g_MonBaseStr.DataLen = 0xff;
    g_MonBaseStr.Cmd = 0xff;
    g_MonBaseStr.ResvStart = 0xff;
    memset(&g_MonBaseStr.DataStr,0xff,sizeof(g_MonBaseStr.DataStr));
    g_MonBaseStr.ChkSum = 0xff;
    memset(&g_MonBaseStr.ResvEnd[0],0xff,sizeof(g_MonBaseStr.ResvEnd));
    memset(&g_WifiFrameHeadStr,0,sizeof(g_WifiFrameHeadStr));

    /* �Ѿ���ȡ����SSID���� */
    g_ssid_read_len = 0;

    /* �Ѿ���ȡ�������볤�� */
    g_pwd_read_len = 0;

    /* �Ѿ���ȡ����SN���� */
    g_sn_read_len = 0;

    /* �Ѿ���ȡ������������� */
    g_rdm_read_len = 0;

    /* �Ѿ���ȡ����IP���� */
    g_ip_read_len = 0;

    /* У�����δ׼���� */
    g_sum_ready_flag = 0;

    /* ͬ��ͷ���� */
    g_sync_cnt = 0;
}

int InitStaMulticastStr(void)
{
    int tmpJ=0;
    for(tmpJ=0;tmpJ<sizeof(g_StaMulticastStr.Sync);tmpJ++)
    {
        g_StaMulticastStr.Sync[tmpJ] = 0xff;
    }
    g_StaMulticastStr.DataLen = 0xff;
    g_StaMulticastStr.Cmd = 0xff;
    g_StaMulticastStr.SsidLen = 0xff;
    g_StaMulticastStr.PwdLen = 0xff;
    g_StaMulticastStr.Ver = 0xff;
    
    for(tmpJ=0;tmpJ<sizeof(g_StaMulticastStr.Ssid);tmpJ++)
    {
        g_StaMulticastStr.Ssid[tmpJ] = 0;
    }
    
    for(tmpJ=0;tmpJ<sizeof(g_StaMulticastStr.Pwd);tmpJ++)
    {
        g_StaMulticastStr.Pwd[tmpJ] = 0;
    }
    
    for(tmpJ=0;tmpJ<sizeof(g_StaMulticastStr.Sn);tmpJ++)
    {
        g_StaMulticastStr.Sn[tmpJ] = 0;
    }

    for(tmpJ=0;tmpJ<sizeof(g_StaMulticastStr.RespInfoIp);tmpJ++)
    {
        g_StaMulticastStr.RespInfoIp[tmpJ] = 0;
    }

    for(tmpJ=0;tmpJ<sizeof(g_StaMulticastStr.RespRandomId);tmpJ++)
    {
        g_StaMulticastStr.RespRandomId[tmpJ] = 0;
    }

    for(tmpJ=0;tmpJ<sizeof(g_StaMulticastStr.ssid_frame);tmpJ++)
    {
        g_StaMulticastStr.ssid_frame[tmpJ] = 0xff;
    }
    
    for(tmpJ=0;tmpJ<sizeof(g_StaMulticastStr.pwd_frame);tmpJ++)
    {
        g_StaMulticastStr.pwd_frame[tmpJ] = 0xff;
    }
    
    for(tmpJ=0;tmpJ<sizeof(g_StaMulticastStr.sn_frame);tmpJ++)
    {
        g_StaMulticastStr.sn_frame[tmpJ] = 0xff;
    }

    for(tmpJ=0;tmpJ<sizeof(g_StaMulticastStr.RespInfoIpFrame);tmpJ++)
    {
        g_StaMulticastStr.RespInfoIpFrame[tmpJ] = 0xff;
    }

    for(tmpJ=0;tmpJ<sizeof(g_StaMulticastStr.RespRandomIdFrame);tmpJ++)
    {
        g_StaMulticastStr.RespRandomIdFrame[tmpJ] = 0xff;
    }
    
    g_StaMulticastStr.SnLen = 0xff;

    g_StaMulticastStr.RespInfoIpLen = RESP_IP_LEN;
    
    g_StaMulticastStr.RespRandomIdLen = RESP_RAND_LEN;

    g_StaMulticastStr.ChkSum = 0xff;
}

void DealMonCmdPro(I_LINK_CONNECT_MULTICAST_BASE_STR *multicastStr)
{
    if(0 == multicastStr->FrameNo)
    {
        return;
    }
    
    g_llLastReceiveTime = system_get_time() / 1000;  
    switch(multicastStr->StepNo)
    {
    case I_LINK_CONNECT_STA_SETTING_STATUS_IN_DATA_LEN://= 102,//�����ܳ���         
        {   
            if(0xff == g_MonBaseStr.DataLen)
            {
                g_MonBaseStr.DataLen = multicastStr->Data;
            }
        }
        break;

    case I_LINK_CONNECT_STA_SETTING_STATUS_IN_CMD:// = 103,//���ڽ���������
        {
            if(0xff == g_MonBaseStr.Cmd)
            {
                g_MonBaseStr.Cmd = multicastStr->Data;

                if (g_MonBaseStr.Cmd == I_LINK_CONNECT_CMD_STA_SETTING_SN)
                {
                    os_printf("CMD=CK_SN\n");
                }
                else if (g_MonBaseStr.Cmd == I_LINK_CONNECT_CMD_STA_SETTING_NON_SN)
                {
                    os_printf("CMD=NO_SN\n");
                }
            }
        }
        break;  

    case I_LINK_CONNECT_STA_SETTING_STATUS_IN_SSID_LEN:// = 111,//���ڽ���SSID��PWD���볤��
        {
            if(0xff == g_StaMulticastStr.SsidLen)
            {
                g_StaMulticastStr.SsidLen = multicastStr->Data;
                os_printf("SSID_LEN=%d\n", g_StaMulticastStr.SsidLen);
            }
        }
        break;
        
    case I_LINK_CONNECT_STA_SETTING_STATUS_IN_PWD_LEN:// = 112,//���ڽ���SSID��PWD���볤��
        {
            if(0xff == g_StaMulticastStr.PwdLen)
            {
                g_StaMulticastStr.PwdLen = multicastStr->Data;
                os_printf("PWD_LEN=%d\n", g_StaMulticastStr.PwdLen);
            }
        }
        break;
        
    case I_LINK_CONNECT_STA_SETTING_STATUS_IN_VER:// = 113,//���ڽ���Э��汾
        {
            if(0xff == g_StaMulticastStr.Ver)
            {
                g_StaMulticastStr.Ver = multicastStr->Data;
                os_printf("VER=%d\n", g_StaMulticastStr.Ver);
                if(10 == g_StaMulticastStr.Ver)
                {
                    g_resp_info_flag = 1;
                    g_rdm_read_len = RESP_RAND_LEN;
                    g_ip_read_len = RESP_IP_LEN;
                    //�ɵ�����������û�������
                    //set_wifi_config_app_ip(0);
                    //set_wifi_config_random(0);
                }
            }
        }
        break;
            
    case I_LINK_CONNECT_STA_SETTING_STATUS_IN_SSID:// = 112,//���ڽ���SSID
        {
            /* ���ݺ�֡���һͬ���뻺�棬ͬʱ��¼���յ���Ԫ�صĸ���������SSID_LEN������ж����꣬���������� */
            if (g_ssid_read_len == g_StaMulticastStr.SsidLen)
            {
                /* SSID��ȡ��� */
                break;
            }
            
            uint32_t i;
            for (i = 0; i < 36; i++)
            {
                if (g_StaMulticastStr.ssid_frame[i] == multicastStr->FrameNo)
                {
                    /* �ظ���ȡ��֡ */
                    break;
                }
            }

            if (i < 36)
            {
                /* �ظ���ȡ��֡ */
                break;
            }

            for (i = 0; i < 36; i++)
            {
                if (g_StaMulticastStr.ssid_frame[i] == 0xff)
                {
                    /* ����ȡ��֡ */
                    if (multicastStr->Data > 0)
                    {
                        g_StaMulticastStr.ssid_frame[i] = multicastStr->FrameNo;
                        g_StaMulticastStr.Ssid[i] = multicastStr->Data;
                        g_ssid_read_len++;
                        os_printf("SSID:%d/%d\n", g_ssid_read_len, g_StaMulticastStr.SsidLen);
                    }
                    else
                    {
                        os_printf("Invalid SSID char\n");
                    }
break;
                }
            }
        }
        break;
        
    case I_LINK_CONNECT_STA_SETTING_STATUS_IN_PWD:// = 113,//���ڽ���PWD����
        {
            /* ���ݺ�֡���һͬ���뻺�棬ͬʱ��¼���յ���Ԫ�صĸ���������SSID_LEN������ж����꣬���������� */
            if (g_pwd_read_len == g_StaMulticastStr.PwdLen)
            {
                /* PWD��ȡ��� */
                break;
            }
            
            uint32_t i;
            for (i = 0; i < 68; i++)
            {
                if (g_StaMulticastStr.pwd_frame[i] == multicastStr->FrameNo)
                {
                    /* �ظ���ȡ��֡ */
                    break;
                }
            }

            if (i < 68)
            {
                /* �ظ���ȡ��֡ */
                break;
            }

            for (i = 0; i < 68; i++)
            {
                if (g_StaMulticastStr.pwd_frame[i] == 0xff)
                {
                    if (multicastStr->Data > 0)
                    {
                        /* ����ȡ��֡ */
                        g_StaMulticastStr.pwd_frame[i] = multicastStr->FrameNo;
                        g_StaMulticastStr.Pwd[i] = multicastStr->Data;
                        g_pwd_read_len++;
                        os_printf("PWD:%d/%d\n", g_pwd_read_len, g_StaMulticastStr.PwdLen);
                    }
                    else
                    {
                        os_printf("Invalid PWD char\n");
                    }
                    break;
                }
            }
        }
        break;

    case I_LINK_CONNECT_STA_SETTING_STATUS_IN_SN_LEN:// = 111,//���ڽ���SN����
        {
            if(0xff == g_StaMulticastStr.SnLen)
            {
                g_StaMulticastStr.SnLen = multicastStr->Data;
                os_printf("SN_LEN=%d\n", g_StaMulticastStr.SnLen);
            }
        }
        break;
        
    case I_LINK_CONNECT_STA_SETTING_STATUS_IN_SN:// = 114,//���ڽ���SN
        {
            /* ���ݺ�֡���һͬ���뻺�棬ͬʱ��¼���յ���Ԫ�صĸ���������SSID_LEN������ж����꣬���������� */
            if (g_sn_read_len == g_StaMulticastStr.SnLen)
            {
                /* SN��ȡ��� */
                break;
            }
            
            uint32_t i;
            for (i = 0; i < 36; i++)
            {
                if (g_StaMulticastStr.sn_frame[i] == multicastStr->FrameNo)
                {
                    /* �ظ���ȡ��֡ */
                    break;
                }
            }

            if (i < 36)
            {
                /* �ظ���ȡ��֡ */
                break;
            }

            for (i = 0; i < 36; i++)
            {
                if (g_StaMulticastStr.sn_frame[i] == 0xff)
                {
                    /* ����ȡ��֡ */
                    g_StaMulticastStr.sn_frame[i] = multicastStr->FrameNo;
                    g_StaMulticastStr.Sn[i] = multicastStr->Data;
                    g_sn_read_len++;
                    os_printf("SN:%d/%d\n", g_sn_read_len, g_StaMulticastStr.SnLen);
                    break;
                }
            }
        }
        break;

    case I_LINK_CONNECT_STA_SETTING_STATUS_IN_RESP_INFO_FLAG:
        {
            g_resp_info_flag = RESP_INFO_ENABLE;
            g_StaMulticastStr.RespInfoFlag = multicastStr->Data;
        }
        break;

    case I_LINK_CONNECT_STA_SETTING_STATUS_RESP_INFO:
        {
            /* ���ݺ�֡���һͬ���뻺�棬ͬʱ��¼���յ���Ԫ�صĸ���������IP_LEN������ж����꣬���������� */
            if ( g_ip_read_len == g_StaMulticastStr.RespInfoIpLen )
            {
                /* IP��ȡ��� */
                break;
            }
            
            uint32_t i;
            for ( i = 0; i < RESP_IP_LEN; i++ )
            {
                if (g_StaMulticastStr.RespInfoIpFrame[i] == multicastStr->FrameNo)
                {
                    /* �ظ���ȡ��֡ */
                    break;
                }
            }

            if ( i < RESP_IP_LEN )
            {
                /* �ظ���ȡ��֡ */
                break;
            }

            for ( i = 0; i < RESP_IP_LEN; i++ )
            {
                if ( g_StaMulticastStr.RespInfoIpFrame[i] == 0xff )
                {
                    /* ����ȡ��֡ */
                    g_StaMulticastStr.RespInfoIpFrame[i] = multicastStr->FrameNo;
                    g_StaMulticastStr.RespInfoIp[i] = multicastStr->Data;
                    g_ip_read_len++;
                    os_printf("IP[%d]\n", g_StaMulticastStr.RespInfoIp[i] );
                    break;
                }
            }
        }
        break;

        case I_LINK_CONNECT_STA_SETTING_STATUS_RESP_RANDOM:       /* 120,���ڽ��ջظ������ */
        {
            /* ���ݺ�֡���һͬ���뻺�棬ͬʱ��¼���յ���Ԫ�صĸ���������RAND_LEN������ж����꣬���������� */
            if ( g_rdm_read_len == g_StaMulticastStr.RespRandomIdLen)
            {
                /* RandomID��ȡ��� */
                break;
            }
            
            uint32_t i;
            for ( i = 0; i < RESP_RAND_LEN; i++ )
            {
                if (g_StaMulticastStr.RespRandomIdFrame[i] == multicastStr->FrameNo)
                {
                    /* �ظ���ȡ��֡ */
                    break;
                }
            }

            if ( i < RESP_RAND_LEN )
            {
                /* �ظ���ȡ��֡ */
                break;
            }

            for ( i = 0; i < RESP_RAND_LEN; i++ )
            {
                if ( g_StaMulticastStr.RespRandomIdFrame[i] == 0xff )
                {
                    /* ����ȡ��֡ */
                    g_StaMulticastStr.RespRandomIdFrame[i] = multicastStr->FrameNo;
                    g_StaMulticastStr.RespRandomId[i] = multicastStr->Data;
                    g_rdm_read_len++;
                    os_printf("RANDOM:[%d]\n", g_StaMulticastStr.RespRandomId[i]);
                    os_printf("RANDOM_LEN:%d/%d\n", g_rdm_read_len, g_StaMulticastStr.RespRandomIdLen);
                    break;
                }
            }
        }
        break;
        
    case I_LINK_CONNECT_STA_SETTING_STATUS_IN_SUM:// = 199,//���ڽ��պ�У��
        {
            if (g_sum_ready_flag == 0)
            {
                g_MonBaseStr.ChkSum = multicastStr->Data;
                g_sum_ready_flag = 1;
                os_printf("CKSUM:%d\n", g_MonBaseStr.ChkSum);
            }
        }
        break;
        
    default:
        {
            
        }
        break;
    }
}

int DealMonDataPro(I_LINK_CONNECT_MULTICAST_BASE_STR *multicastStr,I_LINK_CONNECT_WIFI_FRAME_HEAD_SHORT_STR *wifiFrameHeadStr)
{
    //int tmpStatus = nowStatus;
    if(0 == multicastStr->FrameNo)
    {
        //tmpStatus = I_LINK_CONNECT_STA_SETTING_STATUS_START;
        return;
    }
    
    if((126 < multicastStr->StepNo)||(101 > multicastStr->StepNo))
    {
        return;
    }
    g_llLastReceiveTime = system_get_time() / 1000;

    static uint8_t ap[MAC_ADDR_LEN] = {0};
    
    switch(g_ilink_status)
    {
    case I_LINK_CONNECT_STA_SETTING_STATUS_START://�տ�ʼ
        {
            os_printf("lock channel: %d!\n", ChnGruop[CurrentGroupPoint]);
            //PrintHexData("Data:", (uint8_t *)wifiFrameHeadStr, sizeof(I_LINK_CONNECT_WIFI_FRAME_HEAD_SHORT_STR));
            g_ilink_status = I_LINK_CONNECT_STA_SETTING_STATUS_IN_SYNC;
            g_llLastSyncReceiveTime = system_get_time() / 1000;
        }
        break;
        
    case I_LINK_CONNECT_STA_SETTING_STATUS_IN_SYNC://= 101, 0x65,//���ڽ���ͬ��ͷ   
        {
            if(I_LINK_CONNECT_STA_SETTING_STATUS_IN_SYNC == multicastStr->StepNo)
            {
                if (multicastStr->Data == 1 && multicastStr->FrameNo <= 5)
                {
                    if(0 == g_sync_cnt)
                    {
                        memcpy(ap, wifiFrameHeadStr->bssid, MAC_ADDR_LEN);
                        ilink_relock_channel(ap);
                    }
                    else if(memcmp(ap, wifiFrameHeadStr->bssid, MAC_ADDR_LEN))
                    {
                        InitMonStr();
                        InitStaMulticastStr();
                        memcpy(ap, wifiFrameHeadStr->bssid, MAC_ADDR_LEN);
                        ilink_relock_channel(ap);
                    }
                    
                    if (g_StaMulticastStr.Sync[multicastStr->FrameNo - 1] == 1)
                    {
                        break;
                    }

                    g_StaMulticastStr.Sync[multicastStr->FrameNo - 1] = 1;
                    g_sync_cnt++;

                    os_printf("SYNC: %d/5!\n", g_sync_cnt);
                    if (g_sync_cnt < 5)
                    {
                        break;
                    }
                
                    g_ilink_status = I_LINK_CONNECT_STA_SETTING_STATUS_IN_DATA_LEN;
                }
            }
        }
        break;
        
    default:
        {
            if(os_memcmp(ap, wifiFrameHeadStr->bssid, MAC_ADDR_LEN))
            {
                break;
            }
            DealMonCmdPro(multicastStr);
        }
        break;
    }

    return g_ilink_status;
}

LOCAL void ilink_data_enqueue(uint8_t *pdest, uint8_t *psrc, uint8_t *pbssid)
{
    uint32_t tail = _g_ilink_buf_tail;
    uint32_t head = _g_ilink_buf_head;
    tail++;
    if (tail >= ILINK_BUF_CNT) {
        tail = 0;
    }
    if(tail == head) {
        //full;
        return;
    }
    memcpy(_g_pilink_recv_buf[_g_ilink_buf_tail].dest, pdest, MAC_ADDR_LEN);
    memcpy(_g_pilink_recv_buf[_g_ilink_buf_tail].bssid, pbssid, MAC_ADDR_LEN);
    _g_ilink_buf_tail = tail;
}

LOCAL void ilink_recv_event(uint8_t *pbuf, uint16_t buf_len)
{
    if(buf_len < 12 + sizeof(I_LINK_CONNECT_WIFI_FRAME_HEAD_STR))
    {
        return;
    }

    uint16_t pkg_len = buf_len;
    if(buf_len == 128)
    {
        //struct sniffer_buf2
        memcpy(&pkg_len, pbuf + 126, 2);
    }
    else if(0 == (buf_len % 10))
    {
        //struct sniffer_buf
        if(buf_len != 60)
        {
            //Ϊ���㣬ֻ��������
            return;
        }
        memcpy(&pkg_len, pbuf + 50, 2);
    }
    else
    {
        //err.
        return;
    }
    
    I_LINK_CONNECT_WIFI_FRAME_HEAD_STR frame;
    I_LINK_CONNECT_WIFI_FRAME_HEAD_STR *_g_pframe = &frame;
    memcpy(_g_pframe, pbuf+12, sizeof(I_LINK_CONNECT_WIFI_FRAME_HEAD_STR));

    uint8_t *pdest = NULL;
    uint8_t *psrc = NULL;
    uint8_t *pbssid = NULL;
    if(frame.FC.ToDs)
    {
        //����
        pdest = frame.Addr3;
        if(frame.FC.FrDs)
        {
            return; 
        }
        else
        {
            pbssid = frame.Addr1;
            psrc = frame.Addr2;
            pkg_len -= 2; //��������Ӧ�ü�ȥ�����ֽڳ���
        }
    }
    else
    {
        //����
        pdest = frame.Addr1;
        if(frame.FC.FrDs)
        {
            pbssid = frame.Addr2;
            psrc = frame.Addr3;
        }
        else
        {
            return;
        }
        
    }
    if(0xff == pdest[4] || 0x7f == pdest[4] || 0x00 == pdest[4])
    {
        return;
    }
    if(!os_memcmp(pdest, "\x01\x00\x5E", 3))
    {
        ilink_data_enqueue(pdest, psrc, pbssid);
    }
}

int32_t ilink_read(I_LINK_CONNECT_WIFI_FRAME_HEAD_SHORT_STR *pbuf)
{
    uint32_t head = _g_ilink_buf_head;
    if (_g_ilink_buf_head == _g_ilink_buf_tail) {
        //empty
        return -1;
    }
    
    memcpy(pbuf, &_g_pilink_recv_buf[_g_ilink_buf_head], sizeof(I_LINK_CONNECT_WIFI_FRAME_HEAD_SHORT_STR));
    head++;
    if (head >= ILINK_BUF_CNT) {
        head = 0;
    }
    _g_ilink_buf_head = head;
    return 0;
}

LOCAL void honyar_ilink_task(void *ppara)
{
    //2.�������ݴ���
    {
#if 1
        //1���Ƿ��л�Ƶ�����Ǿ��л�Ƶ��Ȼ���ʼ��������
        NowTimeMs = system_get_time() / 1000;
        
        if((WIFI_SWITCH_FREQ_TIME < ( NowTimeMs-LastTimeMs))&&(I_LINK_CONNECT_STA_SETTING_STATUS_START == g_ilink_status))//����ʱ����Ƶ��
        {
            CurrentGroupPoint++;
            if(sizeof(ChnGruop) <= CurrentGroupPoint)
            {
                CurrentGroupPoint = 0;
            }

            _ilink_change_channel(ChnGruop[CurrentGroupPoint]);
        }
        else 
        {
            if(I_LINK_CONNECT_STA_SETTING_STATUS_START != g_ilink_status)
            {
                if(WIFI_WAIT_TIMEOUT_TIME < (NowTimeMs - g_llLastSyncReceiveTime))
                {
                    g_llLastSyncReceiveTime = NowTimeMs;
                    g_llLastReceiveTime = NowTimeMs;
                    InitMonStr();
                    InitStaMulticastStr();
                    g_ilink_status = I_LINK_CONNECT_STA_SETTING_STATUS_START;
                }
                
                if(WIFI_WAIT_FRAME_TIMEOUT_TIME <(NowTimeMs - g_llLastReceiveTime))//����������ݽ�����ͬ�������Ƿ��ֲ����Լ���
                {
                    os_printf("too long time...%d\n", ChnGruop[CurrentGroupPoint]);
                    g_llLastSyncReceiveTime = NowTimeMs;
                    g_llLastReceiveTime = NowTimeMs;
                    InitMonStr();
                    InitStaMulticastStr();
                    g_ilink_status = I_LINK_CONNECT_STA_SETTING_STATUS_START;
                    #if 0
                    if(0 == CurrentGroupPoint)
                    {
                        CurrentGroupPoint = sizeof(ChnGruop)/sizeof(ChnGruop[0]) - 1;
                    }
                    else
                    {
                        CurrentGroupPoint--;
                    }
                    #endif
                    CurrentGroupPoint++;
                    if(sizeof(ChnGruop) <= CurrentGroupPoint)
                    {
                        CurrentGroupPoint = 0;
                    }
                    _ilink_change_channel(ChnGruop[CurrentGroupPoint]);
                }
            }
        }
#endif
    
        I_LINK_CONNECT_WIFI_FRAME_HEAD_SHORT_STR tmpWifiFrameHeadStr;
        memset(&tmpWifiFrameHeadStr, 0, sizeof(tmpWifiFrameHeadStr));

        while (ilink_read(&tmpWifiFrameHeadStr) == 0)
        {
            I_LINK_CONNECT_MULTICAST_BASE_STR tmpMulticastStr;
            memset(&tmpMulticastStr, 0, sizeof(tmpMulticastStr));
            tmpMulticastStr.StepNo = tmpWifiFrameHeadStr.dest[3];
            tmpMulticastStr.FrameNo = tmpWifiFrameHeadStr.dest[4];
            tmpMulticastStr.Data = tmpWifiFrameHeadStr.dest[5];
            DealMonDataPro(&tmpMulticastStr, &tmpWifiFrameHeadStr);

            if (_ilink_check_over() == 0)
            {
                if(10 != g_StaMulticastStr.Ver)
                {
                    _ilink_order(g_StaMulticastStr.RespInfoIpFrame, g_StaMulticastStr.RespInfoIp, 4);
                    _ilink_order(g_StaMulticastStr.RespRandomIdFrame, g_StaMulticastStr.RespRandomId, 2);
                    uint16_t random = g_StaMulticastStr.RespRandomId[0] + (g_StaMulticastStr.RespRandomId[1] << 8);
                    uint32_t ip = g_StaMulticastStr.RespInfoIp[3] + (g_StaMulticastStr.RespInfoIp[2] << 8)
                                      + (g_StaMulticastStr.RespInfoIp[1] << 16) + (g_StaMulticastStr.RespInfoIp[0] << 24);
                    //set_wifi_config_app_ip(ip);
                    //set_wifi_config_random(random);
                    os_printf("app ip addr: %d, random: %d\r\n", ip, random);
                }
                
                /* ��SSID���� */
                _ilink_order(g_StaMulticastStr.ssid_frame, g_StaMulticastStr.Ssid, 36);
                /* ��PWD���� */
                _ilink_order(g_StaMulticastStr.pwd_frame, g_StaMulticastStr.Pwd, 68);

                honyar_wifi_set_router_ssid(g_StaMulticastStr.Ssid);
                honyar_wifi_set_router_passwd(g_StaMulticastStr.Pwd);
                dl_config_commit_later();
                if(g_ilink_success_cb) {
                    g_ilink_success_cb();
                }
                dl_config_commit(0);
                honyar_sys_reboot(0);
                honyar_del_task(honyar_ilink_task);
                return;
            }
        }
    }
}

static void ICACHE_FLASH_ATTR 
_honyar_ilink_init(void *parm)
{
    if(!honyar_wifi_scan_isover()) {
        return;
    }
    honyar_del_task(_honyar_ilink_init);
    if (_g_pilink_recv_buf) {
        honyar_free(_g_pilink_recv_buf);
        _g_pilink_recv_buf = NULL;
    }
    
    hy_printf("Lib ilink "iLinkConnectVer"\r\n");

    _g_pilink_recv_buf = (I_LINK_CONNECT_WIFI_FRAME_HEAD_SHORT_STR *)honyar_malloc(ILINK_BUF_CNT * sizeof(I_LINK_CONNECT_WIFI_FRAME_HEAD_SHORT_STR));
    memset(_g_pilink_recv_buf, 0, (sizeof(I_LINK_CONNECT_WIFI_FRAME_HEAD_SHORT_STR) * ILINK_BUF_CNT));

    memset(&g_MonBaseStr, 0, sizeof(g_MonBaseStr));
    memset(&g_StaMulticastStr, 0, sizeof(g_StaMulticastStr));
    memset(&g_WifiFrameHeadStr, 0, sizeof(g_WifiFrameHeadStr));

    LastTimeMs = system_get_time() / 1000;
    NowTimeMs = LastTimeMs;

    g_ilink_status = I_LINK_CONNECT_STA_SETTING_STATUS_START;
    
    InitMonStr();
    InitStaMulticastStr();

    hy_info("wifi_promiscuous_enable...\r\n");
    {
        wifi_set_promiscuous_rx_cb(ilink_recv_event);           
        
        /* ��������ģʽ */
        wifi_set_opmode_current(STATION_MODE);
        wifi_station_disconnect();
        wifi_promiscuous_enable(1);
    }

    /* ѡ��ͨ�� */
    CurrentGroupPoint = 0;
    wifi_set_channel(ChnGruop[CurrentGroupPoint]);
    hy_printf("channel: %d\r\n", ChnGruop[CurrentGroupPoint]);
    honyar_add_task(honyar_ilink_task, NULL, 0);
}

void ICACHE_FLASH_ATTR
honyar_ilink_regist_success_cb(honyar_ilink_success_cb_t func)
{
    g_ilink_success_cb = func;
}

void ICACHE_FLASH_ATTR 
honyar_ilink_init(void)
{
    honyar_wifi_scan();
    honyar_add_task(_honyar_ilink_init, NULL, 1000 / TASK_CYCLE_TM_MS);
}

