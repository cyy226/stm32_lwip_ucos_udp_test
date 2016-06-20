/*************************************************************
�ɶ���ջ����������޹�˾   ��Ȩ����

�ļ���:  Gsm.c
��  ��:  �˹���
��  ��:  Gsmģ��
�޶���¼:   

**************************************************************/

#include "Comm.h"
#include "Cdma1x.h"
#include "Gsm.h"
#include "Sms.h"
#include "dev.h"
#include "uart.h"
#include "HW_uart.h"
#include "DLT645.h"
#include "devTcpip.h"
#include "WlModule.h"

//extern BYTE SysParam[];
extern BYTE IpAddrStr[];
extern BYTE PortStr[];
extern BYTE TcpSendDataLen;
extern BYTE SendFrame_Buffer[];
extern BYTE CurTcpLinkId;
extern BYTE Uart0_Buffer[];

extern BYTE SmsContent[];       //  �����ı�����
extern BYTE SmsContent2[];   //  PDU��������

extern BYTE SmsLength;      //  ���ų���
extern BYTE SmsLength2;      //  PDU���ų���

BYTE TmpRecBuf[TCP_RecBufLen] = {0};   // �������ݻ���

//#define TCP_SendBuf   SendFrame_Buffer
#define TCP_SendBuf   Send645Buf

WORD Gsm_RecDataLen = 0;


BYTE Init_Gprs(void)
{
    static BYTE i;
    if (Send_AT_Cmd("AT+CGACT?") != TRUE)
    {
        DebugMsg();
        return FALSE;
    }
    DebugMsg();

    i = (BYTE)GetSignLoc(',', 1);
    
    
    if ((i == 0xFF) || (TCP_RecBuf[i+1] == '0'))
    {
        
        //����������
        if (Send_AT_Cmd("AT+CGDCONT=1,\"IP\",\"CMNET\"") != TRUE)
        {
            DebugMsg();
            return FALSE;
        }
        DebugMsg();

        if (Send_AT_Cmd("AT+CGDCONT?") != TRUE)
        {
            DebugMsg();
            return FALSE;
        }
        DebugMsg();

        //����������
        if (Send_AT_Cmd("AT+CGACT=1,1") != TRUE)
        {
            DebugMsg();
            return FALSE;
        }
        DebugMsg();

    }
    else
    {
        return TRUE;
    }

    if (Send_AT_Cmd("AT+CGPADDR=1") != TRUE)
    {
        DebugMsg();
        return FALSE;
    }
    DebugMsg();
    
    
    
    /*
    
    //��ʼ�������� Profile 1
    if (!Send_AT_Cmd("AT^SICS=1,conType,GPRS0"))
    {
        DebugMsg();
        return FALSE;
    }
    DebugMsg();
    Sleep(20);
    
    if (!Send_AT_Cmd("AT^SICS=1,apn,CMNET"))
    {
        DebugMsg();
        return FALSE;
    }
    
    
    DebugMsg();
    Sleep(20);

    

    //����������Profile
    if (!Send_AT_Cmd("AT^SISS=1,srvType,Socket"))
    {
        DebugMsg();
        return FALSE;
    }
    DebugMsg();
    Sleep(20);

    if (!Send_AT_Cmd("AT^SISS=1,conId,1"))
    {
        DebugMsg();
        return FALSE;
    }
    DebugMsg();
    Sleep(20);
    
    if (!Send_AT_Cmd("AT^SISS=1,address,\"socktcp://listener:8000\""))
    {
        DebugMsg();
        return FALSE;
    }
    DebugMsg();
    Sleep(20);

    
    //�򿪷���
    if (Send_AT_Cmd("AT^SISO=1") != TRUE)
    {
        DebugMsg();
        Clear_Tcp_RecBuf();
        Sleep(200);
        return FALSE;
    }
    DebugMsg();
    */

    
    //��������Profile 0 ��������
    if (!Send_AT_Cmd("AT^SICS=0,conType,GPRS0"))
    {
        DebugMsg();
        return FALSE;
    }
    DebugMsg();
    Sleep(20);
    
    if (!Send_AT_Cmd("AT^SICS=0,apn,CMNET"))
    {
        DebugMsg();
        return FALSE;
    }
    
    DebugMsg();
    Sleep(20);
    

    //��������Profile 0 ��������
    if (!Send_AT_Cmd("AT^SISS=0,srvType,Socket"))
    {
        DebugMsg();
        return FALSE;
    }
    DebugMsg();
    Sleep(20);

    if (!Send_AT_Cmd("AT^SISS=0,conId,0"))
    {
        DebugMsg();
        return FALSE;
    }
    DebugMsg();
    Sleep(20);
    
    /*
    //��ȡ����IP��ַ
    if (!Send_AT_Cmd("AT^SICI?"))
    {
        DebugMsg();
        return FALSE;
    }
    DebugMsg();
    Sleep(20);
    */

    
    
    return TRUE;
}


BYTE Check_GsmNet(BYTE FLed)
{
    static BYTE Ec;

    Ec = 0;

    if (FLed)
    {
        //LED_ALARM = LED_ON;
//        ALM_ON();
    }
    
    while(1)
    {   
        if (Send_AT_Cmd("AT+CREG?") == TRUE)
        {
            DebugMsg();
            if ((Check_string_rxed("+CREG: 0,1") == 1)  // ע���˱�����Ӫ��
              || (Check_string_rxed("+CREG: 0,4") == 1)  // ע������������
              || (Check_string_rxed("+CREG: 0,5") == 1)  // δ֪״̬
            )
            {
                if (FLed)
                {
                    //LED_ALARM = LED_OFF;
                    //ALM_OFF();
               
                }
                return TRUE;
            }
        }
      
        OSTimeDlyHMSM(0,0,2,0);

        if (FLed)
        {
           // ALM_SW();
        }

        Ec ++;
        if (Ec > 100)
        {
            return FALSE;
        }
    }

    //return TRUE;
}

BYTE ATCmd_OpenLink_G(void)
{
    static BYTE i;
    Clear_Tcp_RecBuf();

    Send_String("AT^SISO=0\r\n");

    i  = 0;
    while(Check_string_rxed("OK") == 0)
    {
        Sleep(200); 
        if(Check_string_rxed("ERROR") == 1) 
        {
            DebugMsg();
            Clear_Tcp_RecBuf();
            Sleep(20);
            return FALSE;
        }
        WDog();

        if((Check_string_rxed("rejected") == 1) ||
            (Check_string_rxed("unreachable") == 1))
        {
            DebugMsg();
            Clear_Tcp_RecBuf();
            Sleep(20);
            return FALSE;
        }
        WDog();

        
        i ++;
        if (i > 150)  // 30s
        {
            DebugMsg();
            Clear_Tcp_RecBuf();
            CmdTimeOutHandle();
            return TIMEOUT;
        }
    }
    DebugMsg();
    Clear_Tcp_RecBuf();


    return TRUE;
    
}


BYTE ATCmd_IpOpen_G(void)
{
    static BYTE i;

    Clear_Tcp_RecBuf();

    Send_String("AT^SISS=0,address,\"socktcp://");
        
    //������IP
    //printf("%s\n",&SysParam[SP_SERVERIP]);
    IpToStr((BYTE *)&SysParam[SP_SERVERIP]);
    Send_String(IpAddrStr);
    Send_a_byte(':');
        
    //�������˿�  
    PortToStr((BYTE *)&SysParam[SP_SERVERPORT]);
    Send_String(PortStr);
    Send_a_byte('"');
        
    Send_a_byte(0x0D);
    Send_a_byte(0x0A);
    
    i  = 0;
    while(Check_string_rxed("OK") == 0)
    {
        Sleep(200); 
        if(Check_string_rxed("ERROR") == 1) 
        {
            DebugMsg();
            Clear_Tcp_RecBuf();
            Sleep(200);
            return FALSE;
        }
 
        i ++;
        if (i > 50)  // 10s
        {
            CmdTimeOutHandle();
            return TIMEOUT;
        }
    }
    DebugMsg();
    Clear_Tcp_RecBuf();


    //�򿪷���
    if (ATCmd_OpenLink_G() != TRUE)
    {
        DebugMsg();
        
        Clear_Tcp_RecBuf();
        Sleep(200);
        return FALSE;
    }
    /*
    if (Send_AT_Cmd("AT^SISO=0") != TRUE)
    {
        DebugMsg();
        Clear_Tcp_RecBuf();
        Sleep(200);
        return FALSE;
    }
    DebugMsg();
    */


    //��ȡ����IP��ַ
    if (!Send_AT_Cmd("AT^SICI?"))
    {
        DebugMsg();
        return FALSE;
    }
    DebugMsg();
    Sleep(20);

    
    return TRUE;
}


BYTE ATCmd_SendData_G(void)
{
    static BYTE i; 
    /*
    AT^SISW=0,20
    ^SISW: 0��20��20 �Զ��ϱ�����Ϣ����ʾ�û�����20 ���ַ�
    <�û�����������20 ���ַ�>
    OK
    */
    //DebugStr("--1\r\n");
    
    Clear_Tcp_RecBuf();
    
    Send_String("AT^SISW=");
    Send_a_byte(CurTcpLinkId+0x30);
    Send_a_byte(',');
    
    //TcpSendDataLen
    if(TcpSendDataLen>99)
    {
        Send_a_byte(TcpSendDataLen/100 + 0x30);
        i = TcpSendDataLen%100;
        Send_a_byte(i/10 + 0x30);
        Send_a_byte(i%10 + 0x30);
    }
    else if (TcpSendDataLen>9)
    {
        Send_a_byte(TcpSendDataLen/10 + 0x30);
        Send_a_byte(TcpSendDataLen%10 + 0x30);
    }
    else
    {
        Send_a_byte(TcpSendDataLen + 0x30);
    }
    Send_a_byte(0x0D);
    Send_a_byte(0x0A);

    i = 0;
    while(Check_string_rxed("^SISW") == 0)
    {
        Sleep(200); 
        i ++;
        if (i > 50)  // 10s
        {
            DebugMsg();
            Clear_Tcp_RecBuf();
            CmdTimeOutHandle();
            return TIMEOUT;
        }
    }

    //DebugStr("--2\r\n");
    //DebugMsg();
    //Clear_Tcp_RecBuf();

    // д����
    for (i=0;i<TcpSendDataLen;i++)
    {
        Send_a_byte(TCP_SendBuf[i]);
    }

    i = 0;
    while(Check_string_rxed("OK") == 0)
    {
        Sleep(200); 
        i ++;
        if (i > 50)  // 10s
        {
            DebugMsg();
            Clear_Tcp_RecBuf();
            CmdTimeOutHandle();
            return TIMEOUT;
        }
    }
    
    //DebugMsg();
    Clear_Tcp_RecBuf();
    return TRUE;
}


BYTE QueryTcpData_G(WORD DataLen)
{
    static WORD i,len;
    static WORD retlen;  // ʵ�ʷ��ص����ݳ���
    static WORD remlen;  // ʣ�����ݳ���
    static WORD TemStratLoc;
    static WORD DataLoc;

    Clear_Tcp_RecBuf();

    if (DataLen > 1200)
    {
        return FALSE;
    }

    TemStratLoc = 0;
    remlen = DataLen;

ReRead:    
    WDog();
    
    //��ѯ���������Ƿ�������
    Send_String("AT^SISR=0,");
    if (remlen > 999)
    {
        Send_a_byte(remlen/1000 + 0x30);
        i = remlen%1000;
        Send_a_byte(i/100 + 0x30);
        i = i%100;
        Send_a_byte(i/10 + 0x30);
        Send_a_byte(i%10 + 0x30);
    }
    else if(remlen>99)
    {
        Send_a_byte(remlen/100 + 0x30);
        i = remlen%100;
        Send_a_byte(i/10 + 0x30);
        Send_a_byte(i%10 + 0x30);
    }
    else if (remlen>9)
    {
        Send_a_byte(remlen/10 + 0x30);
        Send_a_byte(remlen%10 + 0x30);
    }
    else
    {
        Send_a_byte(remlen + 0x30);
    }
    Send_a_byte(0x0D);
    Send_a_byte(0x0A);
    
    i = 0;
    while(Check_string_rxed("^SISR:") == 0)
    {
        Sleep(200); 
        if (Check_string_rxed("ERROR"))
        {
            DebugMsg();
            Clear_Tcp_RecBuf();
            return FALSE;
        }
        i++;
        if (i > 50)  // 10s
        {
            DebugMsg();
            Clear_Tcp_RecBuf();
            CmdTimeOutHandle();
            return FALSE;
        }
    }

    //DebugMsg();

    RemoveFrontData();
    
    i = GetSignLoc(',', 1);
    if (i == 0xFFFF)
    {
        DebugStr("Err1\r\n");
        return FALSE;
    }

    //��ȡTCP BUF�����ݳ���
    DataLoc = GetSignLoc(0x0D, 1);
    if (DataLoc == 0xFFFF)
    {
        DebugStr("Err2\r\n");
        return FALSE;
    }

    len = DataLoc - i - 1;
    if ((len > 4) || (len == 0))
    {
        //DebugStr("Err3\r\n");
        return FALSE;
    }

    retlen = 0; // ʵ�ʷ��ص�������
    if (!StrToWord(&TCP_RecBuf[i+1],len,&retlen))
    {
        DebugStr("Err4\r\n");
        return FALSE;
    }

    if ((retlen > TCP_RecBufLen) || (retlen == 0))
    {
        //DebugStr("Err5\r\n");
        return FALSE;
    }
 
    i = 0;
    while(Check_string_rxed("OK") == 0)
    {
        Sleep(200); 
        i ++;
        if (i > 50)  // 10s
        {
            DebugMsg();
            Clear_Tcp_RecBuf();
            CmdTimeOutHandle();
            return FALSE;
        }
    }

    
    if (DataLen == 0)
    {
        Gsm_RecDataLen = retlen;
    }
    else
    {
        mcpy(&TmpRecBuf[TemStratLoc],&TCP_RecBuf[DataLoc+2],retlen);
        remlen -= retlen;
        TemStratLoc += retlen;
        if (remlen > 0)
        {
            goto ReRead;
        }
        else
        {
            TCP_RecBuf[0] = 0x0A;
            mcpy(&TCP_RecBuf[1],TmpRecBuf,DataLen);
        }
    }
   
    //DebugMsg();
    //Clear_Tcp_RecBuf();
    
    return TRUE;
}

BYTE ATCmd_CloseIp_G(void)
{
    //�ر�����
    if (Send_AT_Cmd("AT^SISC=0") != TRUE)
    {
        DebugMsg();
        Clear_Tcp_RecBuf();
        Sleep(200);
        return FALSE;
    }
    DebugMsg();
    Clear_Tcp_RecBuf();

    //�رշ���

    return TRUE;
}

BYTE NewMsg_G(void)
{
    if (Send_AT_Cmd("AT+CMGL") != TRUE)
    {
        return FALSE;
    }

    if (Check_string_rxed("+CMGL:"))
    {
        return TRUE;
    }
    
    Clear_Sms_buf();
    return FALSE;    
}

BYTE Read_SMS_G(void)
{
    
    return FALSE;
}

BYTE Set_Pdu_SMS()
{
    if (Send_AT_Cmd("AT+CGSMS=1") != TRUE)
    {
        return FALSE;
    }
    
    if (Send_AT_Cmd("AT+CMGF=0") != TRUE)
    {
        return FALSE;
    }

    return Send_AT_Cmd("AT+CNMI=1,2,0,0,0");
    
}


BYTE Send_Pdu_SMS(void)
{  
    static BYTE Length,i,n;
        
    if (Set_Pdu_SMS() == FALSE)
    {
        return FALSE;
    }

    DebugMsg();
    Clear_Tcp_RecBuf();

    n = ChangeContent();
    
    Clear_Sms_buf();                      //��մ��ڻ�����

    Send_String("AT+CMGS=");              //����AT+CMGS���� 

    //Length = 15 + SmsLength;
    Length = 15 + n;
    
    Send_a_byte(Length/10 + 0x30);
    Send_a_byte(Length%10 + 0x30);

    Send_a_byte(0x0D);
    Send_a_byte(0x0A);
  
    while(Check_string_rxed(">") == 0)               //�յ�">"���������SMS����
    {
        Sleep(50);
        if (Check_string_rxed("ERROR") == 1)     //���յ�ERROR������ŷ���ʧ�� 
        {
            Sleep(200);
            ////WDog();  
        	return FALSE;
        }
    }
    Send_String(SmsContent2); 
    Send_a_byte(0x1A);                      //�������ݽ�����־
   
    Sleep(1000);
    i = 0;
    while(Check_string_rxed("OK") == 0)         //�յ�OK����ŷ��ͳɹ�
    {
        Sleep(200);
        if(Check_string_rxed("ERROR") == 1)      //�յ�ERROR����ŷ���ʧ��
        {
            DebugMsg();
            Clear_Tcp_RecBuf();
    
            Sleep(1000);
            ////WDog();  
            return FALSE;
        }

        ////WDog();
        i ++;
        if (i > 100)  // 20s
        {
            return FALSE;
        }
    }

    DebugMsg();
    Clear_Tcp_RecBuf();

    return TRUE;
}

