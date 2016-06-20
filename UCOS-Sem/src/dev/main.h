/*************************************************************
�ɶ���ջ����������޹�˾   ��Ȩ����

�ļ���:  main.h
��  ��:  �˹���
��  ��:  Cdmaģ��
�޶���¼:   

**************************************************************/


#ifndef __main_H__
#define __mian_H__
#include "comm.h"
#include "includes.h"
/*******************************************************************************
˵�� os ������
*******************************************************************************/
/*tcp server ����Ϣ������*/
#define PRIO_MAIN_HANDLE        2

/*SHELL��������*/
#define PRIO_SHELL              3 

//���ж�PE0����
#define PRIO_RNISR              4

/*rs485��������*/
#define PRIO_RS485              5

//����ģ���������
#define PRIO_GSMMSG             6


/*GSM��������*/
#define PRIO_GSM                7

/*GSM��ʼ������*/
#define PRIO_GSMINIT            8

/*hart report����*/
#define PRIO_HEART_REP          9

/*���ڴ�������*/
#define PRIO_PRID               10

/*WATCHDOG ��������*/
#define PRIO_WATCH0             11

extern BYTE DevVersion[7]        ;       // ��ʽ 001.100        // �豸�汾  7�ֽ�
extern const BYTE DevRelTime[]   ;       // �������ʱ��  2012��  7�ֽ� 
extern const BYTE *DevIndex      ;       // �豸���  8�ֽ�   

extern unsigned int  TheSysClock;


void GetSysParam(void);
void SetDefParam(void);

BYTE SaveSysParam(void);
BYTE CheckValidUserName(void);
BYTE CheckValidPassWord(void);
void SysReset(void);
BYTE SysSelfCheck(void);

#endif

