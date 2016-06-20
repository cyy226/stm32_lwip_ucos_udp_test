#ifndef _INCLUDES_H__
#define _INCLUDES_H__

#include <stdio.h>
#include <string.h>

#include "stm32f10x.h"
#include "stm32f10x_conf.h"

#include "ucos_ii.h"

#include "BSP.h"
#include "app.h"
#include "app_cfg.h"
//#include "LwIP.h"

#define uchar unsigned char
#define BYTE  unsigned char
#define WORD  unsigned short
#define uint  unsigned int

#define FALSE     0
#define TRUE      1
typedef struct  
{
	u8 mac[6];      //MAC��ַ
	u8 remoteip[4];	//Զ������IP��ַ 
	u8 ip[4];       //����IP��ַ
	u8 netmask[4]; 	//��������
	u8 gateway[4]; 	//Ĭ�����ص�IP��ַ
	
	vu8 dhcpstatus;	//dhcp״̬ 
					//0,δ��ȡDHCP��ַ;
					//1,����DHCP��ȡ״̬
					//2,�ɹ���ȡDHCP��ַ
					//0XFF,��ȡʧ��.
}__lwip_dev;
extern __lwip_dev lwipdev;	//lwip���ƽṹ��

#endif //_INCLUDES_H__
