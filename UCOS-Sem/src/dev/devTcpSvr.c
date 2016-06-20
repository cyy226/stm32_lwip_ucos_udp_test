#include "includes.h"
#include <dev.h>
#include <SCom.h>
#include <comm.h>
#include <err.h>
#include <ip_addr.h>
#include <tcp.h>
#include "DLT645.h"
#include "devTcpip.h"

/*���ͻ��˷���һ��tcp���ӣ����������Ӿ����
��rs485�յ�����ʱ������������д������*/
struct tcp_pcb * gPcbLastConnectFromClient = NULL;

uchar TCP_DataBuf1[100]={0};
uchar TCP_sav_num1=0;
unsigned int debugFlag = 1<<2;
//void tcpsendend(void *arg, struct tcp_pcb *tpcb, u16_t len);
static err_t tcpSvr_err ( void *arg,  err_t err );
/*******************************************************************************
*������:
����tcpSvr_recv()
*����: ����һ���ص���������һ��TCP�ε����������ʱ�ᱻ����

*����:
*���:
*˵��:
*******************************************************************************/
static err_t tcpSvr_recv ( void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err )
{
   // char i;

    if ( ( err == ERR_OK ) && ( p != NULL ))
    {
        gPcbLastConnectFromClient = pcb;

        /* Inform TCP that we have taken the data. */
        tcp_recved ( pcb, p->tot_len );
        
        if ( debugFlag & 0x01 )
        {
            printf ( "\r\n p->len %d/%d " , p->len, p->tot_len );
            printf ( "msg:[%s]" , p->payload );																		
        }

        memcpy ( TCP_DataBuf1 , p->payload , p->len );
        TCP_sav_num1 = p->len;
        
       // gPubVal.bLastRs485CmdFromUart = false;
        
        tcp_write(gPcbLastConnectFromClient, TCP_DataBuf1,TCP_sav_num1,0);
        tcp_output(gPcbLastConnectFromClient);
/*
        //DLT645-2007
        if(RxValidDataFrame(TCP_DataBuf,TCP_sav_num))
        {           
           
            ClearDLT645SendBuf();
            MeterCmdAnalysis1(TCP_DataBuf);//645��ʽ������� 
            tcp_write(gPcbLastConnectFromClient, Send645Buf,Send_645Buf_num,0);
            tcp_output(gPcbLastConnectFromClient);
            ClearDLT645RecvBuf();
            //len = tcp_sndbuf(pcb);
            //ClearDLT645SendBuf();
            
        }
        else
        {
           // printf("false");
        }*/
        //ιһ�ι�������ͻ��˶�η���һֱռ��CPU������
       // WDog();
        //gPubVal.iSvrNoCommCnt = 0;

        //gPubVal.bTcpInComm += 10;

        pbuf_free ( p );  //�ͷŸ�TCP��
       

    }

    if ( ( err == ERR_OK )  &&  ( p == NULL ) )
    {
        DBMTR( "\r\n close socket in recv \r\n" );

        tcp_recv ( pcb, NULL );        /* ����TCP�ε�ʱ�Ļص����� */

        // tcp_err ( pcb, cli_conn_err );

        tcp_close ( pcb );
        gPcbLastConnectFromClient = NULL;
    }

    if (err != ERR_OK)
    {
        DBMTR("");
      
    }
    
//    tcp_close ( pcb );                                           /* �ر�������� */

    err = ERR_OK; 
    return err;
}

/*******************************************************************************
*������:
����devTcpSvr_accept()
*����:
*����:
*���:
*˵��:
*******************************************************************************/
static err_t devTcpSvr_accept ( void *arg, struct tcp_pcb *pcb, err_t err )
{
    if ( debugFlag & 0x01 )
        printf ( "\r\n tcp connect accepted.. " );

    tcp_setprio ( pcb, TCP_PRIO_MIN );  /* ���ûص��������ȼ��������ڼ�������ʱ�ر���Ҫ���˺����������*/
    tcp_recv ( pcb, tcpSvr_recv );        /* ����TCP�ε�ʱ�Ļص����� */

   tcp_err ( pcb,   tcpSvr_err );

    //gPcbLastConnectFromClient = pcb;


    //gPubVal.iSvrNoCommCnt = 0;

    //printf("test tcp\n");
    err = ERR_OK;

    return err;
}
/*******************************************************************************
*������:
����taskTcpServer()
*����:
*����:
*���:
*˵��:
*******************************************************************************/
void tcpServerInit ( void )
{
    struct tcp_pcb *pcb;
    err_t ret;
    struct ip_addr ipaddr; 

    printf ( "\r\nstart  tcpServerInit " );

    OSTimeDly ( OS_TICKS_PER_SEC );
  
    pcb = tcp_new();   //����TCP����           

#if 0
    ret = tcp_bind ( pcb, IP_ADDR_ANY, 5000 );        /* �󶨱���IP��ַ�Ͷ˿ں� */
#else

    /* �󶨱���IP��ַ�Ͷ˿ں� */
    //ret = tcp_bind ( pcb, IP_ADDR_ANY,  *(WORD *)&SysParam[ SP_LOCALPORT] ); 
     ret = tcp_bind ( pcb, IP_ADDR_ANY,  5000 );  

    //printf( "\r\ntcp server bind port %d " , *(WORD *) &SysParam[ SP_LOCALPORT] );
#endif

    if ( ret != ERR_OK )
    {
        DBMTR("");
        printf ( "\r\ntcp_bind ret:0x%x" , ret );
    }
    
    pcb = tcp_listen ( pcb );                                       /* �������״̬ */

    if ( pcb == NULL )
    {
        DBMTR("");
        printf ( "\r\ntcp_listen pcb:0x%x" , ( int ) pcb );
    }
    printf ( "\r\ntcp_listen pcb start...." );
    tcp_accept ( pcb, devTcpSvr_accept );                /* ��������������ʱ�Ļص����� */

}


static err_t tcpSvr_err ( void *arg,  err_t err )
{
    printf("\r\n *** tcpSvr_err()**** " );
    
    if (gPcbLastConnectFromClient != NULL)
    {
        tcp_close(gPcbLastConnectFromClient);
        gPcbLastConnectFromClient = NULL;
    }
    return ERR_OK;
}


/*******************************************************************************
*������:
����sendToLastConnectFromClient()
*����:
*����:
*���:
*˵��:
*******************************************************************************/
void sendToLastConnectFromClient ( void * buff , int len )
{
    err_t ret;
    if ( gPcbLastConnectFromClient != NULL )
    {
       // _pl_;
        ret = tcp_write ( gPcbLastConnectFromClient, buff, len , 0 );
        if ( ERR_OK !=  ret )   //��������
        {
            DBMTR("");
            printf("ret :0x%x " ,ret);
            //gPcbLastConnectFromClient = NULL;
        }

        if ( tcp_output ( gPcbLastConnectFromClient ) != ERR_OK )
        {
            DBMTR("");
        }
    }


}



