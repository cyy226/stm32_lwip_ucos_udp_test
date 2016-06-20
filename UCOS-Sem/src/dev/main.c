/********************************************************************************************************
*                                              �������ȼ�����ջ                                         *
********************************************************************************************************/
#include "includes.h"
#include <update.h>
#include <dev.h>
#include <comm.h>
#include <hw_types.h>
#include <sysctl.h>
#include <lwiplib.h>
#include <hw_ints.h>
#include <systick.h>
#include <flash.h>
#include <interrupt.h>
#include <spi.h>
#include <ethernet.h>
#include <Watchdog.h>
#include "rn8209x.h"
#include "devTimer.h"
#include "ip_addr.h"
#include "API.h"
#include "main.h"
#include "WlModule.h"
#include "devI2c.h"


unsigned int  TheSysClock =0;

const char* DefHost = "www.ecgcp.com";

//�������ʱ���޸����������  -- ����汾��Startup.s ��110��
BYTE DevVersion[7]           = {0};        // ��ʽ 001.100        // �豸�汾  7�ֽ�
const BYTE DevRelTime[]      = {0,48,15,26,02,0xDD,0x07};    // �������ʱ��  2012��  7�ֽ�
//BYTE DevRelTime[10] ;
const BYTE *DevIndex         = "CDMA-R01";                    // �豸���  8�ֽ�

//#define SWVERSION   "V1.9T02"   // �豸�汾,7�ֽ�
//#define DEVICENO    "00000002"  // �豸���,8�ֽ�

BYTE PowerStr[3][30]   = {0};
BYTE IpAddrStr[16]  = {0};
BYTE PortStr[6]     = {0};


BYTE NoCenter = 0;          // û�лر�����
BYTE NeedSleep = 0;        // �Ƿ�˯��   �ɼ������������紦������״̬
BYTE NeedReConnect  = 0;  // ��Ҫ���»㱨������
BYTE NeedConnSrv  = 0 ;   // �Ƿ����ӵ�������

BYTE DeviceStatus = 0;
//BYTE NeedReboot = 0;




#define SYSTICKHZ               100
#define SYSTICKMS               (1000 / SYSTICKHZ)
#define SYSTICKUS               (1000000 / SYSTICKHZ)
#define SYSTICKNS               (1000000000 / SYSTICKHZ)

#if 0
#define My_Mac_ID 	{0X00,0x14,0x97,0x0F,0x1D,0xE3}  //�洢��̫���������������ַ,��MAC��ַ
#define IP_MARK_ID 	{255,255,255,0} 		         //255.255.255.0,��������
#define MY_IP_ID       	{192,168,1,25}                   //��̫��ͨ�ŵ�IP��ַ
#define MY_GATEWAY_ID   {192,168,1,254}                  //��̫��ͨ�ŵ����ص�ַ

static const unsigned char MACAddress[] = My_Mac_ID;
unsigned char IPAddress[] = MY_IP_ID;
unsigned char NetMaskAddr[] = IP_MARK_ID;
unsigned char GwWayAddr[] = MY_GATEWAY_ID;
#endif

uint debugFlag = 0x0;
tPubVal gPubVal;

extern void setRandMac(void);
extern void tcpServerInit ( void );

/*******************************************************************************
˵�� os ������,�����ȼ�������main.h�У����������ط�����
*******************************************************************************/
/*tcp server ����Ϣ������*/
#define STK_SIZE_MAIN_HANDLE    400
//#define PRIO_MAIN_HANDLE        2
static  OS_STK  stkTaskMainHandle[STK_SIZE_MAIN_HANDLE];


/*SHELL��������*/
#define STK_SIZE_SHELL   300
//#define PRIO_SHELL        3 
static  OS_STK  stkTaskShell[STK_SIZE_SHELL];


/*hart report����*/
#define STK_SIZE_HEART_REP    300
//#define PRIO_HEART_REP        8
static  OS_STK  stkTaskHearReq[STK_SIZE_HEART_REP];


/*���ڴ�������*/
#define STK_SIZE_PRID    300
//#define PRIO_PRID        4
static  OS_STK  stkTaskPrid[STK_SIZE_PRID];


/*WATCHDOG ��������*/
#define STK_SIZE_WATCH0   200
//#define PRIO_WATCH0        11
static  OS_STK  stkTaskWatch0[STK_SIZE_WATCH0];


/*rs485��������*/
#define STK_SIZE_RS485    300
//#define PRIO_RS485        5
static  OS_STK  stkTaskRS485[STK_SIZE_RS485];

/*GSM��������*/
#define STK_SIZE_GSMMSG    200
//#define PRIO_GSMMSG       
static  OS_STK  stkTaskGSMMSG[STK_SIZE_GSMMSG];


/*GSM��������*/
#define STK_SIZE_GSM    300
//#define PRIO_GSM        7
static  OS_STK  stkTaskGSM[STK_SIZE_GSM];

/*�ж�RN8209�ж�������������*/
//#define STK_SIZE_RNISR    200
//#define PRIO_RNISR       9
//static  OS_STK  stkTaskRnISR[STK_SIZE_RNISR];

//RN isr�ź���
OS_EVENT * RNSemIsr;


void taskHeartRep ( void *pParam );
void taskShellCmd ( void *pParam );
//void taskPriod ( void *pParam );
void taskUartHandle ( void * pdata );
void taskRS485Rx ( void *param );
//
void GetVersion()
{
    static DWORD ver = 0;
    ver = *(DWORD *)ADDR_BIN_VER;
    DevVersion[0] = '0';
    DevVersion[1] = '0';
    DevVersion[2] = (BYTE)(ver>>16) + 0x30;
    DevVersion[3] = '.';
    DevVersion[4] = (BYTE)(ver) + 0x30;
    DevVersion[5] = '0';
    DevVersion[6] = '0';
}

void SetDefParam(void)
{
    static BYTE i,len;

    //��ʱ�㱨�ֻ�����
    for (i=0;i<11;i++)   
    {
        SysParam[SP_MOBLENUM+i] = '9';
    }
    SysParam[SP_MOBLENUM+11] = 0;
    //NoCenter = 0;

    //������
    for (i=SP_SELFNUM;i<SP_SELFNUM+11;i++)   
    {
        SysParam[i] = '9';
    }
    SysParam[SP_SELFNUM+11] = 0;

    *(WORD *)&SysParam[SP_RECTIMEOUT] = DEF_RECTIMEOUT;       
    SysParam[SP_SW_VERSION]   = 0;
    SysParam[SP_SW_VERSION+1] = 0;
    *(DWORD *)&SysParam[SP_SW_LENGTH] = 0;
    *(WORD *)&SysParam[SP_SW_CURID] = 0;
    SysParam[SP_SW_UPDATE] = FALSE;
   
         
    //�Ƿ�͵�� 
    SysParam[SP_DEVICETAP] = 0;

    //�Ƿ�ѭ�������ⲿ�豸    
    SysParam[SP_DEVICECTL] = 0;
   
    //�Ƿ�����HTTP    
    SysParam[SP_ENABLEHTTP] = DEF_ENABLEHTTP;

    //�Ƿ�����TCP   
    SysParam[SP_ENABLESOCKET] = DEF_ENABLESOCKET;
    
    SysParam[SP_COMMMODE] = COMMMODE_CDMA1X;
    SysParam[SP_NETMODE] = NETMODE_TCP;
    SysParam[SP_WORKMODE] = WORKMODE_MIX;
    
    //����������
    len = StrLen(DefHost);
    mcpy(&SysParam[SP_HOSTNAME],(BYTE *)DefHost,len);
    
    //������   
    SysParam[SP_LASTERROR] = ERR_NULL;

    //�������
    SysParam[SP_EMTYPE]= 0;
    
    // �Ƿ������Զ��ϱ�
    SysParam[SP_REPENABLE] = 1;
    
    //����ʱ����    
    *(WORD *)&SysParam[SP_READTIME] = 1000;
    
    // �������
    SysParam[SP_METERCOUNT] = 3;
        
    // IP Mode 
    SysParam[SP_DEVIPMODE] = IPMODE_DHCP;

    //���ڲ���
    SysParam[SP_BAUDRATE] = DEF_BAUDRATE;
    SysParam[SP_DATABIT]  = DEF_DATABIT;
    SysParam[SP_CHECKBIT] = DEF_CHECKBIT;
    SysParam[SP_STOPBIT]  = DEF_STOPBIT;

    SysParam[SP_TIMERCOUNT] = 0;
    SysParam[SP_TIMER1_HOUR] = 0;
    SysParam[SP_TIMER1_MINUTE] = 0;
    SysParam[SP_TIMER2_HOUR] = 0;
    SysParam[SP_TIMER2_MINUTE] = 0;
    SysParam[SP_TIMER3_HOUR] = 0;
    SysParam[SP_TIMER3_MINUTE] = 0;

    /*
    *(WORD *)&SysParam[SP_CMDTIMER] = 0;
    SysParam[SP_CMDDIS] = 10;
    SysParam[SP_RETRYCNT] = 0;

    *(WORD *)&SysParam[SP_DATALENGTH] = 200;

    SysParam[SP_ENABLE_MINUTE] = DISABLE;
    SysParam[SP_MINUTE_MINUTE] = 0;

    SysParam[SP_ENABLE_HOUR] = DISABLE;
    SysParam[SP_HOUR_HOUR]   = 0;
    SysParam[SP_HOUR_MINUTE] = 0;

    SysParam[SP_ENABLE_DAY] = DISABLE;
    SysParam[SP_DAY_DAY]    = 0;
    SysParam[SP_DAY_HOUR]   = 0;
    SysParam[SP_DAY_MINUTE] = 0;

    SysParam[SP_ENABLE_MONTH] = DISABLE;
    SysParam[SP_MONTH_MONTH]    = 0;
    SysParam[SP_MONTH_DAY]   = 0;
    SysParam[SP_MONTH_HOUR] = 0;
    SysParam[SP_MINTH_MINUTE] = 0;
    */
    

    SysParam[SP_MODTYPE] = DEF_MODULE;

    *(WORD *)&SysParam[SP_RECTIMEOUT] = DEF_RECTIMEOUT;
   
    //�����û���������       
    mset((BYTE *)&SysParam[SP_NETUSERNAME],0,32);
    mcpy((BYTE *)&SysParam[SP_NETUSERNAME],"card",4);
       
    mset((BYTE *)&SysParam[SP_NETPASSWORD],0,32);
    mcpy((BYTE *)&SysParam[SP_NETPASSWORD],"card",4);
    
    //��ӡ������Ϣ   
    SysParam[SP_DEBUGMSG] = DEBUG_DISABLE;

    SysParam[SP_SW_VERSION]   = 0;
    SysParam[SP_SW_VERSION+1] = 0;

    *(DWORD *)&SysParam[SP_SW_LENGTH] = 0;
    *(WORD *)&SysParam[SP_SW_CURID] = 0;
    SysParam[SP_SW_UPDATE] = FALSE;

    *(DWORD *)&SysParam[SP_FLOWRATE] = 0;

}
void GetSysParam(void)
{
    static WORD i,j,len;
    static WORD Day;
    for (i=0;i<SYSPARAM_COUNT;i++)
    {
        SysParam[i] = *(BYTE *)(SYSPARAMSTART+i);
    }

    // ���Ż㱨�ֻ�����
    for (i=0;i<11;i++)   
    {
        if ((SysParam[SP_MOBLENUM+i] < '0') || (SysParam[SP_MOBLENUM+i] > '9'))
        {
            //NoCenter = 1;
            SysParam[SP_MOBLENUM+i] = '9';
        }
    }
    SysParam[SP_MOBLENUM+11] = 0;

    //������
    for (i=SP_SELFNUM;i<SP_SELFNUM+11;i++)   
    {
        if ((SysParam[i] < '0') || (SysParam[i] > '9'))
        {
            SysParam[i] = '9';
        }
    }
    SysParam[SP_SELFNUM+11] = 0;
    
    // �㱨ʱ����
    Day = *(WORD *)&SysParam[SP_DAYDELTA];
    if (Day > 9999)
    {
        Day = 0;
        *(WORD *)&SysParam[SP_DAYDELTA] = 0;
    }

    //�´λ㱨ʱ��
    Day = *(WORD *)&SysParam[SP_NEXTREPDAY];
    if (Day > 9999)
    {
        Day = 0;
        *(WORD *)&SysParam[SP_NEXTREPDAY] = 0;
    }

    //�㱨ʱ��
    if (SysParam[SP_REPTIME] > 23)
    {
        SysParam[SP_REPTIME] = 0;
    }

    //�Ƿ�͵�� 
    if (SysParam[SP_DEVICETAP] > 1)
    {
        SysParam[SP_DEVICETAP] = 0;
    }

    //�Ƿ�ѭ�������ⲿ�豸
    if (SysParam[SP_DEVICECTL] > 1)
    {
        SysParam[SP_DEVICECTL] = 0;
    }

    //�Ƿ�����HTTP
    if (SysParam[SP_ENABLEHTTP] > 1)
    {
        SysParam[SP_ENABLEHTTP] = DEF_ENABLEHTTP;
    }


    //�Ƿ�����TCP
    if (SysParam[SP_ENABLESOCKET] > 1)
    {
        SysParam[SP_ENABLESOCKET] = DEF_ENABLESOCKET;
    }

    
    //����������
    for (i=SP_HOSTNAME;i<SP_HOSTNAME+5;i++)
    {
        if ((SysParam[i]==0) || (SysParam[i]==0xFF))
        {
            for (j=SP_HOSTNAME;j<SP_HOSTNAME+32;j++)
            {
                SysParam[j] = 0;
            }
            len = StrLen(DefHost);
            mcpy((BYTE *)&SysParam[SP_HOSTNAME],(BYTE *)DefHost,len);
            break;
        }
    }
    

    //������
    if (SysParam[SP_LASTERROR] >= ERR_COUNT)
    {
        SysParam[SP_LASTERROR] = ERR_NULL;
    }

    
    if (SysParam[SP_EMTYPE] >= EM_TYPWCOUNT)
    {
        SysParam[SP_EMTYPE]= 0;
    }

    #if 0
    // 117.89.241.99   ���Ų���ƽ̨   221.237.10.76
    SysParam[SP_SERVERIP]   = 117;
    SysParam[SP_SERVERIP+1] = 89;
    SysParam[SP_SERVERIP+2] = 241;
    SysParam[SP_SERVERIP+3] = 99;
    *(WORD *)&SysParam[SP_SERVERPORT] = 7905;
    #endif

    // �豸��ַ
    if (mcmp(&SysParam[SP_DEVADDR],"\xFF\xFF\xFF\xFF\xFF",5))
    {
        //Def add
        mset(&SysParam[SP_DEVADDR],0x99,3);
        SysParam[SP_DEVADDR+3] = 0x0F;    // 9999
        SysParam[SP_DEVADDR+4] = 0x27;
    }
    
    #if 0
    mcpy(&SysParam[SP_DEVADDR],"\x12\x31\x11\x11\x11",5);
    #endif
    
    mcpy(DevAddrTmp,&SysParam[SP_DEVADDR],5);
    
    
    // ����
    if (mcmp(&SysParam[SP_PASSWORD],"\xFF\xFF\xFF\xFF\xFF\xFF",6))
    {
        //Def Password  123456
        mcpy(&SysParam[SP_PASSWORD],"\x40\xE2\x01\x00\x00\x00",6);
    }
    
    

    //ͨѶģʽ
    if ((SysParam[SP_COMMMODE] < COMMMODE_EARTHNET) || (SysParam[SP_COMMMODE] > COMMMODE_CDMA1X))
    {
        SysParam[SP_COMMMODE] = COMMMODE_CDMA1X;
    }

    //����ģʽ
    if ((SysParam[SP_NETMODE] < NETMODE_TCP) || (SysParam[SP_NETMODE] > NETMODE_UDP))
    {
        SysParam[SP_NETMODE] = NETMODE_TCP;
    }

    //����ģʽ
    /*
    if ((SysParam[SP_WORKMODE] |= WORKMODE_NORMAL) && (SysParam[SP_WORKMODE] |= WORKMODE_TRANC))
    {
        SysParam[SP_WORKMODE] = WORKMODE_NORMAL;
    }
    */

    if ((SysParam[SP_WORKMODE] < WORKMODE_CLIENT) || (SysParam[SP_WORKMODE] > WORKMODE_MIX))
    {
        SysParam[SP_WORKMODE] = WORKMODE_MIX;
    }

    // �Ƿ������Զ��ϱ�
    if (SysParam[SP_REPENABLE] > 1)
    {
        SysParam[SP_REPENABLE] = 1;
    }

    //����ʱ����
    if (*(WORD *)&SysParam[SP_READTIME] == 0xFFFF)
    {
        *(WORD *)&SysParam[SP_READTIME] = 1000;
    }

    // �������
    if ((SysParam[SP_METERCOUNT] <1 ) || (SysParam[SP_METERCOUNT] > 9))
    {
        SysParam[SP_METERCOUNT] = 3;
    }
    
    //���ڲ���  -->
    if ((SysParam[SP_BAUDRATE] < BAUDRATE_300) || (SysParam[SP_BAUDRATE] > BAUDRATE_115200))
    {
        SysParam[SP_BAUDRATE] = DEF_BAUDRATE;
    }

    if (SysParam[SP_DATABIT] != 8)
    {
        SysParam[SP_DATABIT]  = DEF_DATABIT;
    }

    if ((SysParam[SP_CHECKBIT] < CHECK_ODD) || (SysParam[SP_CHECKBIT] > CHECK_NONE))
    {
        SysParam[SP_CHECKBIT] = DEF_CHECKBIT;
    }

    if (SysParam[SP_STOPBIT] != 1)
    {
        SysParam[SP_STOPBIT]  = DEF_STOPBIT;
    }
    //<--


    // IP Mode 
    if ((SysParam[SP_DEVIPMODE] < IPMODE_FIXED) || (SysParam[SP_DEVIPMODE] > IPMODE_PPPOE))
    {
        SysParam[SP_DEVIPMODE] = IPMODE_DHCP;
    }


    if (SysParam[SP_TIMERCOUNT] > 3)
    {
        SysParam[SP_TIMERCOUNT] = 0;
    }

    if (SysParam[SP_TIMER1_HOUR] > 23)
    {
        SysParam[SP_TIMER1_HOUR] = 0;
    }

    if (SysParam[SP_TIMER2_HOUR] > 23)
    {
        SysParam[SP_TIMER2_HOUR] = 0;
    }

    if (SysParam[SP_TIMER3_HOUR] > 23)
    {
        SysParam[SP_TIMER3_HOUR] = 0;
    }

    if (SysParam[SP_TIMER1_MINUTE] > 59)
    {
        SysParam[SP_TIMER1_MINUTE] = 0;
    }

    if (SysParam[SP_TIMER2_MINUTE] > 59)
    {
        SysParam[SP_TIMER2_MINUTE] = 0;
    }

    if (SysParam[SP_TIMER3_MINUTE] > 59)
    {
        SysParam[SP_TIMER3_MINUTE] = 0;
    }


    if (SysParam[SP_MODTYPE] > CDMA_MC323)
    {
        SysParam[SP_MODTYPE] = DEF_MODULE;
    }

    if (*(WORD *)&SysParam[SP_RECTIMEOUT] == 0xFFFF)
    {
        *(WORD *)&SysParam[SP_RECTIMEOUT] = DEF_RECTIMEOUT;
    }


    //�����û���������    
    if (!CheckValidUserName())
    {
        mset((BYTE *)&SysParam[SP_NETUSERNAME],0,32);
        mcpy((BYTE *)&SysParam[SP_NETUSERNAME],"card",4);
    }

    if (!CheckValidPassWord())
    {
        mset((BYTE *)&SysParam[SP_NETPASSWORD],0,32);
        mcpy((BYTE *)&SysParam[SP_NETPASSWORD],"card",4);
    }

    //�Ƿ��ӡ������Ϣ
    if (SysParam[SP_DEBUGMSG] > DEBUG_ENABLE)
    {
        SysParam[SP_DEBUGMSG] = DEBUG_DISABLE;
    }
    
    if (*(DWORD *)&SysParam[SP_FLOWRATE] == 0xFFFFFFFF)
    {
        *(DWORD *)&SysParam[SP_FLOWRATE] = 0;
    }
    //У�����
    for(i=0;i<18;i+=2)
    {
        if((*(WORD *)&SysParam[SP_RNKIA+i] ==0xFFFF)||\
            (*(WORD *)&SysParam[SP_RNKIA+i] ==0))
        {
            *(WORD *)&SysParam[SP_RNKIA+i] = DEF_KI_VALUE;
        }
    }
    for(i=0;i<18;i+=2)
    {
        if((*(WORD *)&SysParam[SP_RNKIB+i] ==0xFFFF) ||\
           (*(WORD *)&SysParam[SP_RNKIB+i] ==0) )
        {
            *(WORD *)&SysParam[SP_RNKIB+i] =DEF_KI_VALUE;
        }
    }
    for(i=0;i<18;i+=2)
    {
        if((*(WORD *)&SysParam[SP_RNKU+i] ==0xFFFF)||\
            (*(WORD *)&SysParam[SP_RNKU+i] ==0))
        {
            *(WORD *)&SysParam[SP_RNKU+i] =DEF_KU_VALUE;
        }
    }
    for(i=0;i<18;i+=2)
    {
        if((*(WORD *)&SysParam[SP_RNKAP+i] ==0xFFFF)||\
           (*(WORD *)&SysParam[SP_RNKAP+i] ==0))
        {
            *(WORD *)&SysParam[SP_RNKAP+i]= DEF_KP_VALUE;
        }
    }
    for(i=0;i<18;i+=2)
    {
        if((*(WORD *)&SysParam[SP_RNKBP+i] ==0xFFFF)||\
            (*(WORD *)&SysParam[SP_RNKBP+i] ==0))
        {
            *(WORD *)&SysParam[SP_RNKBP+i]= DEF_KP_VALUE;
        }
    }
    for(i=0;i<18;i+=2)
    {
        if(*(WORD *)&SysParam[SP_RNIARMSOS+i] ==0xFFFF)   
        {
            *(WORD *)&SysParam[SP_RNIARMSOS+i]= 0x0;;
        }
    }
    for(i=0;i<18;i+=2)
    {
        if(*(WORD *)&SysParam[SP_RNIBRMSOS+i] ==0xFFFF)  
        {
            *(WORD *)&SysParam[SP_RNIBRMSOS+i]= 0x0;
        }
    }
    for(i=0;i<18;i+=2)
    {
        if(*(WORD *)&SysParam[SP_RNIBGAIN+i] ==0xFFFF)  
        {
            *(WORD *)&SysParam[SP_RNIBGAIN+i]= 0;
        }
    }
    for(i=0;i<18;i+=2)
    {
        if(*(WORD *)&SysParam[SP_RNGPQA+i] ==0xFFFF)   
        {
            *(WORD *)&SysParam[SP_RNGPQA+i]= 0x0000;
        }
    }
    for(i=0;i<18;i+=2)
    {
        if(*(WORD *)&SysParam[SP_RNGPQB+i] ==0xFFFF)    
        {
            *(WORD *)&SysParam[SP_RNGPQB+i]= 0x0000;
        }
    }
    for(i=0;i<18;i+=2)
    {
        if(*(WORD *)&SysParam[SP_RNAPOSA+i] ==0xFFFF)    
        {
            *(WORD *)&SysParam[SP_RNAPOSA+i]= 0x00;
        }
    }
    for(i=0;i<18;i+=2)
    {
        if(*(WORD *)&SysParam[SP_RNAPOSB+i] ==0xFFFF)    
        {
            *(WORD *)&SysParam[SP_RNAPOSB+i]= 0x00;
        }
    }
    for(i=0;i<18;i+=2)
    {
        if(*(WORD *)&SysParam[SP_RNPHSA+i] ==0xFFFF)    
        {
            *(WORD *)&SysParam[SP_RNPHSA+i]= 0x00;
        }
    }
    for(i=0;i<18;i+=2)
    {
        if(*(WORD *)&SysParam[SP_RNPHSB+i] ==0xFFFF)    
        {
            *(WORD *)&SysParam[SP_RNPHSB+i]= 0x00;
        }
    }
    for(i=0;i<18;i+=2)
    {
        if(*(WORD *)&SysParam[SP_RNRPOSA+i] ==0xFFFF)    
        {
            *(WORD *)&SysParam[SP_RNRPOSA+i]= 0x00;
        }
    }
    for(i=0;i<18;i+=2)
    {
        if(*(WORD *)&SysParam[SP_RNRPOSB+i] ==0xFFFF)    
        {
            *(WORD *)&SysParam[SP_RNRPOSB+i]= 0x00;
        }
    }
    for(i=0;i<18;i+=2)
    {
        if(*(WORD *)&SysParam[SP_RNQPHSCAL+i] ==0xFFFF)    
        {
            *(WORD *)&SysParam[SP_RNQPHSCAL+i]= 0;
        }
    }
    for(i=0;i<18;i+=2)
    {
        if(*(WORD *)&SysParam[SP_RNHFCONST+i] ==0xFFFF)    
        {
            *(WORD *)&SysParam[SP_RNHFCONST+i]= 0x58b;
        }
    }
    for(i=0;i<18;i+=2)
    {
        if(*(WORD *)&SysParam[SP_RNSYSCON+i] ==0xFFFF)    
        {
            *(WORD *)&SysParam[SP_RNSYSCON+i]= 0x40;
        }
    }
    for(i=0;i<18;i+=2)
    {
        if(*(WORD *)&SysParam[SP_RNPSTART+i] ==0xFFFF)    
        {
            *(WORD *)&SysParam[SP_RNPSTART+i]= 0x60;
        }
    }
    for(i=0;i<18;i+=2)
    {
        if(*(WORD *)&SysParam[SP_RNQSTART+i] ==0xFFFF)    
        {
            *(WORD *)&SysParam[SP_RNQSTART+i]= 0x120;
        }
    }
    //�����ܼĴ����������
    for(i=0;i<9;i++)
    {
        if(SysParam[SP_PEOIF_COUNT+i]==0xFF)
        {
            SysParam[SP_PEOIF_COUNT+i]=0;   
        }
    }
 
    //����ַ 6λ*6
    for(i=0;i<6;i++)
    {
        if(SysParam[SP_METERADDR1+i]==0xFF)
        {
            SysParam[SP_METERADDR1+i]=0x01;   
        }
    }
    for(i=0;i<6;i++)
    {
        if(SysParam[SP_METERADDR2+i]==0xFF)
        {
            SysParam[SP_METERADDR2+i]=0x02;   
        }
    }
    for(i=0;i<6;i++)
    {
        if(SysParam[SP_METERADDR3+i]==0xFF)
        {
            SysParam[SP_METERADDR3+i]=0x03;   
        }
    }
    for(i=0;i<6;i++)
    {
        if(SysParam[SP_METERADDR4+i]==0xFF)
        {
            SysParam[SP_METERADDR4+i]=0x04;   
        }
    }
    for(i=0;i<6;i++)
    {
        if(SysParam[SP_METERADDR5+i]==0xFF)
        {
            SysParam[SP_METERADDR5+i]=0x05;   
        }
    }
    for(i=0;i<6;i++)
    {
        if(SysParam[SP_METERADDR6+i]==0xFF)
        {
            SysParam[SP_METERADDR6+i]=0x06;   
        }
    }
    if(*(WORD *)&SysParam[SP_RNIE]==0xFFFF)
    {
        *(WORD *)&SysParam[SP_RNIE]=0x0;//��ֹ�ж��������        
    }
   
}


BYTE SaveSysParam(void)
{
   return saveParam(&gDevParam);
}
BYTE CheckValidUserName(void)
{
    static BYTE i;

    if (SysParam[SP_NETUSERNAME] == 0)
    {
        return FALSE;
    }
        
    for (i=0; i<32; i++)
    {
        if (SysParam[i+SP_NETUSERNAME] > 127)
        {
            return FALSE;
        }
    }
    return TRUE;
}

BYTE CheckValidPassWord(void)
{
    static BYTE i;

    if (SysParam[SP_NETPASSWORD] == 0)
    {
        return FALSE;
    }
    
    for (i=0; i<32; i++)
    {
        if (SysParam[i+SP_NETPASSWORD] > 127)
        {
            return FALSE;
        }
    }
    return TRUE;
}


void SysReset(void)
{
    //�ر�����ģ���Դ
    MOD_POWEROFF();
    
    while(1)
    {
        ;
    }
}

BYTE SysSelfCheck(void)
{
    /*if (!CheckRealTime())
    {
        DebugStr("RealTimer Err.\r\n");
        SetLastError(ERR_RTCFAIL);
        return FALSE;
    }*/
      //��ʾ��ǰʱ��
    DebugChar(SysTime[0]/16+0x30);
    DebugChar(SysTime[0]%16+0x30);
    DebugChar('-');

    DebugChar(SysTime[2]/16+0x30);
    DebugChar(SysTime[2]%16+0x30);
    DebugChar('-');

    DebugChar(SysTime[3]/16+0x30);
    DebugChar(SysTime[3]%16+0x30);
    DebugChar(' ');
    
    DebugChar(SysTime[4]/16+0x30);
    DebugChar(SysTime[4]%16+0x30);
    DebugChar(':');

    DebugChar(SysTime[5]/16+0x30);
    DebugChar(SysTime[5]%16+0x30);
    DebugChar(':');

    DebugChar(SysTime[6]/16+0x30);
    DebugChar(SysTime[6]%16+0x30);

    printf("\r\n");
    

    return TRUE;    
}


/*******************************************************************************
*������:
����()
*����: ��ʱ��
*����:
*���:
*˵��:
*******************************************************************************/
int ethernet_Init( void )
{
    unsigned long ulUser0, ulUser1;
    char pucMACArray[8];
    uchar *pex = SysParam;
//    unsigned int  i;
//    unsigned int ulTemp;
#if 1 
    SysCtlPeripheralEnable ( SYSCTL_PERIPH_ETH );
    SysCtlPeripheralReset ( SYSCTL_PERIPH_ETH );

    SysCtlPeripheralEnable ( SYSCTL_PERIPH_GPIOF );
    GPIOPinConfigure ( GPIO_PF2_LED1 );
    GPIOPinConfigure ( GPIO_PF3_LED0 );
    GPIOPinTypeEthernetLED ( GPIO_PORTF_BASE, GPIO_PIN_2 | GPIO_PIN_3 );
   
    GPIODirModeSet(SYSCTL_PERIPH_GPIOF, GPIO_PIN_2, GPIO_DIR_MODE_OUT);
    GPIOPadConfigSet(SYSCTL_PERIPH_GPIOF, GPIO_PIN_2, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD);
    GPIODirModeSet(SYSCTL_PERIPH_GPIOF, GPIO_PIN_3, GPIO_DIR_MODE_OUT);
    GPIOPadConfigSet(SYSCTL_PERIPH_GPIOF, GPIO_PIN_3, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD);
    
#else

    SysCtlPeripheralReset(SYSCTL_PERIPH_ETH);                  /*��λ��̫��*/
    
    /*****���¾��Ƕ���̫��������******************/
    
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ETH);                 /*ʹ����̫������*/
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);               /*ʹ��LED����*/
        
    
    GPIODirModeSet(GPIO_PORTF_BASE, (GPIO_PIN_2 | GPIO_PIN_3),
                   GPIO_DIR_MODE_HW);
    GPIOPadConfigSet(GPIO_PORTF_BASE, (GPIO_PIN_2 | GPIO_PIN_3),
                   GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD);             /*����LED��Ӳ�����ƣ��������Ϊ2ma*/
    
    
    EthernetIntDisable(ETH_BASE, (ETH_INT_PHY | ETH_INT_MDIO | ETH_INT_RXER |
                           ETH_INT_RXOF | ETH_INT_TX | ETH_INT_TXER | ETH_INT_RX));
    ulTemp = EthernetIntStatus(ETH_BASE, false);
    EthernetIntClear(ETH_BASE, ulTemp);
    
    EthernetInit(ETH_BASE);                                     /*���ڵ�һ��ʹ����̫���ĳ�ʼ��*/
    for(i=0;i<255;i++) ;
    EthernetConfigSet(ETH_BASE, (ETH_CFG_TX_DPLXEN | ETH_CFG_TX_CRCEN |
                                     ETH_CFG_TX_PADEN));            /*ʹ��ȫ˫��ģʽ���Զ�����CRCУ�飬�Զ���䷢�����ݴﵽ��Сֵ*/
         
    EthernetEnable(ETH_BASE);                                   /*ʹ����̫��*/
    
    //IntEnable(INT_ETH);                                         /*ʹ����̫�������ж�*/
    //IntPrioritySet(INT_ETH, 0xFF);                              /*������̫���ж����ȼ�*/
    //EthernetIntEnable(ETH_BASE, ETH_INT_RX);                    /*ʹ����̫�������ж�*/
#endif
    //
    // Configure SysTick for a periodic interrupt.
    //
    //SysTickPeriodSet ( SysCtlClockGet() / SYSTICKHZ );
    //SysTickEnable();
    //SysTickIntEnable();

    //
    // Enable processor interrupts.
    //
    IntMasterEnable();


    //
    // Configure the hardware MAC address for Ethernet Controller filtering of
    // incoming packets.
    //
    // For the LM3S6965 Evaluation Kit, the MAC address will be stored in the
    // non-volatile USER0 and USER1 registers.  These registers can be read
    // using the FlashUserGet function, as illustrated below.
    //

    /*mac ��ַ��оƬ�ڲ�������дλ�ö�ȡ*/  /*дMAC��ַ*/
    FlashUserGet ( &ulUser0, &ulUser1 );

    if(ulUser0 == 0xffffffff  ||  ulUser1 == 0xffffffff) 
    {
        uint *add = (uint *)(&pex[SP_MACADDR]);
        if(*add == 0xFFFFFFFF || *add == 0)
        {
            setRandMac();
            saveParam( &gDevParam );
        }
        pucMACArray[0] = pex[SP_MACADDR];
        pucMACArray[1] = pex[SP_MACADDR+1];
        pucMACArray[2] = pex[SP_MACADDR+2];
        pucMACArray[3] = pex[SP_MACADDR+3];
        pucMACArray[4] = pex[SP_MACADDR+4];
        pucMACArray[5] = pex[SP_MACADDR+5];
    }
    else
    {
        pucMACArray[0] = ( ( ulUser0 >>  0 ) & 0xff );
        pucMACArray[1] = ( ( ulUser0 >>  8 ) & 0xff );
        pucMACArray[2] = ( ( ulUser0 >> 16 ) & 0xff );
        pucMACArray[3] = ( ( ulUser1 >>  0 ) & 0xff );
        pucMACArray[4] = ( ( ulUser1 >>  8 ) & 0xff );
        pucMACArray[5] = ( ( ulUser1 >> 16 ) & 0xff );
    }

   printf("\r\neth mac addr:%02X-%02X-%02X-%02X-%02X-%02X",pucMACArray[0],pucMACArray[1],
                            pucMACArray[2],pucMACArray[3],pucMACArray[4],pucMACArray[5]);

    if (SysParam[SP_DEVIPMODE] == IPMODE_DHCP)
    {
           printf("\r\nuse dynamic IP address.");
           lwIPInit((uchar *)pucMACArray, 0, 0, 0, IPADDR_USE_DHCP);
    }
    else
    {
        unsigned int ipaddr;

        printf ( "\r\nstatic IP addr:%s" ,getStrParamIpAddr ( (char *)&SysParam[ SP_LOCALIP] ) );
        
        memcpy ( &ipaddr , &SysParam[SP_LOCALIP] , 4 );
        ipaddr =  htonl ( ipaddr );

         lwIPInit ( (uchar *)pucMACArray, ipaddr,
                   htonl ( inet_addr ( gDevParam.DevIpAddrMask ) ),
                   htonl ( inet_addr ( gDevParam.DevIpGatway ) ),
                   IPADDR_USE_STATIC );
    }
   
    EthernetEnable(ETH_BASE); 

    return true;

}


void  Tmr_TickInit ( void )
{
    SysTickPeriodSet ( ( unsigned int ) ( SysCtlClockGet() / OS_TICKS_PER_SEC ) - 1 );
    SysTickEnable();
    SysTickIntEnable();
}


/*******************************************************************************
*������:
����()
*����: ��ʱ��
*����:
*���:
*˵��:
*******************************************************************************/
void taskWatchDog ( void *pParam )
{
    printf ( "\r\nWatchDog task begin..." );

    SysCtlPeripheralEnable(SYSCTL_PERIPH_WDOG0);
    
    //
    // ���Ĵ����Ƿ���������������˼Ĵ������������ͷš�
    //
    if(WatchdogLockState(WATCHDOG0_BASE) == true)
    {
        WatchdogUnlock(WATCHDOG0_BASE);
    }
   
    //
    // ��ʼ�����Ź���ʱ����
    //
    WatchdogReloadSet(WATCHDOG0_BASE,0xFEEFEE);
    //
    // ʹ�ܸ�λ��
    //   
    WatchdogResetEnable(WATCHDOG0_BASE);
    //
    // ʹ�ܿ��Ź���ʱ����
    //   
    WatchdogEnable(WATCHDOG0_BASE);
  
    //IntEnable(INT_WATCHDOG);
    //
    // ι��
    //  

    while(1)
    {
       
        WatchdogReloadSet(WATCHDOG0_BASE, TheSysClock*7);//5s
        OSTimeDly(OS_TICKS_PER_SEC*2);
    }
  
}

void taskMainPackHandle ( void *p_arg )
{

    gPubVal.resetCause  = SysCtlResetCauseGet();
    SysCtlResetCauseClear(gPubVal.resetCause);
    
    printf("\r\n\nresetCause:%x " ,gPubVal.resetCause  );
    printf("\nCPU speed %u",SysCtlClockGet());    
    printf ( "\r\nMainPackHandle task begin..." );
    
    loadParamFromFlash( &gDevParam );//��ȡ����
    uartInitRs485();

    commBaseCtrlInit ( UART0_BASE , 0 );
    commBaseCtrlInit ( UART1_BASE , 1 );
    commBaseCtrlInit ( UART2_BASE , 2 );
   
    IntEnable ( INT_UART0 );
    IntEnable ( INT_UART1 );
    IntEnable ( INT_UART2 );
    
    
    ethernet_Init();
    spiInit();
    OSTimeDly(2 * OS_TICKS_PER_SEC);
    init8209();
      
    if ( gPubVal.bCfgShell )
    {
        OSTaskCreate ( CmdLine,         // Initialize the start task  ��ʼ��shell����
                       ( void * ) 0 ,
                       &stkTaskShell[STK_SIZE_SHELL-1],
                       PRIO_SHELL );
    }
    else
    {
       OSTaskCreate ( taskUartHandle ,         // Initialize the start task  ��ʼ��shell����
                       ( void * ) 0 ,
                       &stkTaskShell[STK_SIZE_SHELL-1],
                       PRIO_SHELL );
    } 
              
    OSTaskCreate ( taskPriod,         //  ��ʼ����������
                   ( void * ) 0 ,
                   &stkTaskPrid[STK_SIZE_PRID-1],
                   PRIO_PRID );
 

    OSTaskCreate ( taskWatchDog,         //   ��ʼ��taskWatchDog����
                   ( void * ) 0 ,
                   &stkTaskWatch0[STK_SIZE_WATCH0-1],
                   PRIO_WATCH0 );
#if 1
    OSTaskCreate ( taskHeartRep,         // Initialize the start task  ��ʼ����������
                   ( void * ) 0 ,
                   &stkTaskHearReq[STK_SIZE_HEART_REP-1],
                   PRIO_HEART_REP );

    OSTaskCreate ( taskRS485Rx,         // Initialize the start task  ��ʼ��485����
                   ( void * ) 0 ,
                   &stkTaskRS485[STK_SIZE_RS485-1],
                   PRIO_RS485 );
 
   
    OSTaskCreate ( taskGSMGetMsg,         // 
                   ( void * ) 0 ,
                   &stkTaskGSMMSG[STK_SIZE_GSMMSG-1],
                   PRIO_GSMMSG);
                   
     OSTaskCreate ( taskGSM,         // 
                   ( void * ) 0 ,
                   &stkTaskGSM[STK_SIZE_GSM-1],
                   PRIO_GSM);
#endif                   
    tcpServerInit();
    upDateServerInit();

    //printf("\r\n\r\nUpdatae test!!!!!!!!!!!!!\r\n"); 
    while (1) 
    {                             
        OSTaskSuspend(OS_PRIO_SELF);  /*  The start task can be pended here */
    } 
      
}


/*******************************************************************************
*������:
����()
*����: ��ʱ��
*����:
*���:
*˵��:
*******************************************************************************/
int main ( void )
{
    memset ( &gPubVal , 0 , sizeof ( gPubVal ) );
    //sprintf(DevRelTime, "%s %s " , __DATE__ , __TIME__ );
    gPubVal.bCfgShell = true;
    
    //strcpy(gPubVal.version, SWVERSION);
    //strcpy(gPubVal.devNo, DEVICENO);
    //sprintf(gPubVal.buildTime, "%s %s " , __DATE__ , __TIME__ );

    IntDisAll();                       // Disable all the interrupts �ر������ж�
    
//    SysCtlClockSet( SYSCTL_SYSDIV_12 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_8MHZ ); //for 9B96
//    SysCtlClockSet( SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ );  //for 6938
    SysCtlClockSet( SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_8MHZ );  //for 6938

    TheSysClock = SysCtlClockGet();
    FlashUsecSet(TheSysClock/1000000);
    
    GetVersion();
    gpioCommInit();
    commUartInit();
    I2C_FM24C_Init();
    IntMasterEnable();//

    OSInit();                          // Initialize the kernel of uC/OS-II ��ʼ��uC/OS-II���ں�

    OSTaskCreate ( taskMainPackHandle,         // Initialize the start task  ��ʼ����������
                   ( void * ) 0,
                   &stkTaskMainHandle[STK_SIZE_MAIN_HANDLE-1],
                   PRIO_MAIN_HANDLE ); 
    
    Tmr_TickInit();//ΪOS �ṩʱ��
   
    OSStart();                        // Start uC/OS-II  ����uC/OS-II
  
    return ( 0 ) ;

}



