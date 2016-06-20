/*************************************************************
�ɶ���ջ����������޹�˾   ��Ȩ����

�ļ���:  Cdma1x.c
��  ��:  �˹���
��  ��:  Cdmaģ��
�޶���¼:   

**************************************************************/


#include "includes.h"
#include "driverlib/uart.h"
#include "Hw_uart.h"
#include "Comm.h"
#include "Cdma1x.h"
#include "Sms.h"
#include "Dev.h"
#include "Scom.h"
#include "DLT645.h"
#include "devTcpip.h"
#include "WlModule.h"
extern BYTE SendFrame_Buffer[];


extern BYTE TcpSendDataLen;

extern BYTE Uart0_Buffer[];
extern WORD Uart0_Sav_Num;

extern BYTE SysTime[];
//extern  char *SysParam;

extern BYTE SmsLength;
extern BYTE SmsContent[];
extern BYTE SmsNumber[];
extern WORD Sms_buf_addr;

extern BYTE IpAddrStr[];
extern BYTE PortStr[];

//extern BYTE Sms_Index;
extern BYTE Sms_IndexStr[];
extern BYTE NeedReConnect;
extern BYTE CurTcpLinkId;

extern uchar NetModulState;


BYTE DataModeEnable = 0;

extern BYTE CurTcpLinkId;

BYTE Test_AtCmd(void)
{
    static BYTE Ec;

    Ec = 0;

    while(1)
    {
        if (Send_AT_Cmd("AT") == TRUE)
        //if (Send_AT_Cmd("AT+COPN") == TRUE)
        {
            DebugMsg();
            //if ((Check_string_rxed("CHINA  MOBILE") == 1) || (Check_string_rxed("CHN-CUGSM") == 1))
            {
              
                return TRUE;
            }
        }

        DebugMsg();

        Sleep(2000);
    
 //       ALM_SW();

        Ec ++;
        if (Ec > 100)
        {
            return FALSE;
        }
    }

//    return FALSE;
}



BYTE Init_Cdma1x(void)
{
    static BYTE ErrCnt;

    //Init TCP/UDP 
    ErrCnt = 0;
    while(1)
    {
        if (ATCmd_InitIp() != TRUE)
        {
            DebugMsg();
            
            ErrCnt ++ ;
            if (ErrCnt > RETRY_COUNT) 
            {
                return FALSE;
            }

            //5�������
            Sleep(5000);       
        }
        else
        {
            break;
        }
    }

    
    DebugMsg();
    //Sleep(200);

    // ���������������ͻ��˵�����
    ErrCnt = 0;
    while(1)
    {
        if (ATCmd_IpListen() != TRUE)
        {
            ErrCnt ++;
            if (ErrCnt > RETRY_COUNT)
            {
                return FALSE;
            }
            Sleep(5000);           
        }
        else
        {
            break;
        }
    }

    DebugMsg();
    Clear_Sms_buf();
    //Sleep(200);
    
    //�����ϱ�����
    if (Send_AT_Cmd("AT^IPDATMODE=1") != TRUE)
    {
        DebugMsg();
        Clear_Sms_buf();
        return FALSE;
    }
    
    DebugMsg();
    Clear_Sms_buf();
    
    
    
    return TRUE;
}





BYTE Send_AT_Cmd(BYTE *Cmd)
{
    static BYTE i;
    Clear_Tcp_RecBuf();
 
    Send_String(Cmd); 
    
    Send_a_byte(0x0D);
    Send_a_byte(0x0A);
 
    i  = 0;
    while(Check_string_rxed("OK") == 0)
    {
        Sleep(200);
        if(Check_string_rxed("ERROR") == 1) 
        {
            Sleep(200);
            return FALSE;
        }
        //WDog();
        i ++;
        if (i > 100)  // 20s
        {
            
            CmdTimeOutHandle();
            return TIMEOUT;
        }
    }

    //WDog();
    return TRUE;
}


BYTE Hang_Call(void)
{
    if (Sms_BUFFER_LEN >= 4)
    {
        if (Check_string_rxed("RING") == 1)
        {
            //DebugMsg();
            Send_AT_Cmd("AT+CHV");
            Clear_Tcp_RecBuf();
            return TRUE;
        }

        if (Check_string_rxed("^MODE:0") == 1) // ������ϻ����޿�
        {
            Clear_Tcp_RecBuf();
            return FALSE;
        }
    }

    return FALSE;
}



//ȡģ��ʱ��
BYTE Get_Modul_Time(void)
{
    static BYTE loc;
    if (Send_AT_Cmd("AT^TIME"))
    {
        loc = (BYTE)GetSignLoc(':', 1);
        SysTime[0] = (Sms_Buffer[loc+3] - 0x30)*16 + (Sms_Buffer[loc+4] - 0x30);
        SysTime[2] = (Sms_Buffer[loc+6] - 0x30)*16 + (Sms_Buffer[loc+7] - 0x30);
        SysTime[3] = (Sms_Buffer[loc+9] - 0x30)*16 + (Sms_Buffer[loc+10] - 0x30);
        SysTime[4] = (Sms_Buffer[loc+12] - 0x30)*16 + (Sms_Buffer[loc+13] - 0x30);
        SysTime[5] = (Sms_Buffer[loc+15] - 0x30)*16 + (Sms_Buffer[loc+16] - 0x30);
        SysTime[6] = (Sms_Buffer[loc+18] - 0x30)*16 + (Sms_Buffer[loc+19] - 0x30);
        return TRUE;
    }

    return FALSE;
}



BYTE Check_CdmaNet(BYTE FLed)
{
    static BYTE Ec;

    if (FLed)
    {
        //ALM_ON();
    }

    Ec = 0;
    while(1)
    {
        if (Check_string_rxed("^MODE:2") == 1)   // CDMA ģ��
        {
            //SysParam[SP_MODTYPE] = CDMA_MC323;
            //DebugStr("Checked Cdma module\r\n");
            //ALM_OFF();
            break;
        }

  
        Sleep(2000);
        WDog();
        
        if (FLed)
        {
          //  ALM_SW();
        }

        DebugMsg();

        Ec ++;
        if (Ec > 60) 
        {
            return FALSE;
        }
    }

    
    Ec = 0;
    while(1)
    {   
        if (Send_AT_Cmd("AT+CREG?") == TRUE)
        {
            //DebugMsg();
            if ((Check_string_rxed("+CREG:0,1") == 1)  // ע���˱�����Ӫ��
              || (Check_string_rxed("+CREG:0,5") == 1)  // ע������������
            )
            {
                if (FLed)
                {
                    //LED_ALARM = LED_OFF;
//                    ALM_OFF();
                }
                return TRUE;
            }
        }

        Sleep(2000);
      

        if (FLed)
        {
            //LED_ALARM = !LED_ALARM;
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


// ���ϱ��ź�ǿ��
void DisableRep(void)
{
    Send_AT_Cmd("AT^RSSIREP=0");
}


BYTE NewMsg_C(void)
{
    static BYTE l1,l2;

    #if 0
    //��ʹ���Զ��ϱ�
    if (Sms_BUFFER_LEN >= 5)   // �Զ��ϱ�����Ϣ��
    {
        if (Check_string_rxed("+CMTI") == 1)
        {
            loc = GetSignLoc(',',1);
            if (loc == 0xFF)
            {
                return FALSE;
            }

            Sms_Index = Sms_Buffer[loc + 1] - 0x30;
            //��ö��Ŵ洢λ��
            //mset(Sms_IndexStr, 0, 3);

            DebugMsg();
            Clear_Tcp_RecBuf();
            
            return TRUE;
        }
    }
    #endif

    //��ѯ�Ѿ��ﵽ����Ϣ
    if (Send_AT_Cmd("AT^HCMGL=0") != TRUE)
    {
        return FALSE;
    }

    if (Check_string_rxed("^HCMGL:") != 1)
    {
        return FALSE;
    }
    
    
    l1 = (BYTE)GetSignLoc(':',1);
    if (l1 == 0xFF)
    {
        return FALSE;
    }

    l2 = (BYTE)GetSignLoc(',',1);
    if (l2 == 0xFF)
    {
        return FALSE;
    }

    if ((l2 - l1) > 3)
    {
        return FALSE;
    }

    //Sms_Index = Sms_Buffer[loc + 1] - 0x30;
    //��ö��Ŵ洢λ��
    mset(Sms_IndexStr, 0, 3);
    mcpy(Sms_IndexStr,&Sms_Buffer[l1 + 1],l2-l1-1);
    

    DebugMsg();
    Clear_Tcp_RecBuf();

    return TRUE;
}

//�����ı�ģʽ
BYTE Set_Txt_SMS(void)
{
    //return Send_AT_Cmd("AT^HSMSSS=0,0,1,0");
    return Send_AT_Cmd("AT^HSMSSS?");
}


//���Ͷ���
BYTE Send_Txt_SMS(BYTE *number,BYTE *content)
{  
    static BYTE i;

    if (Set_Txt_SMS() == FALSE)
    {
        return FALSE;
    }

    DebugMsg();
    Clear_Tcp_RecBuf();                            //��մ��ڻ�����

    Send_String("AT^HCMGS=");             

    Send_a_byte('"');

    //Send_String("+86");
    Send_String(number);

    Send_a_byte('"');

    Send_a_byte(0x0D);
    Send_a_byte(0x0A);

    i = 0;
    while(Check_string_rxed("> ") == 0)               //�յ�">"���������SMS����
    {
        Sleep(200);
    	if (Check_string_rxed("ERROR") == 1)     //���յ�ERROR������ŷ���ʧ�� 
        {
            Sleep(200);
            //WDog();  
            return FALSE;
    	}

        //WDog();  
        i ++;
        if (i > 250)  // 20s
        {
            CmdTimeOutHandle();
            return TIMEOUT;
        }
    }
    //DebugMsg();
    
    Sleep(200);
    Send_String(content); 
    Send_a_byte(0x1A);                      //�������ݽ�����־ CTRL-Z

    Sleep(1000);

    i = 0;
    while(Check_string_rxed("OK") == 0)         //�յ�OK����ŷ��ͳɹ�
    {
        Sleep(200);
    	if(Check_string_rxed("ERROR") == 1)      //�յ�ERROR����ŷ���ʧ��
        {
            Sleep(1000);

            return FALSE;
        }

        //WDog();  
        i ++;
        if (i > 250)  // 20s
        {
            CmdTimeOutHandle();
            return TIMEOUT;
        }
    }

    // ^HCMGSS ���ͳɹ�
    // ^HCMGSF ����ʧ��
    i = 0;
    while((Check_string_rxed("^HCMGSS") == 0) &&
            (Check_string_rxed("^HCMGSF") == 0))
    {
        Sleep(200);
        //WDog();  
        i ++;
        if (i > 250)  // 20s
        {
            CmdTimeOutHandle();
            return TIMEOUT;
        }
    }

    
    if(Check_string_rxed("^HCMGSS") == 1)     // �ɹ�
    {
        return TRUE;
    }

    return FALSE;
}



BYTE Read_SMS(void)
{
    static BYTE i;
    Clear_Tcp_RecBuf();

    Send_String("AT^HCMGR=");
    //Send_a_byte(Sms_Index+0x30);
    for (i=0;i<3;i++)
    {
        if (Sms_IndexStr[i] != 0)
        {
            Send_a_byte(Sms_IndexStr[i]);
        }
    }
    Send_a_byte(0x0D);
    Send_a_byte(0x0A);

    i = 0;
    while(Check_string_rxed("OK") == 0)
    {
        Sleep(200); 
    	if(Check_string_rxed("ERROR") == 1)
        {
            Sleep(100);
            return FALSE;
    	}

        //WDog();

        i ++;
        if (i > 250)  // 20s
        {
            CmdTimeOutHandle();
            return TIMEOUT;
        }
    }

    return TRUE;
}






// ��ȡ���غ�DNS IP��ַ
BYTE GetIpAddress(void)
{
    static BYTE i,j;
    
    i = (BYTE)GetSignLoc(':', 1);

    if (i == 0xFF)
    {
        return FALSE;
    }
        
    if (Sms_Buffer[i+1] == '0')   // û�г�ʼ��
    {
        return FALSE;
    }

    j = (BYTE)GetSignLoc(',', 2);
    if (j == 0xFF)
    {
        return FALSE;
    }

    //IP ��ַ����Ҫ���»㱨������
    IpToStr((BYTE *)&SysParam[SP_LOCALIP]);
    if (!mcmp(IpAddrStr,&Sms_Buffer[i+3],j-i-3))
    {
 
        DebugStr("IP Changed\r\n"); 

        NeedReConnect = 1;
        mset(IpAddrStr,0,16);
        mcpy(IpAddrStr,&Sms_Buffer[i+3],j-i-3);
        if (!StrToIpAdd((BYTE *)&SysParam[SP_LOCALIP]))
        {
            return FALSE;
        }
    }
    
    
    return TRUE;
}


void WaitModuleNormal(void)
{
    static BYTE t;

    t = 0;
    //��Ҫ�ȴ�ģ��AT����ָ�����
    while(1)
    {
        if (Send_AT_Cmd("AT") == TRUE)
        {
            DebugMsg();
            Clear_Tcp_RecBuf();
            break;
        }
        DebugMsg();
        Clear_Tcp_RecBuf();

        Sleep(2000);
     
        t++;
        if (t>150)   // 5����û�лָ���λ
        {
            //SysReset();
            NetModulState =0;//InitState
        }
    }
        
}




BYTE ATCmd_InitIp(void)
{
    if (Send_AT_Cmd("AT^IPINIT?") != TRUE)
    {
        DebugMsg();
        Clear_Tcp_RecBuf();
        return FALSE;
    }

    if (GetIpAddress())
    {
        DebugMsg();
        Clear_Tcp_RecBuf();
        return TRUE;
    }

    DebugMsg();
    Clear_Tcp_RecBuf();
    
    if (Send_AT_Cmd("AT^IPINIT=,\"card\",\"card\"") != TRUE)
    {
        DebugMsg();
        Clear_Tcp_RecBuf();
        return FALSE;
    }
        
    DebugMsg();
    Clear_Tcp_RecBuf();

    if (Send_AT_Cmd("AT^IPINIT?") != TRUE)
    {
        return FALSE;
    }
    
    if (!GetIpAddress())
    {
        return FALSE;
    }

    DebugMsg();
    Clear_Tcp_RecBuf();
    
    return TRUE;
}

BYTE ATCmd_CloseIp(void)
{
    static BYTE i;
    
    //���˳�����ģʽ
    if (DataModeEnable == 1)
    {
        ATCmd_EndDataMode();
    }

    Send_String("AT^IPCLOSE=");
    Send_a_byte(CurTcpLinkId+0x30);

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
        //WDog();
        i ++;
        if (i > 50)  // 10s
        {
            CmdTimeOutHandle();
            return TIMEOUT;
        }
    }
    DebugMsg();
    Clear_Tcp_RecBuf();

    return TRUE;
}

BYTE ATCmd_SendData(BYTE *Data, BYTE Len)
{
    static BYTE i;
        
    for (i=0;i<Len;i++)
    {
        Send_a_byte(Data[i]);
    }

    return TRUE;
}


//��������͸��, ֻ������һ�����ӵ�͸��
BYTE ATCmd_StartDataMode(void)
{
    static BYTE i;
    
    Clear_Tcp_RecBuf();

    Send_String("AT^IPENTRANS=");

    Send_a_byte(CurTcpLinkId + 0x30);

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
            DataModeEnable = 0;
            return FALSE;
        }
        //WDog();
        i ++;
        if (i > 50)  // 10s
        {
            DataModeEnable = 0;
            return FALSE;
        }
    }
    
    
    DataModeEnable = 1;
    return TRUE;
}


BYTE ATCmd_EndDataMode(void)
{
    static BYTE i;

    DataModeEnable = 0;
    
    i  = 0;
    while(i<RETRY_COUNT)
    {
        Sleep(1000);    // ������ʱ�������˲�����
        Send_String("+++");  // ����͸������ 
        Sleep(1000);

        //WDog();

        if (Send_AT_Cmd("AT") == TRUE)
        {
            DebugMsg();
            Clear_Tcp_RecBuf();
            return TRUE;
        }
        
        i ++;
    }
    
    DebugMsg();
    Clear_Tcp_RecBuf();
    return FALSE;
}

BYTE ATCmd_IpOpen(void)
{
    static BYTE i;
    
    //AT^IPOPEN=1,"TCP","129.11.18.8",10000,9000
    //if (Send_AT_Cmd("AT^IPOPEN=1,\"TCP\",\"222.212.14.20\",5000,9000") != TRUE)
    Clear_Tcp_RecBuf();

    Send_String("AT^IPOPEN=1,\"TCP\",\"");
        
    //������IP
    IpToStr((BYTE *)&SysParam[SP_SERVERIP]);
    Send_String(IpAddrStr);
    Send_String("\",");
        
    //�������˿�
    PortToStr((BYTE *)&SysParam[SP_SERVERPORT]);
    Send_String(PortStr);
    Send_a_byte(',');
    
    //���ض˿� ͬ �������˿�
    Send_String(PortStr);
    
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
        //WDog();
        i ++;
        if (i > 50)  // 10s
        {
            CmdTimeOutHandle();
            return TIMEOUT;
        }
    }
    DebugMsg();
    Clear_Tcp_RecBuf();

    
    return TRUE;
}



BYTE ATCmd_IpListen(void)
{
    static BYTE i;
    
    Clear_Tcp_RecBuf();

    if (Send_AT_Cmd("AT^IPLISTEN?") != TRUE)
    {
        return FALSE;
    }
  
    if (Check_string_rxed("NULL") == 0)
    {
        DebugMsg();
        Clear_Tcp_RecBuf();
        return TRUE;
    }

    DebugMsg();
    Clear_Tcp_RecBuf();

    Send_String("AT^IPLISTEN=\"TCP\",");
    PortToStr((BYTE *)&SysParam[SP_LOCALPORT]);
    Send_String(PortStr);

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
        //WDog();
        i ++;
        if (i > 50)  // 10s
        {
            CmdTimeOutHandle();
            return TIMEOUT;
        }
    }
    
    DebugMsg();
    Clear_Tcp_RecBuf();

    
    return TRUE;
}


BYTE Get_Sms_Content(void)
{
    static BYTE i;
    i = (BYTE)GetSignLoc(0x0A,2);
    if (i == 0xFF)
    {
        return FALSE;
    }

    if (Check_string_rxed("OK") == 1)
    {
        SmsLength = (BYTE)Sms_buf_addr - i - 8;
        if (SmsLength >= SMSMSGCOUNT)
        {
            return FALSE;
        }
        mcpy(SmsContent,&Sms_Buffer[i+1],SmsLength);

        #if 0
        Uart1_SendByte(SmsLength/100+0x30);
        Uart1_SendByte(SmsLength%100/10+0x30);
        Uart1_SendByte(SmsLength%10+0x30);
        Uart1_SendStr(SmsContent,SmsLength); 
        #endif
    }

    return TRUE;
}

BYTE Get_Phone_Number(void)
{
    static BYTE i,j;
    i = (BYTE)GetSignLoc(':',1);
    if (i == 0xFF)
    {
        return FALSE;
    }

    //check phone number
    for (j=0;j<11;j++)   
    {
        if ((Sms_Buffer[i+j+1] < '0') || (Sms_Buffer[i+j+1] > '9'))
        {
            return FALSE;
        }
    }

    //if (Sms_Buffer[i+1] == '"')
    {
        mcpy(SmsNumber,&Sms_Buffer[i+1],11);
        SmsNumber[11] = 0;

        
        DebugStr((char *)SmsNumber); 
        
    }

    return TRUE;
}


BYTE ATCmd_SendDataEx(void)
{
    static BYTE i,h,l;
    static WORD t;
    
    Clear_Tcp_RecBuf();
                   
    Send_String("AT^IPSENDEX=");
    Send_a_byte(CurTcpLinkId+0x30);
    Send_String(",1,\"");
    
    for (i=0;i<TcpSendDataLen;i++)
    {
        h = HexToBcd(TCP_SendBuf[i]/0x10);
        Send_a_byte(h);
        l = HexToBcd(TCP_SendBuf[i]%0x10);
        Send_a_byte(l);
    }
    
    Send_a_byte('"');
    Send_a_byte(0x0D);
    Send_a_byte(0x0A);

    t  = 0;
    while(Check_string_rxed("OK") == 0)
    {
        Sleep(10); 
        //WDog();
        
        if(Check_string_rxed("ERROR") == 1) 
        {
            Sleep(200);
            DebugMsg();
            Clear_Tcp_RecBuf();
    
            return FALSE;
        }
        
        t ++;
        if (t > 3000)  // 30s
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


