
#ifndef _HONYAR_ILINK_H_
#define _HONYAR_ILINK_H_

#include "c_types.h"


typedef struct __I_LINK_CONNECT_STA_SETTING_MULTICAST_STR
{
	unsigned char Sync[5];//同步头"11111",StepNo=I_LINK_CONNECT_STA_SETTING_STATUS_IN_SYNC,数据内容1
	unsigned char DataLen;//去掉同步头的所有数据长度,StepNo=I_LINK_CONNECT_STA_SETTING_STATUS_IN_DATA_LEN，数据内容22
	unsigned char Cmd;			//,StepNo=I_LINK_CONNECT_STA_SETTING_STATUS_IN_CMD,数据内容33,命令号Bit6为关闭条码验证标志(1、关闭验证、0、开启验证),bit5~bit0：快速设置为0x1F,I_LINK_CONNECT_CMD_TYPE
	
	unsigned char SsidLen;//ssid长度,StepNo=I_LINK_CONNECT_STA_SETTING_STATUS_IN_SSID_LEN,数据内容4
	unsigned char PwdLen;//PWD长度,StepNo=I_LINK_CONNECT_STA_SETTING_STATUS_IN_PWD_LEN,数据内容4
	unsigned char Ver;//加密方式,StepNo=I_LINK_CONNECT_STA_SETTING_STATUS_IN_SECMOD,数据内容4
	unsigned char Ssid[36];//Ssid,StepNo=I_LINK_CONNECT_STA_SETTING_STATUS_IN_SSID,数据内容44
	unsigned char ssid_frame[36];//ssid帧序号
	unsigned char Pwd[68];//PWD,StepNo=I_LINK_CONNECT_STA_SETTING_STATUS_IN_SUM,数据内容444
	unsigned char pwd_frame[68];//
	unsigned char SnLen;//SN长度,StepNo=I_LINK_CONNECT_STA_SETTING_STATUS_IN_SN_LEN,数据内容4444
	unsigned char Sn[36];//SN,StepNo=I_LINK_CONNECT_STA_SETTING_STATUS_IN_SN,数据内容4444
	unsigned char sn_frame[36];//
	unsigned char RespInfoFlag;//I_LINK_CONNECT_STA_SETTING_STATUS_IN_RESP_INFO_FLAG,数据内容44444
    unsigned char RespInfoIp[4];//SN,StepNo=I_LINK_CONNECT_STA_SETTING_STATUS_IN_RESP_INFO,数据内容44444
    //(当RespIpByte1、RespIpByte2、RespIpByte3、RespIpByte4都未255时广播回复，RespPort为0时按照固定端口回复)
    unsigned char RespInfoIpLen;
    unsigned char RespInfoIpFrame[4];
    unsigned char RespRandomId[2];//MagicId，2随机数字节,低字节在前,I_LINK_CONNECT_STA_SETTING_STATUS_IN_RESP_INFO,数据内容44444
    unsigned char RespRandomIdLen;
    unsigned char RespRandomIdFrame[2];
    //**(参数 RespIpByte1:ip第1个字节,RespIpByte2:ip第2个字节,RespIpByte3:ip第3个字节,RespIpByte4:ip第4个字节,RespRandomId,回复随机数据串)
    // ** (当RespIpByte1、RespIpByte2、RespIpByte3、RespIpByte4都未255时广播回复，固定端口为20087回复)
    // ** (回送信息按照XML搜索回复信息进行回送，里面含随机数字段)
	unsigned char ChkSum;//和校验,数据区取和舍去高字节,StepNo=126,数据内容9
}I_LINK_CONNECT_STA_SETTING_MULTICAST_STR, *P_I_LINK_CONNECT_STA_SETTING_MULTICAST_STR;

#pragma pack(1)
//----------------------------1.基础数据定义--------------------------------//
typedef enum 
{
		I_LINK_CONNECT_CMD_SET_IO_OUT = 0x04, //0xF4为设置IO（value为GPIO号,value2为设置状态0or1）
		I_LINK_CONNECT_CMD_WRITE_PASSWD = 0x07,//设置密码
		I_LINK_CONNECT_CMD_INIT_PARAM = 0x0E,//参数初始化
		I_LINK_CONNECT_CMD_MODEL_SET = 0x0D,//模式设置（value为模式类型,value2为模式进入（连接数）or退出）>0表示当前连接数;=0表示无连接或者退出;
		I_LINK_CONNECT_CMD_SN_SET = 0x12,//SN设置设置(后面22字节都是)
		I_LINK_CONNECT_CMD_SSID = 0x13,//AP名称设置（value为读取或者设置(0,1),SSID名称总长度，info为SSID名称头)
		I_LINK_CONNECT_CMD_MAC = 0x14,//MAC配置（value为读取或者设置(0,1)，value2为网卡号，ETH0为0，ETH1为1，ath0为10,ath1为11MAC地址)
		I_LINK_CONNECT_CMD_UPDATE = 0x15,//升级（value为类型(0:smb;1:透传模块),value2为升级类型(1:tftp,0:ftp)，info[0~3]为服务器地址;info[4~9]:user;info[10~15]:pwd;reserved1:port)
		I_LINK_CONNECT_CMD_TEST_MOD = 0x16,//产检或者测试模式/实际功能无效（value为进入退出状态,0x01进入,0x00,退出)
		I_LINK_CONNECT_CMD_WIFI_ON = 0x17,//WIFI开控制（value为分钟,value2为秒)
		I_LINK_CONNECT_CMD_WIFI_OFF = 0x18,//WIFI关控制（value为分钟,value2为秒)
		I_LINK_CONNECT_CMD_IR_BYTE_SEND = 0x19,//红外控制（发送数据的方式进行控制）(后面22字节都是)
		I_LINK_CONNECT_CMD_UART_BYTE_SEND = 0x1A,//串口数据发送(后面22字节都是)
		I_LINK_CONNECT_CMD_SETTING_AP_ON = 0x1B,//进入AP配置模式
		I_LINK_CONNECT_CMD_AP_ON = 0x1C,//进入AP模式
		I_LINK_CONNECT_CMD_STA_ON = 0x1D,//进入STA模式
		I_LINK_CONNECT_CMD_MON_ON = 0x1E,//进入MONITOR模式
		I_LINK_CONNECT_CMD_STA_SETTING_SN = 0x1F,//一键设置：需要校验SN，STA参数配置（value为是否立即生效标志)
		I_LINK_CONNECT_CMD_STA_SETTING_NON_SN = 0x5F,//一键设置：不需要校验SN，STA参数配置（value为是否立即生效标志)
		
		
}I_LINK_CONNECT_CMD_TYPE;


typedef struct __I_LINK_CONNECT_DATA_STR//数据区
{
	unsigned char Resv[4];//
	unsigned char info[192];//StepNo从111开始
}I_LINK_CONNECT_DATA_STR, *P_I_LINK_CONNECT_MON_DATA_STR;

typedef struct __I_LINK_CONNECT_BASE_STR
{
	unsigned char Sync[5];//同步头"11111",StepNo=I_LINK_CONNECT_STA_SETTING_STATUS_IN_SYNC,数据内容1
	unsigned char DataLen;//去掉同步头的所有数据长度,StepNo=I_LINK_CONNECT_STA_SETTING_STATUS_IN_DATA_LEN，数据内容22
	unsigned char Cmd;			//,StepNo=I_LINK_CONNECT_STA_SETTING_STATUS_IN_CMD,数据内容33,命令号Bit7为是否验证条码的标志,Bit6为是否立即生效的标志,bit5~bit0：快速设置为0x1F,I_LINK_CONNECT_CMD_TYPE
	unsigned char ResvStart;//
	I_LINK_CONNECT_DATA_STR DataStr;//StepNo从102开始
	unsigned char ChkSum;//和校验,数据区取和舍去高字节,StepNo=199,数据内容9
	unsigned char ResvEnd[3];//
}I_LINK_CONNECT_MON_BASE_STR, *P_I_LINK_CONNECT_MON_BASE_STR;



typedef struct __I_LINK_CONNECT_MULTICAST_BASE_STR
{
 	unsigned char Multicastsync;//同步头0x239
	unsigned char StepNo;//大步骤号
	unsigned char FrameNo;//帧序号
	unsigned char Data;//数据
}I_LINK_CONNECT_MULTICAST_BASE_STR, *P_I_LINK_CONNECT_MULTICAST_BASE_STR;

//----------------------------1.基础数据定义--------------------------------//





//----------------------------2.1、I_LINK_CONNECT_CMD_STA_SETTING = 0x1F详细说明--------------------------------//
//I_LINK_CONNECT_CMD_STA_SETTING = 0x1F,//STA参数配置（value为是否立即生效标志)
typedef enum 
{
		I_LINK_CONNECT_STA_SETTING_STATUS_START = 0,//初始状态
		I_LINK_CONNECT_STA_SETTING_STATUS_IN_SYNC = 101,//正在接收同步头
		I_LINK_CONNECT_STA_SETTING_STATUS_IN_DATA_LEN = 102,//正在总长度
		I_LINK_CONNECT_STA_SETTING_STATUS_IN_CMD = 103,//正在接收命令字
		I_LINK_CONNECT_STA_SETTING_STATUS_RECVD_CMD = 104,//已接收完命令字
		I_LINK_CONNECT_STA_SETTING_STATUS_IN_SSID_LEN = 111,//正在接收SSID长度
		I_LINK_CONNECT_STA_SETTING_STATUS_IN_PWD_LEN = 112,//正在接收PWD密码长度
		I_LINK_CONNECT_STA_SETTING_STATUS_IN_VER = 113,//正在接收VER
		I_LINK_CONNECT_STA_SETTING_STATUS_IN_SSID = 114,//正在接收SSID
		I_LINK_CONNECT_STA_SETTING_STATUS_IN_PWD = 115,//正在接收PWD密码
		I_LINK_CONNECT_STA_SETTING_STATUS_IN_SN_LEN = 116,//正在接收SN
		I_LINK_CONNECT_STA_SETTING_STATUS_IN_SN = 117,//正在接收SN
		I_LINK_CONNECT_STA_SETTING_STATUS_IN_RESP_INFO_FLAG = 118,//正在接收回复使能
		I_LINK_CONNECT_STA_SETTING_STATUS_RESP_INFO = 119,//正在接收回复信息
		I_LINK_CONNECT_STA_SETTING_STATUS_RESP_RANDOM = 120,
		I_LINK_CONNECT_STA_SETTING_STATUS_IN_SUM = 126,//正在接收和校验
		I_LINK_CONNECT_STA_SETTING_STATUS_END = 200,//接收完成

		
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
typedef struct __I_LINK_CONNECT_WIFI_FRAME_HEAD_STR//组播包数据帧
{
	//unsigned char FrameControl[2];//帧控制头
	FRAME_CONTROL   FC;
	/*
Protocol Version（协议版本）：通常为0；
Type（类型域）和Subtype（子类型域）：共同指出帧的类型；
To DS：表明该帧是BSS向DS发送的帧；
From DS：表明该帧是DS向BSS发送的帧；
More Frag：用于说明长帧被分段的情况，是否还有其它的帧；
Retry（重传域）：用于帧的重传，接收STA利用该域消除重传帧；
Pwr Mgt（能量管理域）：1：STA处于power_save模式；0：处于active模式；
More Data（更多数据域）：1：至少还有一个数据帧要发送给STA ；
Protected Frame： 1：帧体部分包含被密钥套处理过的数据；否则：0；
Order（序号域）：1：长帧分段传送采用严格编号方式；否则：0。
	*/
	unsigned short DurationId;//
	/*表明该帧和它的确认帧将会占用信道多长时间；对于帧控制域子类型为：Power Save-Poll的帧，该域表示了STA的连接身份（AID, Association Indentification）。*/
	unsigned char Addr1[I_LINK_CONNECT_MAC_ADDR_LEN];//工作站地址
	unsigned char Addr2[I_LINK_CONNECT_MAC_ADDR_LEN];//发送源
	unsigned char Addr3[I_LINK_CONNECT_MAC_ADDR_LEN];//接收者,如多播/组播
	//unsigned char bssid[6];//基本服务组合识别码
	unsigned short SeqControl;//帧序控制
	/* Sequence Control（序列控制域）：由代表MSDU（MAC Server Data Unit）或者MMSDU（MAC Management Server Data Unit）的12位序列号（Sequence Number）和表示MSDU和MMSDU的每一个片段的编号的4位片段号组成（Fragment Number）。*/
	unsigned char Addr4[I_LINK_CONNECT_MAC_ADDR_LEN];//在STA->AP模式下不适用，为了WDS等功能使用
	//unsigned char DataBody[2312];//0~2312字节的数据
	/*包含信息根据帧的类型有所不同，主要封装的是上层的数据单元，长度为0~2312个字节，可以推出，802.11帧最大长度为：2346个字节；*/
	unsigned long FrameCrcSum;//CRC校验//	包含32位循环冗余码
	
	
}I_LINK_CONNECT_WIFI_FRAME_HEAD_STR, *P_I_LINK_CONNECT_WIFI_FRAME_HEAD_STR;

typedef struct __I_LINK_CONNECT_WIFI_FRAME_HEAD_SHORT_STR
{
	unsigned char dest[I_LINK_CONNECT_MAC_ADDR_LEN];//接收者,如多播/组播
	unsigned char bssid[I_LINK_CONNECT_MAC_ADDR_LEN];
}I_LINK_CONNECT_WIFI_FRAME_HEAD_SHORT_STR, *P_I_LINK_CONNECT_WIFI_FRAME_HEAD_SHORT_STR;


//----------------------------2.1、I_LINK_CONNECT_CMD_STA_SETTING = 0x1F详细说明--------------------------------//


#pragma pack()

typedef void (*honyar_ilink_success_cb_t)(void);

void honyar_ilink_regist_success_cb(honyar_ilink_success_cb_t func);

void honyar_ilink_init(void);

#endif


