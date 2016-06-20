/*************************************************************
�ɶ���ջ����������޹�˾   ��Ȩ����

�ļ���:  Cdma1x.h
��  ��:  �˹���
��  ��:  Cdmaģ��
�޶���¼:   

**************************************************************/


#ifndef __CDMA1X_H__
#define __CDMA1X_H__

BYTE Init_Cdma1x(void);


BYTE NewMsg_C(void);

void DisableRep(void);

BYTE Check_CdmaNet(BYTE FLed);
BYTE Get_Modul_Time(void);

BYTE ATCmd_IpOpen(void);
BYTE ATCmd_SendData(BYTE *Data, BYTE Len);
BYTE ATCmd_SendDataEx(void);

BYTE ATCmd_StartDataMode(void);
BYTE ATCmd_EndDataMode(void);
BYTE ATCmd_InitIp(void);
BYTE ATCmd_CloseIp(void);
BYTE ATCmd_IpListen(void);


BYTE Set_Txt_SMS(void);
BYTE Test_AtCmd(void);
BYTE Hang_Call(void);

BYTE Send_AT_Cmd(BYTE *Cmd);
BYTE Read_SMS(void);
BYTE Get_Sms_Content(void);
BYTE Get_Phone_Number(void);

void WaitModuleNormal(void);


#endif

