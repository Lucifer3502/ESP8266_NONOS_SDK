
#ifndef _HONYAR_ILINK_H_
#define _HONYAR_ILINK_H_

#include "c_types.h"


typedef struct __I_LINK_CONNECT_STA_SETTING_MULTICAST_STR
{
	unsigned char Sync[5];//ͬ��ͷ"11111",StepNo=I_LINK_CONNECT_STA_SETTING_STATUS_IN_SYNC,��������1
	unsigned char DataLen;//ȥ��ͬ��ͷ���������ݳ���,StepNo=I_LINK_CONNECT_STA_SETTING_STATUS_IN_DATA_LEN����������22
	unsigned char Cmd;			//,StepNo=I_LINK_CONNECT_STA_SETTING_STATUS_IN_CMD,��������33,�����Bit6Ϊ�ر�������֤��־(1���ر���֤��0��������֤),bit5~bit0����������Ϊ0x1F,I_LINK_CONNECT_CMD_TYPE
	
	unsigned char SsidLen;//ssid����,StepNo=I_LINK_CONNECT_STA_SETTING_STATUS_IN_SSID_LEN,��������4
	unsigned char PwdLen;//PWD����,StepNo=I_LINK_CONNECT_STA_SETTING_STATUS_IN_PWD_LEN,��������4
	unsigned char Ver;//���ܷ�ʽ,StepNo=I_LINK_CONNECT_STA_SETTING_STATUS_IN_SECMOD,��������4
	unsigned char Ssid[36];//Ssid,StepNo=I_LINK_CONNECT_STA_SETTING_STATUS_IN_SSID,��������44
	unsigned char ssid_frame[36];//ssid֡���
	unsigned char Pwd[68];//PWD,StepNo=I_LINK_CONNECT_STA_SETTING_STATUS_IN_SUM,��������444
	unsigned char pwd_frame[68];//
	unsigned char SnLen;//SN����,StepNo=I_LINK_CONNECT_STA_SETTING_STATUS_IN_SN_LEN,��������4444
	unsigned char Sn[36];//SN,StepNo=I_LINK_CONNECT_STA_SETTING_STATUS_IN_SN,��������4444
	unsigned char sn_frame[36];//
	unsigned char RespInfoFlag;//I_LINK_CONNECT_STA_SETTING_STATUS_IN_RESP_INFO_FLAG,��������44444
    unsigned char RespInfoIp[4];//SN,StepNo=I_LINK_CONNECT_STA_SETTING_STATUS_IN_RESP_INFO,��������44444
    //(��RespIpByte1��RespIpByte2��RespIpByte3��RespIpByte4��δ255ʱ�㲥�ظ���RespPortΪ0ʱ���չ̶��˿ڻظ�)
    unsigned char RespInfoIpLen;
    unsigned char RespInfoIpFrame[4];
    unsigned char RespRandomId[2];//MagicId��2������ֽ�,���ֽ���ǰ,I_LINK_CONNECT_STA_SETTING_STATUS_IN_RESP_INFO,��������44444
    unsigned char RespRandomIdLen;
    unsigned char RespRandomIdFrame[2];
    //**(���� RespIpByte1:ip��1���ֽ�,RespIpByte2:ip��2���ֽ�,RespIpByte3:ip��3���ֽ�,RespIpByte4:ip��4���ֽ�,RespRandomId,�ظ�������ݴ�)
    // ** (��RespIpByte1��RespIpByte2��RespIpByte3��RespIpByte4��δ255ʱ�㲥�ظ����̶��˿�Ϊ20087�ظ�)
    // ** (������Ϣ����XML�����ظ���Ϣ���л��ͣ����溬������ֶ�)
	unsigned char ChkSum;//��У��,������ȡ����ȥ���ֽ�,StepNo=126,��������9
}I_LINK_CONNECT_STA_SETTING_MULTICAST_STR, *P_I_LINK_CONNECT_STA_SETTING_MULTICAST_STR;

#pragma pack(1)
//----------------------------1.�������ݶ���--------------------------------//
typedef enum 
{
		I_LINK_CONNECT_CMD_SET_IO_OUT = 0x04, //0xF4Ϊ����IO��valueΪGPIO��,value2Ϊ����״̬0or1��
		I_LINK_CONNECT_CMD_WRITE_PASSWD = 0x07,//��������
		I_LINK_CONNECT_CMD_INIT_PARAM = 0x0E,//������ʼ��
		I_LINK_CONNECT_CMD_MODEL_SET = 0x0D,//ģʽ���ã�valueΪģʽ����,value2Ϊģʽ���루��������or�˳���>0��ʾ��ǰ������;=0��ʾ�����ӻ����˳�;
		I_LINK_CONNECT_CMD_SN_SET = 0x12,//SN��������(����22�ֽڶ���)
		I_LINK_CONNECT_CMD_SSID = 0x13,//AP�������ã�valueΪ��ȡ��������(0,1),SSID�����ܳ��ȣ�infoΪSSID����ͷ)
		I_LINK_CONNECT_CMD_MAC = 0x14,//MAC���ã�valueΪ��ȡ��������(0,1)��value2Ϊ�����ţ�ETH0Ϊ0��ETH1Ϊ1��ath0Ϊ10,ath1Ϊ11MAC��ַ)
		I_LINK_CONNECT_CMD_UPDATE = 0x15,//������valueΪ����(0:smb;1:͸��ģ��),value2Ϊ��������(1:tftp,0:ftp)��info[0~3]Ϊ��������ַ;info[4~9]:user;info[10~15]:pwd;reserved1:port)
		I_LINK_CONNECT_CMD_TEST_MOD = 0x16,//������߲���ģʽ/ʵ�ʹ�����Ч��valueΪ�����˳�״̬,0x01����,0x00,�˳�)
		I_LINK_CONNECT_CMD_WIFI_ON = 0x17,//WIFI�����ƣ�valueΪ����,value2Ϊ��)
		I_LINK_CONNECT_CMD_WIFI_OFF = 0x18,//WIFI�ؿ��ƣ�valueΪ����,value2Ϊ��)
		I_LINK_CONNECT_CMD_IR_BYTE_SEND = 0x19,//������ƣ��������ݵķ�ʽ���п��ƣ�(����22�ֽڶ���)
		I_LINK_CONNECT_CMD_UART_BYTE_SEND = 0x1A,//�������ݷ���(����22�ֽڶ���)
		I_LINK_CONNECT_CMD_SETTING_AP_ON = 0x1B,//����AP����ģʽ
		I_LINK_CONNECT_CMD_AP_ON = 0x1C,//����APģʽ
		I_LINK_CONNECT_CMD_STA_ON = 0x1D,//����STAģʽ
		I_LINK_CONNECT_CMD_MON_ON = 0x1E,//����MONITORģʽ
		I_LINK_CONNECT_CMD_STA_SETTING_SN = 0x1F,//һ�����ã���ҪУ��SN��STA�������ã�valueΪ�Ƿ�������Ч��־)
		I_LINK_CONNECT_CMD_STA_SETTING_NON_SN = 0x5F,//һ�����ã�����ҪУ��SN��STA�������ã�valueΪ�Ƿ�������Ч��־)
		
		
}I_LINK_CONNECT_CMD_TYPE;


typedef struct __I_LINK_CONNECT_DATA_STR//������
{
	unsigned char Resv[4];//
	unsigned char info[192];//StepNo��111��ʼ
}I_LINK_CONNECT_DATA_STR, *P_I_LINK_CONNECT_MON_DATA_STR;

typedef struct __I_LINK_CONNECT_BASE_STR
{
	unsigned char Sync[5];//ͬ��ͷ"11111",StepNo=I_LINK_CONNECT_STA_SETTING_STATUS_IN_SYNC,��������1
	unsigned char DataLen;//ȥ��ͬ��ͷ���������ݳ���,StepNo=I_LINK_CONNECT_STA_SETTING_STATUS_IN_DATA_LEN����������22
	unsigned char Cmd;			//,StepNo=I_LINK_CONNECT_STA_SETTING_STATUS_IN_CMD,��������33,�����Bit7Ϊ�Ƿ���֤����ı�־,Bit6Ϊ�Ƿ�������Ч�ı�־,bit5~bit0����������Ϊ0x1F,I_LINK_CONNECT_CMD_TYPE
	unsigned char ResvStart;//
	I_LINK_CONNECT_DATA_STR DataStr;//StepNo��102��ʼ
	unsigned char ChkSum;//��У��,������ȡ����ȥ���ֽ�,StepNo=199,��������9
	unsigned char ResvEnd[3];//
}I_LINK_CONNECT_MON_BASE_STR, *P_I_LINK_CONNECT_MON_BASE_STR;



typedef struct __I_LINK_CONNECT_MULTICAST_BASE_STR
{
 	unsigned char Multicastsync;//ͬ��ͷ0x239
	unsigned char StepNo;//�����
	unsigned char FrameNo;//֡���
	unsigned char Data;//����
}I_LINK_CONNECT_MULTICAST_BASE_STR, *P_I_LINK_CONNECT_MULTICAST_BASE_STR;

//----------------------------1.�������ݶ���--------------------------------//





//----------------------------2.1��I_LINK_CONNECT_CMD_STA_SETTING = 0x1F��ϸ˵��--------------------------------//
//I_LINK_CONNECT_CMD_STA_SETTING = 0x1F,//STA�������ã�valueΪ�Ƿ�������Ч��־)
typedef enum 
{
		I_LINK_CONNECT_STA_SETTING_STATUS_START = 0,//��ʼ״̬
		I_LINK_CONNECT_STA_SETTING_STATUS_IN_SYNC = 101,//���ڽ���ͬ��ͷ
		I_LINK_CONNECT_STA_SETTING_STATUS_IN_DATA_LEN = 102,//�����ܳ���
		I_LINK_CONNECT_STA_SETTING_STATUS_IN_CMD = 103,//���ڽ���������
		I_LINK_CONNECT_STA_SETTING_STATUS_RECVD_CMD = 104,//�ѽ�����������
		I_LINK_CONNECT_STA_SETTING_STATUS_IN_SSID_LEN = 111,//���ڽ���SSID����
		I_LINK_CONNECT_STA_SETTING_STATUS_IN_PWD_LEN = 112,//���ڽ���PWD���볤��
		I_LINK_CONNECT_STA_SETTING_STATUS_IN_VER = 113,//���ڽ���VER
		I_LINK_CONNECT_STA_SETTING_STATUS_IN_SSID = 114,//���ڽ���SSID
		I_LINK_CONNECT_STA_SETTING_STATUS_IN_PWD = 115,//���ڽ���PWD����
		I_LINK_CONNECT_STA_SETTING_STATUS_IN_SN_LEN = 116,//���ڽ���SN
		I_LINK_CONNECT_STA_SETTING_STATUS_IN_SN = 117,//���ڽ���SN
		I_LINK_CONNECT_STA_SETTING_STATUS_IN_RESP_INFO_FLAG = 118,//���ڽ��ջظ�ʹ��
		I_LINK_CONNECT_STA_SETTING_STATUS_RESP_INFO = 119,//���ڽ��ջظ���Ϣ
		I_LINK_CONNECT_STA_SETTING_STATUS_RESP_RANDOM = 120,
		I_LINK_CONNECT_STA_SETTING_STATUS_IN_SUM = 126,//���ڽ��պ�У��
		I_LINK_CONNECT_STA_SETTING_STATUS_END = 200,//�������

		
}I_LINK_CONNECT_STA_SETTING_STATUS;

struct RxControl {
    signed rssi:8;
    unsigned rate:4;
    unsigned is_group:1;
    unsigned:1;
    unsigned sig_mode:2;
    unsigned legacy_length:12;
    unsigned damatch0:1;
    unsigned damatch1:1;
    unsigned bssidmatch0:1;
    unsigned bssidmatch1:1;
    unsigned MCS:7;
    unsigned CWB:1;
    unsigned HT_length:16;
    unsigned Smoothing:1;
    unsigned Not_Sounding:1;
    unsigned:1;
    unsigned Aggregation:1;
    unsigned STBC:2;
    unsigned FEC_CODING:1;
    unsigned SGI:1;
    unsigned rxend_state:8;
    unsigned ampdu_cnt:8;
    unsigned channel:4;
    unsigned:12;
};

struct LenSeq{
    u16 len; // length of packet
    u16 seq; // serial number of packet, the high 12bits are serial number,
    // low 14 bits are Fragment number (usually be 0)
    u8 addr3[6]; // the third address in packet
};

struct sniffer_buf{
    struct RxControl rx_ctrl;
    u8 buf[36]; // head of ieee80211 packet
    u16 cnt; // number count of packet
    struct LenSeq lenseq[1]; //length of packet
};

struct sniffer_buf2{
    struct RxControl rx_ctrl;
    u8 buf[112];
    u16 cnt;
    u16 len; //length of packet
};

typedef    struct    GNU_PACKED {
    u16        Ver:2;                // Protocol version
    u16        Type:2;                // MSDU type
    u16        SubType:4;            // MSDU subtype
    u16        ToDs:1;                // To DS indication
    u16        FrDs:1;                // From DS indication
    u16        MoreFrag:1;            // More fragment bit
    u16        Retry:1;            // Retry status bit
    u16        PwrMgmt:1;            // Power management bit
    u16        MoreData:1;            // More data bit
    u16        Wep:1;                // Wep data
    u16        Order:1;            // Strict order expected
} FRAME_CONTROL;



#define I_LINK_CONNECT_MAC_ADDR_LEN		6
typedef struct __I_LINK_CONNECT_WIFI_FRAME_HEAD_STR//�鲥������֡
{
	//unsigned char FrameControl[2];//֡����ͷ
	FRAME_CONTROL   FC;
	/*
Protocol Version��Э��汾����ͨ��Ϊ0��
Type�������򣩺�Subtype���������򣩣���ָͬ��֡�����ͣ�
To DS��������֡��BSS��DS���͵�֡��
From DS��������֡��DS��BSS���͵�֡��
More Frag������˵����֡���ֶε�������Ƿ���������֡��
Retry���ش��򣩣�����֡���ش�������STA���ø��������ش�֡��
Pwr Mgt�����������򣩣�1��STA����power_saveģʽ��0������activeģʽ��
More Data�����������򣩣�1�����ٻ���һ������֡Ҫ���͸�STA ��
Protected Frame�� 1��֡�岿�ְ�������Կ�״���������ݣ�����0��
Order������򣩣�1����֡�ֶδ��Ͳ����ϸ��ŷ�ʽ������0��
	*/
	unsigned short DurationId;//
	/*������֡������ȷ��֡����ռ���ŵ��೤ʱ�䣻����֡������������Ϊ��Power Save-Poll��֡�������ʾ��STA��������ݣ�AID, Association Indentification����*/
	unsigned char Addr1[I_LINK_CONNECT_MAC_ADDR_LEN];//����վ��ַ
	unsigned char Addr2[I_LINK_CONNECT_MAC_ADDR_LEN];//����Դ
	unsigned char Addr3[I_LINK_CONNECT_MAC_ADDR_LEN];//������,��ಥ/�鲥
	//unsigned char bssid[6];//�����������ʶ����
	unsigned short SeqControl;//֡�����
	/* Sequence Control�����п����򣩣��ɴ���MSDU��MAC Server Data Unit������MMSDU��MAC Management Server Data Unit����12λ���кţ�Sequence Number���ͱ�ʾMSDU��MMSDU��ÿһ��Ƭ�εı�ŵ�4λƬ�κ���ɣ�Fragment Number����*/
	unsigned char Addr4[I_LINK_CONNECT_MAC_ADDR_LEN];//��STA->APģʽ�²����ã�Ϊ��WDS�ȹ���ʹ��
	//unsigned char DataBody[2312];//0~2312�ֽڵ�����
	/*������Ϣ����֡������������ͬ����Ҫ��װ�����ϲ�����ݵ�Ԫ������Ϊ0~2312���ֽڣ������Ƴ���802.11֡��󳤶�Ϊ��2346���ֽڣ�*/
	unsigned long FrameCrcSum;//CRCУ��//	����32λѭ��������
	
	
}I_LINK_CONNECT_WIFI_FRAME_HEAD_STR, *P_I_LINK_CONNECT_WIFI_FRAME_HEAD_STR;

typedef struct __I_LINK_CONNECT_WIFI_FRAME_HEAD_SHORT_STR
{
	unsigned char dest[I_LINK_CONNECT_MAC_ADDR_LEN];//������,��ಥ/�鲥
	unsigned char bssid[I_LINK_CONNECT_MAC_ADDR_LEN];
}I_LINK_CONNECT_WIFI_FRAME_HEAD_SHORT_STR, *P_I_LINK_CONNECT_WIFI_FRAME_HEAD_SHORT_STR;


//----------------------------2.1��I_LINK_CONNECT_CMD_STA_SETTING = 0x1F��ϸ˵��--------------------------------//


#pragma pack()

void honyar_ilink_init(void);

#endif


