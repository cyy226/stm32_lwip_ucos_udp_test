/*************************************************************
�ɶ���ջ����������޹�˾   ��Ȩ����


�ļ���:   
��  ��:         
��  ��:
�޶���¼:

**************************************************************/


#ifndef __COMM_H__
#define __COMM_H__
#include "devUartProc.h"


#define BYTE unsigned char
#define WORD unsigned short
#define DWORD unsigned long
#define uint  unsigned int

#define INNER_WDOG    // �ڲ����Ź�
#define CDMA_SRVMODE // CDMA ������ģʽ

//Sys Address Define
#define MAINAPPSTART      0x0
#define BAKAPPSTART       0x0001FC00
#define SYSPARAMSTART     0x0003FC00
#define APPMAXLENGTH      0x1FC00   //127*1024

#define DEBUG_DISABLE   0
#define DEBUG_ENABLE    1

//������
#define BAUDRATE_150        0
#define BAUDRATE_300        1
#define BAUDRATE_600        2
#define BAUDRATE_1200       3
#define BAUDRATE_2400       4
#define BAUDRATE_4800       5
#define BAUDRATE_9600       6
#define BAUDRATE_19200      7
#define BAUDRATE_38400      8
#define BAUDRATE_57600      9
#define BAUDRATE_115200     10

//У��
#define CHECK_ODD      1     // ��У��
#define CHECK_EVEN     2     // żУ��
#define CHECK_NONE     3     // ��У��

#define GSM_MG323    0
#define CDMA_MC323   1

//#define MODULE_MG323
//#define MODULE_MC323
//#define MODULE_GTM900 

//#define RT_DS1302   // HT1381
#undef  RT_DS1302 

#define BYTE  unsigned char
#define WORD  unsigned short
#define DWORD unsigned long


#define FALSE     0
#define TRUE      1
#define OK        2
#define TIMEOUT   3
#define LINKDOWN  4  // ��·�Ͽ�
#define NOMETER   5  // û�м�⵽���

//IP��ַ��ȡ��ʽ
#define IPMODE_FIXED   1
#define IPMODE_DHCP    2
#define IPMODE_PPPOE   3

#define DEVFRAME    1   // �豸֡
#define METERFRAME  2  // ���֡
#define MODBUSFRAME 3  //mod ���


#define SENDBUFLEN  100

//���ŵ������
#define HTTP_ENABLE        1   
#define SOCKET_ENABLE      1

#define RETRY_COUNT       3


typedef void (*tFun)();
typedef struct 
{
    WORD Time;   //MS
    tFun Fun;
}LOGICTIMER;



// ϵͳ����ģʽ
//#define SYSMODE_CONTRON    0
//#define SYSMODE_CHECK      1
//#define SYSMODE_TEST       2

#define COMMMODE_EARTHNET  1
#define COMMMODE_CDMA1X    2

#define NETMODE_TCP     1
#define NETMODE_UDP     2

#define WORKMODE_CLIENT  1
#define WORKMODE_SERVER  2
#define WORKMODE_MIX     3


#define SP_MOBLENUM       0  // ���� 12    �㱨�ֻ�����
#define SP_DAYDELTA       12 // ���� 2    �㱨ʱ����
#define SP_NEXTREPDAY     14 // ���� 2   �´λ㱨ʱ��
#define SP_REPTIME        16 // ���� 1    �㱨ʱ��
#define SP_DEVICETAP      17 // ���� 1    �Ƿ�͵��

#define SP_DEVICECTL      18 // ���� 1    �Ƿ�ʱ��������
#define SP_ENABLEHTTP     19 // ����1     �Ƿ�ʹ��HTTP
#define SP_HOSTNAME       20 // ����32    ����������
#define SP_SELFNUM        52 // ����12    ��������  
#define SP_LASTERROR      64  //����1     ������
#define SP_EMTYPE         65  //����1     �������

/*heart server*/
#define SP_SERVERIP       66  // ����4    ������IP��ַ
/*def to 7080*/
#define SP_SERVERPORT     70  // ����2    �������˿�

/*local ip*/
#define SP_LOCALIP        72  // ����4     ����IP
#define SP_LOCALPORT      76  // ����2     ���ض˿�

/*no need*/
#define SP_SLEEPTIME      78  // ����4     ����ʱ�� : ��

/*need to */
#define SP_ENABLESOCKET   88  // ����1    �Ƿ�ʹ��SOCKET-TCP
#define SP_COMMMODE       89  // 1     ͨѶģʽ      1: ��̫��  2: ����(CDMA1X)
/*devId*/
#define SP_DEVADDR        90  // ����5  �豸��ַ 
#define SP_NETMODE        95  // 1     ����ͨѶģʽ   1: TCP  2: UDP 3:TCP-DHCP
/*pwd*/
#define SP_PASSWORD       96  // ����6  ����
/*fix to misc*/
#define SP_WORKMODE       102 // 1  ����ģ�

/*no need*/
#define SP_REPENABLE      103 // 1  �Ƿ������Զ��ϱ�

/*�� def to 3600*/
#define SP_HEARTTIME      104 // 2  ����������
/*no need*/
#define SP_READTIME       106 // 2   ����ʱ����
#define SP_METERCOUNT     108 // 1  �������
//#define SP_USEDHCP        109 // 1  ip��ַ�̶���ʹ��DHCP
#define SP_MACADDR        110 //6  MAC��ַ
#define SP_MODTYPE        116 // 1 ͨ��ģ������

//485���ڲ���
#define SP_BAUDRATE       117  // ����1    ����ڲ�����  
#define SP_DATABIT        118  // ����1    ����λ      
#define SP_CHECKBIT       119  // ����1    У��λ      //01:��У��;02:żУ��;03��У��
#define SP_STOPBIT        120  // ����1    ֹͣλ   

#define SP_DEVIPMODE      121 // ����1     �ɼ���IP��ַģʽ  1: ��̬�� 2: DHCP  3: PPPOE

//CDMA �����û���������
#define SP_NETUSERNAME    122 // ����32    �����û���
#define SP_NETPASSWORD    154 // ����32    ��������   -- 224
#define SP_DEBUGMSG       186// 1  �Ƿ��ӡ������Ϣ


//����ɼ�ʱ��
#define SP_TIMERCOUNT      187
#define SP_TIMER1_HOUR     188
#define SP_TIMER1_MINUTE   189
#define SP_TIMER2_HOUR     190
#define SP_TIMER2_MINUTE   191
#define SP_TIMER3_HOUR     192
#define SP_TIMER3_MINUTE   193

#define SP_RECTIMEOUT      194  // ����2 ���ճ�ʱ

//У�����
#define SP_RNKIA           196  //����9*2
#define SP_RNKIB           214 //����9*2
#define SP_RNKU            232  //����9*2
#define SP_RNKAP           250 //����9*2
#define SP_RNKBP           268 //����9*2
        
#define SP_RNSYSCON        286 // 2*9  
#define SP_RNIARMSOS       304 // 2*9
#define SP_RNIBRMSOS       322 // 2*9
#define SP_RNIBGAIN        340 // 2*9 
#define SP_RNGPQA          358 // 2*9
#define SP_RNGPQB          376 // 2*9 
#define SP_RNAPOSA         394 // 2*9  
#define SP_RNAPOSB         412 // 2*9 
#define SP_RNPHSA          430 // 2*9
#define SP_RNPHSB          448 // 2*9 
#define SP_RNRPOSA         466 // 2*9
#define SP_RNRPOSB         484 // 2*9 
#define SP_RNQPHSCAL       502 //2*9 
#define SP_RNHFCONST       520 //2*9 
#define SP_RNPSTART        538 //2*9
#define SP_RNQSTART        556 //2*9

#define SP_PEOIF_COUNT     574 // 1*9,�й����ܼĴ���������� 

#define SP_METERADDR1       583//1*6 ����ַ1,
#define SP_METERADDR2       589//1*6 ����ַ2,
#define SP_METERADDR3       595//1*6 ����ַ3,
#define SP_METERADDR4       601//1*6 ����ַ4,
#define SP_METERADDR5       607//1*6 ����ַ5,
#define SP_METERADDR6       613//1*6 ����ַ6,
#define SP_RNIE             619 // 2*1 ʹ���ж�,
#define SP_FLOWRATE         621 // 4 ��������

// �����������
#define SP_SW_VERSION       625  // 2 ������汾  
#define SP_SW_LENGTH        627  // 4 ���������
#define SP_SW_CURID         631  // 2 ��ǰ���ݰ�ID
#define SP_SW_UPDATE        633  // 1 ���������־  0: ������  1 : �ɹ�������ɣ���Ҫ����

#define SYSPARAM_COUNT      636  // 4�ı���

#if 0
#define DELTA_DOWN_LIMIT  1
#define DELTA_UP_LIMIT    10

#define TMP_DOWN_LIMIT   20
#define TMP_UP_LIMIT     45

#define HUM_DOWN_LIMIT   50
#define HUM_UP_LIMIT     95

#define BAT_DOWN_LIMIT   40
#define BAT_UP_LIMIT     60

#define DIS_DOWN_LIMIT   2
#define DIS_UP_LIMIT     60

#define LEN_DOWN_LIMIT   1
#define LEN_UP_LIMIT     60

#define AIR_DOWN_LIMIT   20
#define AIR_UP_LIMIT     40

#define TC_DOWN_LIMIT   0
#define TC_UP_LIMIT     30

#define HOT_DOWN_LIMIT  5
#define HOT_UP_LIMIT    20



//����Ĭ��ֵ
#define DEF_COOLSTART    25
#define DEF_COOLDELTA    3
#define DEF_HOTSTART     10
#define DEF_HOTDELTA     5
#define DEF_AIRCONSTART   29
#define DEF_AIRCONDELTA   3
#endif

#define DEF_ENABLEHTTP        HTTP_ENABLE
#define DEF_ENABLESOCKET      SOCKET_ENABLE 
#define DEF_HEARTTIME         3600

#define DEF_MODULE       CDMA_MC323
//#define DEF_SRVPORT       7080       // ������Ĭ�϶˿�
//#define DEF_LOCPORT       9000       // ���ؼ���Ĭ�϶˿�
#define WORKMODE_NORMAL  0
#define WORKMODE_TRANC   0xFF

//485����Ĭ�ϲ���
#define DEF_BAUDRATE     BAUDRATE_1200  
#define DEF_DATABIT      8
#define DEF_STOPBIT      1
#define DEF_CHECKBIT     CHECK_EVEN   // żУ��

#define DEF_RECTIMEOUT   120   // 2����

//RN8209����ϵ��Ĭ��ֵ
#define DEF_KI_VALUE 472  //����ͨ������ֵ
#define DEF_KU_VALUE 4655  //��ѹͨ������ֵ
#define DEF_KP_VALUE 720  //����ͨ������ֵ

#if 1
#define LED_ON   0
#define LED_OFF  1
#define EM_TYPWCOUNT 3

//�����붨��
#define ERR_COUNT      16
#define ERR_NULL       0
#define ERR_EMFAIL     1
#define ERR_MODULEFAIL 2
#define ERR_GSMFAIL    3
#define ERR_GPRSFAIL   4
#define ERR_TCPIPFAIL  5
#define ERR_RTCFAIL    6    // ʵʱ��
#define ERR_HEARTFAIL  7
#define ERR_CONNSRVFAIL 8
#define ERR_REGSRVFAIL  9
#define ERR_COMMFAIL    10
#define ERR_NOMETER    11    // û�е�����������ݷ���
#define ERR_CHECK      12    // У�����
#define ERR_NOTMYDATA  13   // �Ǳ�������
#define ERR_INVALIDDATA 14  // �Ƿ�������������
#define ERR_LINKDOWN    15   // ��·�쳣�Ͽ�

#define SET_ALARM(Alm)     SysAlarm |= Alm
#define CLEAR_ALARM(Alm)   SysAlarm &= ~Alm
#define GET_ALARM(Alm)     (SysAlarm & Alm)
#endif

//����ģ��
#define MOD_POWERON()    GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_7,0)
#define MOD_POWEROFF()   GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_7,GPIO_PIN_7)
#define MOD_RESETHIGH()  GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_6,0)
#define MOD_RESETLOW()   GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_6,GPIO_PIN_6)
#define MOD_PDHIGH()     GPIOPinWrite(GPIO_PORTC_BASE,GPIO_PIN_5,0) 
#define MOD_PDLOW()      GPIOPinWrite(GPIO_PORTC_BASE,GPIO_PIN_5,GPIO_PIN_5) 


extern const DWORD BaudRateTable[];

BYTE HexToBcd(BYTE Hex);
BYTE BcdToHex(BYTE Bh, BYTE Bl);
BYTE mcmp(BYTE *Des,BYTE *Src,BYTE Len);
void mcpy(BYTE *Des,BYTE *Src, WORD Len);

BYTE mcmp2(BYTE *Des,BYTE Src,BYTE Len);
BYTE mcmp3(BYTE *Des,BYTE *Src,BYTE Len,BYTE bit);
void mset(BYTE *Des, BYTE Data, BYTE Len);
BYTE StrLen(const char *Str);
void DebugMsg(void);
void DebugWord(WORD Num);

void Init_WDog(void);
void WDog(void);
void Sleep (WORD ms);
void DebugStr(const char * Msg) ;
void SetLastError(BYTE Err);
void ALM_SW(void);
void DebugChar(BYTE ch);
void IpToStr(BYTE *IpAddr);
BYTE StrToIpAdd(BYTE *IpAddr);

WORD DateToDay(void);
void PortToStr(BYTE *Port);
WORD DateToDay(void);
void DayToDate(WORD day);

BYTE StrToWord(BYTE *Str, BYTE Len, WORD *Dat);

BYTE StrToByte(BYTE *Str, BYTE Len, BYTE *Dat);
BYTE CheckValidIpAddr(void);
BYTE StrToPort(BYTE *Port);
BYTE CheckValidPort(void);
uint MeanValue(uint *pBuf ,BYTE len);


#endif
