#define __LW_IP_C

/* Includes ------------------------------------------------------------------*/
#include "lwip/memp.h"
#include "LwIP.h"
#include "lwip/tcp.h"
#include "lwip/udp.h"
#include "lwip/tcpip.h"
#include "netif/etharp.h"
#include "lwip/dhcp.h"
#include "ethernetif.h"
#include "stm32f10x.h"
#include "arch/sys_arch.h"
#include <stdio.h>
#include "stm32_eth.h"
#include "includes.h" 
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define MAX_DHCP_TRIES        4
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static struct netif netif;
static uint32_t IPaddress = 0;

/* Private function prototypes -----------------------------------------------*/
static void TcpipInitDone(void *arg);
static void list_if(void);
////add xpz
__lwip_dev lwipdev;
void lwip_comm_default_ip_set(__lwip_dev *lwipx);
////
/* Private functions ---------------------------------------------------------*/
/** 
 * @brief TcpipInitDone wait for tcpip init being done
 * 
 * @param arg the semaphore to be signaled
 */
static void TcpipInitDone(void *arg)
{
    sys_sem_t *sem;
    sem = arg;
    sys_sem_signal(*sem);
}

/** 
 * @brief Display_IPAddress Display IP Address
 * 
 */
void Display_IPAddress(void)
{
    if(IPaddress != netif.ip_addr.addr)
    {   /* IP ��ַ�����ı�*/
        __IO uint8_t iptab[4];
        uint8_t iptxt[20];
        
        /* read the new IP address */
        IPaddress = netif.ip_addr.addr;
        
        iptab[0] = (uint8_t)(IPaddress >> 24);
        iptab[1] = (uint8_t)(IPaddress >> 16);
        iptab[2] = (uint8_t)(IPaddress >> 8);
        iptab[3] = (uint8_t)(IPaddress);
        
        sprintf((char*)iptxt, "   %d.%d.%d.%d    ", iptab[3], iptab[2], iptab[1], iptab[0]);
        
        list_if();                           
    }
#if LWIP_DHCP
    else if(IPaddress == 0)
    {   // �ȴ�DHCP����IP
        
        /* If no response from a DHCP server for MAX_DHCP_TRIES times */
        /* stop the dhcp client and set a static IP address */
        if(netif.dhcp->tries > MAX_DHCP_TRIES) 
        {   /* ����DHCP���Դ��������þ�̬IP���� */
            struct ip_addr ipaddr;
            struct ip_addr netmask;
            struct ip_addr gw;
            
            dhcp_stop(&netif);
            
//            IP4_ADDR(&ipaddr, 10, 21, 11, 245);
//            IP4_ADDR(&netmask, 255, 255, 255, 0);
//            IP4_ADDR(&gw, 10, 21, 11, 254);
					  IP4_ADDR(&ipaddr, 192, 168, 1, 8);
						IP4_ADDR(&netmask, 255, 255, 255, 0);
						IP4_ADDR(&gw, 192, 168, 1, 1);
            
            netif_set_addr(&netif, &ipaddr , &netmask, &gw);
            
            list_if();
        }
    }
#endif
}

/**
* @brief display ip address in serial port debug windows
*/
static void list_if()
{
    printf("Default network interface: %c%c\n", netif.name[0], netif.name[1]);
    printf("ip address: %s\n", inet_ntoa(*((struct in_addr*)&(netif.ip_addr))));
    printf("gw address: %s\n", inet_ntoa(*((struct in_addr*)&(netif.gw))));
    printf("net mask  : %s\n", inet_ntoa(*((struct in_addr*)&(netif.netmask))));
}

/**
* @brief Init_LwIP initialize the LwIP
*/
void Init_LwIP(void)
{
    struct ip_addr ipaddr;
    struct ip_addr netmask;
    struct ip_addr gw;
		sys_sem_t sem;
   
    sys_init();
    
    /* Initializes the dynamic memory heap defined by MEM_SIZE.*/
    mem_init();
    
    /* Initializes the memory pools defined by MEMP_NUM_x.*/
    memp_init();
    
    pbuf_init();
    netif_init();
    
    printf("TCP/IP initializing... \r\n");
    sem = sys_sem_new(0);
    tcpip_init(TcpipInitDone, &sem);
		lwip_comm_default_ip_set(&lwipdev);	//����Ĭ��IP����Ϣ	//add xpz
    sys_sem_wait(sem);
    sys_sem_free(sem);
    printf("TCP/IP initialized. \r\n");

#if LWIP_DHCP
    /* ����DHCP������ */
    ipaddr.addr = 0;
    netmask.addr = 0;
    gw.addr = 0;
#else
    /* ���þ�̬IP */
	IP4_ADDR(&ipaddr,lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);
	IP4_ADDR(&netmask,lwipdev.netmask[0],lwipdev.netmask[1] ,lwipdev.netmask[2],lwipdev.netmask[3]);
	IP4_ADDR(&gw,lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);
	printf("����en��MAC��ַΪ:................%d.%d.%d.%d.%d.%d\r\n",lwipdev.mac[0],lwipdev.mac[1],lwipdev.mac[2],lwipdev.mac[3],lwipdev.mac[4],lwipdev.mac[5]);
	printf("��̬IP��ַ........................%d.%d.%d.%d\r\n",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);
	printf("��������..........................%d.%d.%d.%d\r\n",lwipdev.netmask[0],lwipdev.netmask[1],lwipdev.netmask[2],lwipdev.netmask[3]);
	printf("Ĭ������..........................%d.%d.%d.%d\r\n",lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);

#endif
    
    Set_MAC_Address(lwipdev.mac);
    
    netif_add(&netif, &ipaddr, &netmask, &gw, NULL, &ethernetif_init, &tcpip_input);
    netif_set_default(&netif);
    
//#if LWIP_DHCP
//    dhcp_start(&netif);
//#endif
    netif_set_up(&netif);
		list_if();
}

/**
* @brief  Ethernet ISR
*/
void LwIP_Pkt_Handle(void)
{
  /* Read a received packet from the Ethernet buffers and send it to the lwIP for handling */
  ethernetif_input(&netif);
}

////add xpz
//���ʹ����DHCP
//#if LWIP_DHCP
//����DHCP����
//void lwip_comm_dhcp_creat(void)
//{
//	OS_CPU_SR cpu_sr;
//	//OS_ENTER_CRITICAL();  //�����ٽ���
//	OSTaskCreate(lwip_dhcp_task,(void*)0,(OS_STK*)&LWIP_DHCP_TASK_STK[LWIP_DHCP_STK_SIZE-1],LWIP_DHCP_TASK_PRIO);//����DHCP���� 
//	//OS_EXIT_CRITICAL();  //�˳��ٽ���
//}
//ɾ��DHCP����
void lwip_comm_dhcp_delete(void)
{
	dhcp_stop(&netif); 		//�ر�DHCP
	OSTaskDel(15);	//ɾ��DHCP����
}
//DHCP��������
void lwip_dhcp_task(void *pdata)
{
	u32 ip=0,netmask=0,gw=0;
	printf("���ڲ���DHCP������,���Ե�...........\r\n"); 
	dhcp_start(&netif);//����DHCP 
	lwipdev.dhcpstatus=0;	//����DHCP
	printf("���ڲ���DHCP������,���Ե�...........\r\n");   
	while(1)
	{ 
		printf("���ڻ�ȡ��ַ...\r\n");
		ip=netif.ip_addr.addr;		//��ȡ��IP��ַ
		netmask=netif.netmask.addr;//��ȡ��������
		gw=netif.gw.addr;			//��ȡĬ������ 
		if(ip!=0)   					//����ȷ��ȡ��IP��ַ��ʱ��
		{
			lwipdev.dhcpstatus=2;	//DHCP�ɹ�
 			printf("����en��MAC��ַΪ:................%d.%d.%d.%d.%d.%d\r\n",lwipdev.mac[0],lwipdev.mac[1],lwipdev.mac[2],lwipdev.mac[3],lwipdev.mac[4],lwipdev.mac[5]);
			//������ͨ��DHCP��ȡ����IP��ַ
			lwipdev.ip[3]=(uint8_t)(ip>>24); 
			lwipdev.ip[2]=(uint8_t)(ip>>16);
			lwipdev.ip[1]=(uint8_t)(ip>>8);
			lwipdev.ip[0]=(uint8_t)(ip);
			printf("ͨ��DHCP��ȡ��IP��ַ..............%d.%d.%d.%d\r\n",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);
			//����ͨ��DHCP��ȡ�������������ַ
			lwipdev.netmask[3]=(uint8_t)(netmask>>24);
			lwipdev.netmask[2]=(uint8_t)(netmask>>16);
			lwipdev.netmask[1]=(uint8_t)(netmask>>8);
			lwipdev.netmask[0]=(uint8_t)(netmask);
			printf("ͨ��DHCP��ȡ����������............%d.%d.%d.%d\r\n",lwipdev.netmask[0],lwipdev.netmask[1],lwipdev.netmask[2],lwipdev.netmask[3]);
			//������ͨ��DHCP��ȡ����Ĭ������
			lwipdev.gateway[3]=(uint8_t)(gw>>24);
			lwipdev.gateway[2]=(uint8_t)(gw>>16);
			lwipdev.gateway[1]=(uint8_t)(gw>>8);
			lwipdev.gateway[0]=(uint8_t)(gw);
			printf("ͨ��DHCP��ȡ����Ĭ������..........%d.%d.%d.%d\r\n",lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);
			break;
		}else if(netif.dhcp->tries>MAX_DHCP_TRIES) //ͨ��DHCP�����ȡIP��ַʧ��,�ҳ�������Դ���
		{  
			lwipdev.dhcpstatus=0XFF;//DHCPʧ��.
			//ʹ�þ�̬IP��ַ
			IP4_ADDR(&(netif.ip_addr),lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);
			IP4_ADDR(&(netif.netmask),lwipdev.netmask[0],lwipdev.netmask[1],lwipdev.netmask[2],lwipdev.netmask[3]);
			IP4_ADDR(&(netif.gw),lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);
			printf("DHCP����ʱ,ʹ�þ�̬IP��ַ!\r\n");
			printf("����en��MAC��ַΪ:................%d.%d.%d.%d.%d.%d\r\n",lwipdev.mac[0],lwipdev.mac[1],lwipdev.mac[2],lwipdev.mac[3],lwipdev.mac[4],lwipdev.mac[5]);
			printf("��̬IP��ַ........................%d.%d.%d.%d\r\n",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);
			printf("��������..........................%d.%d.%d.%d\r\n",lwipdev.netmask[0],lwipdev.netmask[1],lwipdev.netmask[2],lwipdev.netmask[3]);
			printf("Ĭ������..........................%d.%d.%d.%d\r\n",lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);
			break;
		}  
		OSTimeDlyHMSM(0,0,10,0);
	}
	lwip_comm_dhcp_delete();//ɾ��DHCP���� 
}

//lwip Ĭ��IP����
//lwipx:lwip���ƽṹ��ָ��
void lwip_comm_default_ip_set(__lwip_dev *lwipx)
{
	//Ĭ��Զ��IPΪ:192.168.1.100
	lwipx->remoteip[0]=192;	
	lwipx->remoteip[1]=168;
	lwipx->remoteip[2]=1;
	lwipx->remoteip[3]=100;
	//MAC��ַ����(�����ֽڹ̶�Ϊ:2.0.0,�����ֽ���STM32ΨһID)
	lwipx->mac[0]=2;
	lwipx->mac[1]=0;
	lwipx->mac[2]=0;
	lwipx->mac[3]=0;
	lwipx->mac[4]=1;
	lwipx->mac[5]=6; 
	//Ĭ�ϱ���IPΪ:192.168.1.30
	lwipx->ip[0]=192;	
	lwipx->ip[1]=168;
	lwipx->ip[2]=1;
	lwipx->ip[3]=30;
	//Ĭ����������:255.255.255.0
	lwipx->netmask[0]=255;	
	lwipx->netmask[1]=255;
	lwipx->netmask[2]=255;
	lwipx->netmask[3]=0;
	//Ĭ������:192.168.1.1
	lwipx->gateway[0]=192;	
	lwipx->gateway[1]=168;
	lwipx->gateway[2]=1;
	lwipx->gateway[3]=1;	
	lwipx->dhcpstatus=0;//û��DHCP	
} 
//#endif 
////