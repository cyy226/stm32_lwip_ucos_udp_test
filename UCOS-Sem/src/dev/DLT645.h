#ifndef _DLT645_H_
#define _DLT645_H_

#define BYTE unsigned char
#define WORD unsigned short
#define DWORD unsigned long

#define FULL_ERR      0x00
#define ADDR_ERR      0x01
#define LENGTH_ERR    0x02
#define CSCHECK_ERR   0x03

#define SEND_645_MAX  100
#define RECV_645_MAX  200

#define OTHER_ERR     0x01
#define UNREQSDAT_ERR 0x02



typedef struct 
{
    //unsigned char Addr[6];  //��ַ
   volatile unsigned int  EE;  //�ܵ���
   volatile unsigned int  AE;  //A�����
   volatile unsigned int  BE;
   volatile unsigned int  CE;
   volatile unsigned int  Ai;  //A�����
   volatile unsigned int  Bi;
   volatile unsigned int  Ci;
   volatile unsigned short int  Au;  //A���ѹ
   volatile unsigned short int  Bu;
   volatile unsigned short int  Cu;    
   volatile unsigned short int  PQ;//�޹�����
   volatile unsigned short int  PWR;//����
   volatile unsigned short int  PWRA;//A����
   volatile unsigned short int  PWRB;//B����
   volatile unsigned short int  PWRC;//C����
    
}EMeterData_t;




extern BYTE Send645Buf[SEND_645_MAX];
extern BYTE Recv645Buf[RECV_645_MAX];
extern WORD Recv_645Save_num;
extern BYTE Send_645Buf_num;

extern EMeterData_t EMeterDat[6];

/***********************************************************/

extern void sendUart1Term ( unsigned char *buff , int len );

extern void MeterParamInit(void);
extern void ClearDLT645RecvBuf(void);
extern void ClearDLT645SendBuf(void);
extern void RxDataHandle645(BYTE *pBuf, BYTE len);
extern BYTE RxValidDataFrame(BYTE *pBuf, BYTE len);
//extern void MeterCmdAnalysis(void);
//extern void HandleCmdReadData(void);
extern void MeterCmdAnalysis1(BYTE *pchar);
extern void HandleCmdReadData1(BYTE *pchar);

#endif
