/*************************************************************
�ɶ���ջ����������޹�˾   ��Ȩ����
���߲ɼ���


�ļ���:   
��  ��:    �˹���
��  ��:    ����ģ��
�޶���¼:

**************************************************************/

#ifndef __WLMODULE_H__
#define __WLMODULE_H__

//ģ��״̬����
#define InitState          0
#define InitEndState       1
#define CheckIpModState    2
#define TCPLinkState       3
#define LoginSvr           4
#define NomState           5


extern unsigned char NetModulState;


void Init_NetModul(void);

void PowerOn_Modul(void);
void PowerOff_Modul(void);
BYTE Check_Modul(void);

BYTE Check_Net(BYTE FLed);

BYTE ATCmd_GetVersion(void);
extern void Del_all_SMS(void);
extern void CmdTimeOutHandle(void);

extern void taskGSM(void* msg);
extern void taskGSMInit(void* msg);
extern void taskGSMGetMsg(void* msg);


#endif

