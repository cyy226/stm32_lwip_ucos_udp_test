#ifndef  __INCLUDES_H__
#define  __INCLUDES_H__

#include <cmmtype.h>

#include <ucos_ii.h>
#include "Os_cpu.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "update.h"
#include "main.h"
#include "console.h"
#include "comm.h"



#define PARA_CNT_MAX    6
#define PARA_LEN_MAX    20
#define MAX_CMD_BUF_LEN 100


typedef struct
{
	uint argc;
	char argv[PARA_CNT_MAX][PARA_LEN_MAX];
	uint level;
} CMD_LIN;

typedef struct
{
    char cmdName[16];
    char cmdHelp[30];
    uint ( *routine ) ( CMD_LIN* );
} linCmdTabSTRUC;


/***************************************************
*  t_commCtrl:   ͨѶ���ƿ�
*  ÿ���ӿ�һ���������жϴ�����������豸
****************************************************/
typedef struct
{
    unsigned long uartBase;     /*�ÿ��ƿ鴦���Ӳ��*/
    void * pRecvNetBuff;          /*���ڽ����õ�netBuff */

    OS_EVENT * pRecvMbox;  /*�ж��յ����ĺ��������Ͷ��*/

    OS_EVENT * pSendMux;    /*�ô˱�����֤���ͷ��ʵĻ���*/
    OS_EVENT * pSendNoteBack;   /*��������÷��ͺ���������ͬ����
                                                            ���������ж�ʱ���ô˺���*/

    OS_EVENT * pRecvNoteBack;   /*��������÷��ͺ���������ͬ����
                                                            ���������ж�ʱ���ô˺���*/

    /*���������ڽ����жϵ��ֳ�����*/
    uint recv_rLen;     /*��ǰ�����յ��ֽ���*/
    uchar recv_xor;     /*���У��*/
    // void * pRecvdata;
    // unsigned int recv_packBegin; /*ָʾ��ǰ�յ�����ͷ�����Խ��պ����ֽ�*/

    /*    ��������ͳ��*/
    uint cntTxPack;
    uint cntRxPack;
    uint cntRxChar;
    uint cntRxErr;
    uint commReInitRemain ;
} t_commCtrl;


typedef struct
{
    uint resetCause; 
    char mcuNeedReboot;

    bool bCfgShell;
    uint bLastRs485CmdFromUart;
    //uint iSvrNoCommCnt;

    uint iRs485KeepLive;
    uint iHeartReportKeepLive;

    uint bTcpInComm;
    //uint last485LifeIsMod;
        
}tPubVal;

extern tPubVal gPubVal;


/*debug flag*/
#define CMM_DEBUG_COMM      (1<<0)
#define CMM_DEBUG_UPDTE     (1<<1)
#define CMM_DEBUG_METER     (1<<2)
#define CMM_DEBUG_SHELL     (1<<3)

extern uint debugFlag;

#define _pl_   printf("\n%s L:%d " , __FUNCTION__ ,__LINE__);
#define DBCMM(str)  if (debugFlag & CMM_DEBUG_COMM) printf("\n%s(%d): %s", __FUNCTION__, __LINE__, str)
#define DBMTR(str)  if (debugFlag & CMM_DEBUG_METER) printf("\n%s(%d): %s", __FUNCTION__, __LINE__, str)
#define DBSHL(str)  if (debugFlag & CMM_DEBUG_SHELL) printf("\n%s(%d): %s", __FUNCTION__, __LINE__, str)
#define DBUDT(str)  if (debugFlag & CMM_DEBUG_UPDTE) printf("\n%s(%d): %s", __FUNCTION__, __LINE__, str)

#define SPEED_UART_STD    115200
#define SPEED_UART_HIGH   921600
#define SPEED_UART_RS485  1200

//#define FLASH_ADDR_PRARM  (1024 * 254)
#define FLASH_ADDR_PRARM    0x0003FC00

#define FLASH_PARAM_FLAG  0X12345678

#define BEEP(bOn)  signalOut(GPIO_PORTF_BASE ,GPIO_PIN_5, (bOn))

//--------include function from uarthandle.c---------
void commUartInit ( void );
//void shellCtrlInit ( void );
uchar shellPutChar ( uchar ch );
//uchar shellGetChar ( void );
void commBaseCtrlInit ( unsigned long uartBase , unsigned int channel );
void commBaseIsr (unsigned int channel );
char commBaseGetChar (unsigned int channel );
void commBasePutChar (unsigned int channel ,char ch );
void rs485TxSwitch(  unsigned int bTx);
void commBaseRs485SendBuff(unsigned int channel ,char* buff,int len);
//char commBaseRs485RecvBuff(unsigned int channel ,char* buff,int len);
void uartInitRs485(void);
ushort getParamBps(void);
char commBaseGetCharTimeout( uint channel ,uint timeout, uchar *err);


//--------include function from hwMisc.c---------
uint signalIn ( unsigned long ulPort ,  uchar ucPins );
void signalOut ( unsigned long ulPort ,  uchar ucPins , uint bHigh );
void watchdogInit ( void );
void watchdogFeed ( void );
void SysReset(void);
void gpioCommInit ( void );
int  bEthPhyLink(void);
extern void armLed(int bOn);


//--------include function from other file---------
void IntDisAll(void);
int atoi(const char *s);
uint StrToNum ( char * str );
int matchCmd ( CMD_LIN * pCmd, const linCmdTabSTRUC * pCmdTab );
void sendToLastConnectFromClient ( void * buff , int len );

#endif


