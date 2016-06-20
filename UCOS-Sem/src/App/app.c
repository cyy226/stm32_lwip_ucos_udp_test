#include "includes.h"
#include "LwIP.h"
#include "api.h"
/*
*********************************************************************************************************
*                                        ����ջ
*********************************************************************************************************
*/
static  OS_STK         App_TaskToggleLEDStk[APP_CFG_TASK_ToggleLED_STK_SIZE];
static  OS_STK         App_TaskToggleLED1Stk[APP_CFG_TASK_ToggleLED_STK_SIZE];
////add xpz

//�����ջ�������ڴ����ķ�ʽ��������	
static OS_STK LWIP_DHCP_TASK_STK[LWIP_DHCP_STK_SIZE];	
static OS_STK UDPRECV_TASK_STK[UDPRECV_STK_SIZE];
//������
//void lwip_dhcp_task(void *pdata); 
////
/*
*********************************************************************************************************
*                                        �������
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                       ����ԭ������
*********************************************************************************************************
*/
static void App_TaskToggleLED(void *p_arg);
static void App_TaskToggleLED1(void *p_arg);
static void udp_recv_thread(void *p_arg);
/*
 * ��������App_TaskCreate
 * ��  ����
 * ��  �룺��
 * ��  ������
 */
void App_TaskCreate(void)
{
#if OS_TASK_NAME_SIZE > 7
    INT8U  err;
#endif

    //LED ����˸����
#if OS_TASK_CREATE_EXT_EN > 0
    OSTaskCreateExt((void (*)(void *)) App_TaskToggleLED,             /* Create the start task                                    */
                    (void           *) 0,
                    (OS_STK         *)&App_TaskToggleLEDStk[APP_CFG_TASK_ToggleLED_STK_SIZE - 1],
                    (INT8U           ) APP_CFG_TASK_ToggleLED_PRIO,
                    (INT16U          ) APP_CFG_TASK_ToggleLED_PRIO,
                    (OS_STK         *)&App_TaskToggleLEDStk[0],
                    (INT32U          ) APP_CFG_TASK_ToggleLED_STK_SIZE,
                    (void           *) 0,
                    (INT16U          )(OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR));
	
	OSTaskCreate((void (*)(void *)) App_TaskToggleLED1,
							(void          * ) 0,
							(OS_STK        * )&App_TaskToggleLED1Stk[APP_CFG_TASK_ToggleLED_STK_SIZE - 1],
							(INT8U           ) APP_CFG_TASK_ToggleLED1_PRIO
								);
//add xpz
	OSTaskCreate((void (*)(void *))	 lwip_dhcp_task,
							 (void					 *)0,
							 (OS_STK				 *)&LWIP_DHCP_TASK_STK[LWIP_DHCP_STK_SIZE-1],
							 (INT8U           )	LWIP_DHCP_TASK_PRIO
							  );//����DHCP����
	OSTaskCreate((void (*)(void *))	 udp_recv_thread,
							 (void					 *)0,
							 (OS_STK				 *)&UDPRECV_TASK_STK[UDPRECV_STK_SIZE-1],
							 (INT8U           )	UDPRECV_PRIO
							  );//����DHCP����							 
#else    
    OSTaskCreate((void (*)(void *)) App_TaskToggleLED,
           (void          * ) 0,
           (OS_STK        * )&App_TaskToggleLEDStk[APP_CFG_TASK_ToggleLED_STK_SIZE - 1],
           (INT8U           ) APP_CFG_TASK_ToggleLED_PRIO
            );

    
#endif    
   
}

/*
 * ��������App_TaskToggleLED
 * ��  ����
 * ��  �룺��
 * ��  ������
 */
static void App_TaskToggleLED(void *p_arg)
{
  (void)p_arg;
	
  while(1)
  {
    BSP_LedToggle();
    OSTimeDlyHMSM(0, 0, 1, 0);
  }
}

static void App_TaskToggleLED1(void *p_arg)
{
  (void)p_arg;
	
  while(1)
  {
    BSP_Led1Toggle();
    OSTimeDlyHMSM(0, 0, 0, 500);
  }
}
static void udp_recv_thread(void *p_arg)
{

	u8_t err;
	struct netconn *udpconn;
	struct netbuf  *recvbuf;
	struct netbuf  *sentbuf;
	struct ip_addr destipaddr;
	u16_t destport;
	
	udpconn = netconn_new(NETCONN_UDP);  
	sentbuf = netbuf_new();
	
	if(udpconn != NULL)  
	{
		err = netconn_bind(udpconn,IP_ADDR_ANY,15020); 
		
		if(err == ERR_OK)
		{
			while(1)
			{
				recvbuf = netconn_recv(udpconn); //��������
				if(recvbuf != NULL)          //���յ�����
				{
					netbuf_alloc(sentbuf, recvbuf->p->len);
					memcpy(sentbuf->p->payload, recvbuf->p->payload, recvbuf->p->len);
					memcpy(&destipaddr, recvbuf->addr, sizeof(struct ip_addr));
					destport = recvbuf->port;
					err = netconn_sendto(udpconn, sentbuf, &destipaddr, destport);
					if(err != ERR_OK)
					{
					}	
					netbuf_delete(recvbuf);     							 //ɾ��buf   				
				}
				OSTimeDlyHMSM(0, 0, 0, 500);
			}
		}
	}
	else
	{
		
	}	
}