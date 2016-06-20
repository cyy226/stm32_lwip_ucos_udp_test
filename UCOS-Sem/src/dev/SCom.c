/*************************************************************
�ɶ���ջ����������޹�˾   ��Ȩ����

�ļ���:  Scom.c
��  ��:  �˹���
��  ��:  ͬ������ͨѶģ��
�޶���¼:   

**************************************************************/

#include <devAdc.h>
#include "Comm.h"
#include "Crc.h"
#include "SCom.h"

#include "Em485.h"
#include "dev.h"
#include "devTimer.h"
#include "includes.h"
#include "Tcpip.h"
#include "DLT645.h"
#include "devTcpip.h"
#include "sms.h"
#include "cdma1x.h"
#include "Gsm.h"
#include "SwUpdate.h"

/*
֡��ʼ�� 68H
֡����   L      ֡���ȵ�λ
֡����   H      ֡���ȸ�λ
֡��ʼ�� 68H
��ַ��   Address   //5�ֽ�
������   Control
������   Type
������   Data block
У����   CS    CRC16��ʽУ��
������   16H
*/
WORD SendLength = 0;   // ͸�����ݳ��� 

BYTE NeedSaveParam = 0;
BYTE NeedReboot = 0;
BYTE NeedRebootChk = 0;


WORD DataLength = 0;
WORD StartLoc   = 0;
WORD Crc;
BYTE HeartPackArrived = 0;

BYTE ServerLogin = 0;
BYTE TcpSendDataLen = 0;

extern BYTE  SignPower;
extern BYTE NeedReConnect;
extern BYTE NeedSleep ;        // �Ƿ�˯��   �ɼ������������紦������״̬

extern BYTE CurTcpLinkId;
extern BYTE Gsm_RecDataLen;

extern BYTE SwUpdating ;

//void GetDevEnvInfo(void);
void SysSleep()
{
    //ע������
    
    Close_TcpIpLink();

    //�ر�ģ��-- �ػ�

    //Send_AT_Cmd("AT^MSO");
    //^MSO

    #ifdef CDMA_SRVMODE
    MOD_POWEROFF(); // �ر�ģ���Դ
    //DisableUart0();
    #endif
    
    DebugStr("Sleeped.\r\n"); 

}

//#ifdef CDMA_SRVMODE
#if 0
//���������ģʽ��ֻ����������·���������������
//10���½��ʱ��10�������ݳ�ʱ(����½�ɹ���10����û���µ������򷵻�)
void ServerHandle(void)
{
    static WORD time;
    static BYTE Frame;
    static BYTE Logined;


    time = 0;
    Logined = 0;
    while(1)
    {
        Sleep(200);
        WDog();
        
        Frame  = RecValidTcpFrame();
        if (Frame == DEVFRAME)
        {
            Logined = ServerLogin;
            ClearDLT645SendBuf();
            HandleDevFrame(); 
            time = 0;           
            Clear_Tcp_RecBuf(); 
            
            if (Logined != ServerLogin)
            {
                if (ServerLogin == 0) // logout
                {
                    NeedReConnect = 0;
                    Sleep(2000);   // ���ݿ��ܻ�û�з���ȥ
                    break;
                }
            }
        }
        else if (Frame == METERFRAME)
        {
            //if (ServerLogin == 1)  // ����½Ҳ���Բɼ��������
            {
                //HandleMeterFrame();
                if(RxValidDataFrame(TCP_RecBuf,TCP_RecLen) )
                {
                    //MeterCmdAnalysis();
                    ClearDLT645SendBuf();
                    MeterCmdAnalysis1(TCP_RecBuf);
                    TcpSendDataLen=Send_645Buf_num;
                    Send_Tcp_Data();
                    
                }
                NeedReConnect = 1;
                time = 0;
                Clear_Tcp_RecBuf(); 
            }
        }
        else if (Frame == MODBUSFRAME)
        {
            //HandleModBusFrame();
            NeedReConnect = 0;
            time = 0;
            Clear_Tcp_RecBuf(); 
        }
        else if (Frame == LINKDOWN)
        {
  
            DebugStr("Link Down\r\n"); 

            NeedReConnect = 0;
            break;
        }
        else
        {
            time+=1;

            if (Check_string_rxed("remote close") == 1)
            {
                DebugStr("Remote closed\r\n"); 
                Clear_Tcp_RecBuf(); 
                break;
            }
            
        }
        
        if (((time/10) > CMD_TIMEOUT) && (ServerLogin == 0))  // û�����ݻ��½���˳�����ģʽ ���ر�����
        {
            DebugStr("Time Out1\r\n"); 
            break;      
        }

        if ((time/10) > 120)  // ��½��120��û��������ر�����
        {
            //������ͨ����·���ˣ���Ҫ�����ϱ�������
            
            DebugStr("Time Out2\r\n"); 

            NeedReConnect = 1;
            break;
        }
    }


    Close_TcpIpLink();
    ServerLogin = 0;

    if (NeedSaveParam)
    {
        SaveSysParam();
        WDog();
        Sleep(1000);
        //SysReset();
    }

    if (NeedReboot)
    {
        SysReset();
    }

}
#else

BYTE HandleTcpFrame(BYTE Frm)
{
    static WORD time;
    static BYTE Frame;
    static BYTE Logined;

    Frame = Frm;
    Logined = 0;
    while(1)
    {
        Sleep(100);
        WDog();
        
        
        if (Frame == DEVFRAME)
        {
            DebugStr("DEVFRAME\r\n");
            Logined = ServerLogin;
            HandleDevFrame(); 
            Clear_Tcp_RecBuf(); 
            
            if ((Logined != ServerLogin) || (NeedSleep == 1))
            {
                if (ServerLogin == 0) // logout
                {
                    return TRUE;
                }
            }
        }
        else if (Frame == METERFRAME)
        {
            DebugStr("METERFRAME\r\n");
            // ����½Ҳ���Բɼ��������
            //HandleMeterFrame();
            if(RxValidDataFrame(TCP_RecBuf,TCP_RecLen) )
            {
                ClearDLT645SendBuf();
                MeterCmdAnalysis1(TCP_RecBuf);
                TcpSendDataLen=Send_645Buf_num;
                Send_Tcp_Data();
                    
            }
            Clear_Tcp_RecBuf(); 
        }
        else if (Frame == MODBUSFRAME)
        {
            DebugStr("MODBUSFRAME\r\n");
            //HandleModBusFrame();
            Clear_Tcp_RecBuf(); 
        }
        else if (Frame == LINKDOWN)
        {
            DebugStr("Link Down\r\n"); 
            return FALSE;
        }
        else
        {
            DebugStr("Frame Error\r\n"); 
            return FALSE;
        }
        
        /*
        if (((time/10) > CMD_TIMEOUT) && (ServerLogin == 0))  // û�����ݻ��½���˳�����ģʽ ���ر�����
        {
            DebugStr("Time Out1\r\n"); 
            break;      
        }

        if ((time/10) > 120)  // ��½��120��û��������ر�����
        {
            //������ͨ����·���ˣ���Ҫ�����ϱ�������
            
            DebugStr("Time Out2\r\n"); 

    
            NeedReConnect = 1;
            break;
        }
        */

        time = 0;
        while(1)
        {
            Frame = RecValidTcpFrame();
            if (Frame != FALSE)
            {
                break;
            }
            Sleep(500);  
            time ++;  // Լ1s
            if (time > *(WORD *)&SysParam[SP_RECTIMEOUT]) // û���յ��µ�ָ���򷵻�
            {
                Close_TcpIpLink();
                ServerLogin = 0;
                return FALSE;
            }
        }
    }
}
#endif

WORD GetDataStart_C()
{
    static WORD ret;
    
    if (TCP_RecLen == 0)
    {
        LastError = 4;
        return 0xFFFF;
    }

    if (Check_string_rxed("^IPSTATE") == 1)
    {
        if (Check_string_rxed("remote close") == 1)
        {
            DebugMsg();
            return 0xFFFF;
        }
    }
    
    if (!Query_Tcp_Data())
    {
        return 0xFFFF;
    }

    //�µ�ģ��û�з���OK (��ΪҲ̫xxxxx��)
    if ((Check_string_rxed("OK") != 1) && (Check_string_rxed("\r\n") != 1))
    {
        return 0xFFFF;
    }

    //�յ�������һ֡����
    ret = GetSignLoc(':',1);
    if (ret == 0xFFFF)
    {
        return 0xFFFF;
    }
    
    CurTcpLinkId = TCP_RecBuf[ret+1] - 0x30;
    if ((CurTcpLinkId == 0) || (CurTcpLinkId > 5))
    {
        return 0xFFFF;
    }

    //���ݿ�ʼλ��
    ret = GetSignLoc(',',2);
    if (ret == 0xFFFF)
    {
        return 0xFFFF;
    }

    return ret;
    //return 0xFF;
}

WORD GetDataStart_G()
{
    static WORD i;

    if (!QueryTcpData_G(0))  // ��ѯ���������յ�����
    {
        //DebugStr("GetDataStart_G-1\r\n");
        return 0xFFFF;
    }

    //DebugWord(Gsm_RecDataLen);

    if (!QueryTcpData_G(Gsm_RecDataLen)) // ��ȡ����
    {
        DebugStr("GetDataStart_G-2\r\n");
        return 0xFFFF;
    }

    i = GetSignLoc(0x0A, 1);
    if (i == 0xFFFF)
    {
        DebugStr("GetDataStart_G-3\r\n");
        return 0xFFFF;
    }

    
    
    return i;
}

WORD GetDataStartLoc()
{
    if (SysParam[SP_MODTYPE] == CDMA_MC323)
    {
        return GetDataStart_C();
    }
    else if (SysParam[SP_MODTYPE] == GSM_MG323)
    {
        return GetDataStart_G();
    }
    else
    {
        return 0xFFFF;
    }
}

//�յ���Ч��TCP֡
BYTE RecValidTcpFrame(void)
{
    static WORD i,len,ret;
    static WORD RecCrc;
    static BYTE StartCnt;
    
    
    ret = GetDataStartLoc();
    if (ret == 0xFFFF)
    {
        //DebugStr("RecValidTcpFrame-1\r\n");
        return FALSE;
    }

    

    #if 0
    DebugChar(0x0D);
    DebugChar(0x0A);
    for (i=0;i<TCP_RecLen;i++)
    {
        DebugChar(HexToBcd(TCP_RecBuf[ret+1+i]/16));
        DebugChar(HexToBcd(TCP_RecBuf[ret+1+i]%16));
        DebugChar(' ');
    }
    DebugChar(0x0D);
    DebugChar(0x0A);
    #endif

    if (SysParam[SP_MODTYPE] == CDMA_MC323)
    {
        Sleep(200);  // 24���CDMAģ��Ҫ����ʱ
    }

    *(DWORD *)&SysParam[SP_FLOWRATE] += TCP_RecLen;
    
    for (i=ret+1;i<TCP_RecLen;i++)
    {
        //�ж�ModBus����
        if ((TCP_RecBuf[i] == 0x0A) && (SwUpdating == 0))   // �յ���һ��OD,OA
        {
            if (TCP_RecBuf[i-1] == 0x0D)
            {
                SendLength = i - ret - 2;
                Crc = CRC16(&TCP_RecBuf[ret+1],SendLength-2);
                RecCrc = 0;
                RecCrc = (WORD)TCP_RecBuf[i-3];
                RecCrc = (RecCrc << 8) + TCP_RecBuf[i-2];
                if (Crc != RecCrc)
                {
                    LastError = 6;
                    Clear_Tcp_RecBuf();
                    DebugStr("Crc Error 1\r\n");
                    return FALSE;
                }
                else
                {
                    LastError = 7;
                    StartLoc = ret + 1;
                    //SendLength = i - ret - 2;
                    
                    return MODBUSFRAME;
                }
            }
        }
        else if (TCP_RecBuf[i] == 0x16) // ������
        {
            // ����������ʼ��
            for (StartLoc=ret+1;StartLoc<i;StartLoc++)
            {
                //�жϲɼ�������
                if ((TCP_RecBuf[StartLoc] == 0x68) && (TCP_RecBuf[StartLoc+3] == 0x68))
                {
                    DataLength = *(WORD *)&TCP_RecBuf[StartLoc+1];
                    
                    //CRC
                    Crc = CRC16(&TCP_RecBuf[StartLoc],(DWORD)(DataLength+11));
                    RecCrc = 0;
                    RecCrc = (WORD)TCP_RecBuf[StartLoc+DataLength+12];
                    RecCrc = (RecCrc << 8) + TCP_RecBuf[StartLoc+DataLength+11];
                    if (Crc != RecCrc)
                    {
                        LastError = 1;
                        Clear_Tcp_RecBuf();
                        DebugStr("Crc Error 2\r\n");
                        return FALSE;
                    }
         
                    // �Ǳ������ݷ���    
                    if (!mcmp(&TCP_RecBuf[StartLoc+4], &SysParam[SP_DEVADDR], 5))
                    {
                        LastError = 2;
                        Clear_Sms_buf();
                        return FALSE;
                    }

                    LastError = 0;
                    return DEVFRAME;
                }

                //�ж�645�������
                //����һ��0xFE,���4��
                if ((TCP_RecBuf[StartLoc] == 0xFE)  && (SwUpdating == 0))
                {
                    StartCnt = 1;
                    while(TCP_RecBuf[StartLoc+StartCnt] == 0xFE)
                    {
                        StartCnt++;
                    }
                    
                    if ((TCP_RecBuf[StartLoc+StartCnt] == 0x68) && (TCP_RecBuf[StartLoc+StartCnt+7] == 0x68))
                    {
                        len = TCP_RecBuf[StartLoc+StartCnt+9];
                        SendLength = len + StartCnt + 12;
                        if ((i-ret) == SendLength)
                        {
                            LastError = 5;
                            return METERFRAME;
                        }
                    }
                }
            }
        }
    }

    LastError = 3;
    //Clear_Tcp_RecBuf();
    //DebugStr("No data\r\n");
    return FALSE;
}

void HandleDevFrame(void)
{
    static BYTE CtlCode;
    static BYTE TypeCode;
    
    CtlCode  = TCP_RecBuf[StartLoc+9];
    TypeCode = TCP_RecBuf[StartLoc+10];

    switch(CtlCode)
    {
        case 0x10:  // ��½
            if (TypeCode == 0x10)
            {
                LoginDevice();
            }
            else if (TypeCode == 0x11)
            {
                LogoutDevice();
            }
            else
            {
                InvalidRequest();
            }
        break;

        case 0x11:  //������
            if (TypeCode == 0x10)
            {
                GetDevTimer();
            }
            else if (TypeCode == 0x11)
            {
                GetDevCommMode();
            }
            else if (TypeCode == 0x12)
            {
                GetDevWirelessParam();
            }
            else if (TypeCode == 0x13)
            {
                GetSrvNetParam();
            }
            else if (TypeCode == 0x14)
            {
                GetDevNetParam();
            }
            else if (TypeCode == 0x15)
            {
                GetDevWorkMode();
            }
            else if (TypeCode == 0x16)
            {
                GetDevSerialParam();
            }
            else if (TypeCode == 0x17)
            {
                GetDevMobNumber();
            }
            else if (TypeCode == 0x18)
            {
                GetDevHeartTime();
            }
            else if (TypeCode == 0x19)
            {
                GetDevRepSign();
            }
            else if (TypeCode == 0x1A)
            {
                GetDevReadTime();
            }
            else if (TypeCode == 0x1B)
            {
                GetDevMeterCount();
            }
            else if (TypeCode == 0x1C)
            {
                GetDevMeterParam();
            }
            else if (TypeCode == 0x1D)
            {
                InvalidRequest();
            }
            else if (TypeCode == 0x1E)
            {
                GetDevFlowRate();
            }
            else if (TypeCode == 0x1F)
            {
                GetDevEventSign();
            }
            else if (TypeCode == 0x20)
            {
                GetDevEvent();
            }
            else if (TypeCode == 0x21)
            {
                GetDevVersion();
            }
            else if (TypeCode == 0x22)
            {
                GetDevUserInfo();
            }
            else if (TypeCode == 0x23)
            {
                InvalidRequest();
            }
            else if (TypeCode == 0x24)
            {
                GetDevSleepTime();
            }
            else if (TypeCode == 0x25)
            {
                GetDevNetInfo();
            }
            else if (TypeCode == 0x26)
            {
                GetDevIpMode();
            }
            else if (TypeCode == 0x27)
            {
                GetDevMacAddr();
            }
            else if (TypeCode == 0x28)
            {
              //  GetExdevStatus();
            }
            else if (TypeCode == 0x29)
            {
              //  GetExdevPararm();
            }
            else if (TypeCode == 0x2A)
            {
               // GetTestModeParam();
            }
            else
            {
                InvalidRequest();
            }
        break;

        case 0x12:  // ��λ
            if (TypeCode == 0x10)
            {
                ResetDevice();
            }
            else if (TypeCode == 0x11)
            {
                SleepDevice();
            }
            else
            {
                InvalidRequest();
            }
        break;

        case 0x14:  // д����
            if (TypeCode == 0x10)
            {
                SetDevTime();
            }
            else if (TypeCode == 0x11)
            {
                SetDevCommMode();
            }
            else if (TypeCode == 0x12)
            {
                SetDevWirelessParam();
            }
            else if (TypeCode == 0x13)
            {
                SetSrvNetParam();
            }
            else if (TypeCode == 0x14)
            {
                SetDevNetParam();
            }
            else if (TypeCode == 0x15)
            {
                SetDevWorkMode();
            }
            else if (TypeCode == 0x16)
            {
                SetDevSerialParam();
            }
            else if (TypeCode == 0x17)
            {
                SetDevMobNumber();
            }
            else if (TypeCode == 0x18)
            {
                SetDevHeartTime();
            }
            else if (TypeCode == 0x19)
            {
                SetDevRepSign();
            }
            else if (TypeCode == 0x1A)
            {
                SetDevReadTime();
            }
            else if (TypeCode == 0x1B)
            {
                SetDevMeterCount();
            }
            else if (TypeCode == 0x1C)
            {
                SetDevMeterParam();
            }
            else if (TypeCode == 0x1D)
            {
                InvalidRequest();
            }
            else if (TypeCode == 0x1E)
            {
                ClearDevFlowRate();
            }
            else if (TypeCode == 0x22)
            {
                SetDevUserInfo();
            }
            else if (TypeCode == 0x23)
            {
                SetDevAddress();
            }
            else if (TypeCode == 0x24)
            {
                SetDevSleepTime();
            }
            else if (TypeCode == 0x25)
            {
                SetDevNetInfo();
            }
            else if (TypeCode == 0x26)
            {
                SetDevIpMode();
            }
            else if (TypeCode == 0x27)
            {
               // SetExdevParam();
            }
            else if (TypeCode == 0x28)
            {
               // SetTestModeParam();
            }
            else
            {
                InvalidRequest();
            }
        break;

        //�������
        case 0x15:
            if (TypeCode == 0x10)
            {
                SwUpdateRequest();
            }
            else if (TypeCode == 0x11)
            {
                SwUpdateData();
            }
            else if (TypeCode == 0x12)
            {
                SwUpdateFinish();
            }
            else
            {
                InvalidRequest();
            }
        break;

        case 0x18:  // �޸�����
            if (TypeCode == 0x10)
            {
                ChangePassword();
            }
            else
            {
                InvalidRequest();
            } 
        break;

        
        case 0x9C:  // �����ϱ���Ӧ
            if (TypeCode == 0x10)
            {
                RetDevStatus();
            }
            else if (TypeCode == 0x11)
            {
                RetHeartPack();
            }
            else if (TypeCode == 0x12)
            {
                RetDevData();
            }
            else
            {
                InvalidRequest();
            } 
        break;
    }

    Clear_Tcp_RecBuf();
}


void InvalidRequest(void)
{
    
}

void LoginDevice(void)
{
    Clear_Tcp_SendBuf();
    _pl_;

    TCP_SendBuf[0] = 0x68;
    TCP_SendBuf[1] = 0x01;
    TCP_SendBuf[2] = 0x00;
    TCP_SendBuf[3] = 0x68;
    TCP_SendBuf[4] = SysParam[SP_DEVADDR];
    TCP_SendBuf[5] = SysParam[SP_DEVADDR+1];
    TCP_SendBuf[6] = SysParam[SP_DEVADDR+2];
    TCP_SendBuf[7] = SysParam[SP_DEVADDR+3];
    TCP_SendBuf[8] = SysParam[SP_DEVADDR+4];

    TCP_SendBuf[9]  = 0x90;
    TCP_SendBuf[10] = 0x10;
   
    if (DataLength == 6)
    {
        if (mcmp((BYTE *)&TCP_RecBuf[StartLoc+11],(BYTE *)&SysParam[SP_PASSWORD],6))
        {
            DebugStr("Login\r\n");
            TCP_SendBuf[11] = 0x00;  // �ɹ�
            ServerLogin = 1;
        }
        else
        {
            TCP_SendBuf[11] = 0x01;  // ʧ��
            ServerLogin = 0;
        }
    }
    else
    {
        TCP_SendBuf[11] = 0x01;  // ʧ��
        ServerLogin = 0;
    }

    //CRC
    Crc = CRC16(TCP_SendBuf,12);
    TCP_SendBuf[12] = (BYTE)Crc;
    TCP_SendBuf[13] = (BYTE)(Crc>>8);

    TCP_SendBuf[14] = 0x16;  // ����

    TcpSendDataLen = 15;
    Send_Tcp_Data();
      
}

void LogoutDevice(void)
{
    Clear_Tcp_SendBuf();
    TCP_SendBuf[0] = 0x68;
    TCP_SendBuf[1] = 0x01;
    TCP_SendBuf[2] = 0x00;
    TCP_SendBuf[3] = 0x68;
    TCP_SendBuf[4] = SysParam[SP_DEVADDR];
    TCP_SendBuf[5] = SysParam[SP_DEVADDR+1];
    TCP_SendBuf[6] = SysParam[SP_DEVADDR+2];
    TCP_SendBuf[7] = SysParam[SP_DEVADDR+3];
    TCP_SendBuf[8] = SysParam[SP_DEVADDR+4];

    TCP_SendBuf[9]  = 0x90;
    TCP_SendBuf[10] = 0x11;
    
    if (DataLength == 6)
    {
        if (mcmp(&TCP_RecBuf[StartLoc+11],&SysParam[SP_PASSWORD],6))
        {
            DebugStr("Logout\r\n");
            TCP_SendBuf[11] = 0x00;  // �ɹ�
            ServerLogin = 0;
        }
        else
        {
            TCP_SendBuf[11] = 0x01;  // ʧ��           
        }
    }
    else
    {
        TCP_SendBuf[11] = 0x01;  // ʧ��        
    }

    //CRC
    Crc = CRC16(TCP_SendBuf,12);
    TCP_SendBuf[12] = (BYTE)Crc;
    TCP_SendBuf[13] = (BYTE)(Crc>>8);

    TCP_SendBuf[14] = 0x16;  // ����

    TcpSendDataLen = 15;
    Send_Tcp_Data();
   
}

void ResetDevice(void)
{
    static BYTE NeedReset;

    NeedReset = 0;
    Clear_Tcp_SendBuf();

    TCP_SendBuf[0] = 0x68;
    TCP_SendBuf[1] = 0x01;
    TCP_SendBuf[2] = 0x00;
    TCP_SendBuf[3] = 0x68;
    TCP_SendBuf[4] = SysParam[SP_DEVADDR];
    TCP_SendBuf[5] = SysParam[SP_DEVADDR+1];
    TCP_SendBuf[6] = SysParam[SP_DEVADDR+2];
    TCP_SendBuf[7] = SysParam[SP_DEVADDR+3];
    TCP_SendBuf[8] = SysParam[SP_DEVADDR+4];

    TCP_SendBuf[9]  = 0x92;
    TCP_SendBuf[10] = 0x10;
    
    if (DataLength == 6)
    {
        if (mcmp(&TCP_RecBuf[StartLoc+11],&SysParam[SP_PASSWORD],6))
        {
            TCP_SendBuf[11] = 0x00;  // �ɹ�
            NeedReset = 1;
        }
        else
        {
            TCP_SendBuf[11] = 0x01;  // ʧ��
        }
    }
    else if (DataLength == 0)
    {
        TCP_SendBuf[11] = 0x00;  // �ɹ�
        NeedReset = 1;
    }
    else
    {
        TCP_SendBuf[11] = 0x01;  // ʧ��
    }

    //CRC
    Crc = CRC16(TCP_SendBuf,12);
    TCP_SendBuf[12] = (BYTE)Crc;
    TCP_SendBuf[13] = (BYTE)(Crc>>8);

    TCP_SendBuf[14] = 0x16;  // ����

    TcpSendDataLen = 15;
    Send_Tcp_Data();

    if (NeedReset)
    {
        Sleep(2000);
        SysReset();
    }
}

void GetDevTimer(void)
{
    static WORD Year;

    if (ServerLogin == 0)
    {
        return;
    }
    
    Clear_Tcp_SendBuf();

    TCP_SendBuf[0] = 0x68;
    TCP_SendBuf[1] = 0x07;
    TCP_SendBuf[2] = 0x00;
    TCP_SendBuf[3] = 0x68;
    TCP_SendBuf[4] = SysParam[SP_DEVADDR];
    TCP_SendBuf[5] = SysParam[SP_DEVADDR+1];
    TCP_SendBuf[6] = SysParam[SP_DEVADDR+2];
    TCP_SendBuf[7] = SysParam[SP_DEVADDR+3];
    TCP_SendBuf[8] = SysParam[SP_DEVADDR+4];

    TCP_SendBuf[9]  = 0x91;
    TCP_SendBuf[10] = 0x10;

    
    //Data
    GetSysTime();
    TCP_SendBuf[11] = (SysTime[6]/16) * 10 + SysTime[5]%16;  // ��
    TCP_SendBuf[12] = (SysTime[5]/16) * 10 + SysTime[4]%16;  // ��
    TCP_SendBuf[13] = (SysTime[4]/16) * 10 + SysTime[3]%16;  // ʱ
    TCP_SendBuf[14] = (SysTime[3]/16) * 10 + SysTime[2]%16;  // ��
    TCP_SendBuf[15] = (SysTime[2]/16) * 10 + SysTime[1]%16;  // ��
    Year = 2000 + ((SysTime[0]/16) * 10 + SysTime[0]%16);
    TCP_SendBuf[16] = (BYTE)Year;  // ��
    TCP_SendBuf[17] = (BYTE)(Year>>8);  
    

    //CRC
    Crc = CRC16(TCP_SendBuf,18);
    TCP_SendBuf[18] = (BYTE)Crc;
    TCP_SendBuf[19] = (BYTE)(Crc>>8);

    TCP_SendBuf[20] = 0x16;  // ����

    TcpSendDataLen = 21;
    Send_Tcp_Data();
}

void GetDevCommMode(void)
{
    if (ServerLogin == 0)
    {
        return;
    }

    Clear_Tcp_SendBuf();

    TCP_SendBuf[0] = 0x68;
    TCP_SendBuf[1] = 0x01;
    TCP_SendBuf[2] = 0x00;
    TCP_SendBuf[3] = 0x68;
    TCP_SendBuf[4] = SysParam[SP_DEVADDR];
    TCP_SendBuf[5] = SysParam[SP_DEVADDR+1];
    TCP_SendBuf[6] = SysParam[SP_DEVADDR+2];
    TCP_SendBuf[7] = SysParam[SP_DEVADDR+3];
    TCP_SendBuf[8] = SysParam[SP_DEVADDR+4];

    TCP_SendBuf[9]  = 0x91;
    TCP_SendBuf[10] = 0x11;
    
    //Data
    TCP_SendBuf[11] = SysParam[SP_COMMMODE];  
    

    //CRC
    Crc = CRC16(TCP_SendBuf,12);
    TCP_SendBuf[12] = (BYTE)Crc;
    TCP_SendBuf[13] = (BYTE)(Crc>>8);

    TCP_SendBuf[14] = 0x16;  // ����

    TcpSendDataLen = 15;
    Send_Tcp_Data();
}

void GetDevWirelessParam(void)
{
    if (ServerLogin == 0)
    {
        return;
    }

    Clear_Tcp_SendBuf();

    TCP_SendBuf[0] = 0x68;
    TCP_SendBuf[1] = 20;
    TCP_SendBuf[2] = 0x00;
    TCP_SendBuf[3] = 0x68;
    TCP_SendBuf[4] = SysParam[SP_DEVADDR];
    TCP_SendBuf[5] = SysParam[SP_DEVADDR+1];
    TCP_SendBuf[6] = SysParam[SP_DEVADDR+2];
    TCP_SendBuf[7] = SysParam[SP_DEVADDR+3];
    TCP_SendBuf[8] = SysParam[SP_DEVADDR+4];

    TCP_SendBuf[9]  = 0x91;
    TCP_SendBuf[10] = 0x12;
    
    //Data  20 �ֽ�
    TCP_SendBuf[11] = 0x31;
    TCP_SendBuf[12] = 0x32;  
    TCP_SendBuf[13] = 0x33;  
    TCP_SendBuf[14] = 0x34;  
    

    //CRC
    Crc = CRC16(TCP_SendBuf,31);
    TCP_SendBuf[31] = (BYTE)Crc;
    TCP_SendBuf[32] = (BYTE)(Crc>>8);

    TCP_SendBuf[33] = 0x16;  // ����

    TcpSendDataLen = 34;
    Send_Tcp_Data();
    
}

void GetSrvNetParam(void)
{
    if (ServerLogin == 0)
    {
        return;
    }

    Clear_Tcp_SendBuf();

    TCP_SendBuf[0] = 0x68;
    TCP_SendBuf[1] = 0x09;
    TCP_SendBuf[2] = 0x00;
    TCP_SendBuf[3] = 0x68;
    TCP_SendBuf[4] = SysParam[SP_DEVADDR];
    TCP_SendBuf[5] = SysParam[SP_DEVADDR+1];
    TCP_SendBuf[6] = SysParam[SP_DEVADDR+2];
    TCP_SendBuf[7] = SysParam[SP_DEVADDR+3];
    TCP_SendBuf[8] = SysParam[SP_DEVADDR+4];

    TCP_SendBuf[9]  = 0x91;
    TCP_SendBuf[10] = 0x13;
    
    //Data  9 �ֽ�
    // IP Addr
    TCP_SendBuf[11] = SysParam[SP_SERVERIP];  
    TCP_SendBuf[12] = SysParam[SP_SERVERIP+1];  
    TCP_SendBuf[13] = SysParam[SP_SERVERIP+2];  
    TCP_SendBuf[14] = SysParam[SP_SERVERIP+3];  
    //net mode
    TCP_SendBuf[15] = SysParam[SP_NETMODE];
    //Port
    TCP_SendBuf[16] = SysParam[SP_SERVERPORT];   // Զ�̶˿� 
    TCP_SendBuf[17] = SysParam[SP_SERVERPORT+1];
    TCP_SendBuf[18] = SysParam[SP_SERVERPORT];   // ���ض˿� 
    TCP_SendBuf[19] = SysParam[SP_SERVERPORT+1];

    //CRC
    Crc = CRC16(TCP_SendBuf,20);
    TCP_SendBuf[20] = (BYTE)Crc;
    TCP_SendBuf[21] = (BYTE)(Crc>>8);

    TCP_SendBuf[22] = 0x16;  // ����

    TcpSendDataLen = 23;
    Send_Tcp_Data();
}


void GetDevNetParam(void)
{
    if (ServerLogin == 0)
    {
        return;
    }

    Clear_Tcp_SendBuf();

    TCP_SendBuf[0] = 0x68;
    TCP_SendBuf[1] = 0x09;
    TCP_SendBuf[2] = 0x00;
    TCP_SendBuf[3] = 0x68;
    TCP_SendBuf[4] = SysParam[SP_DEVADDR];
    TCP_SendBuf[5] = SysParam[SP_DEVADDR+1];
    TCP_SendBuf[6] = SysParam[SP_DEVADDR+2];
    TCP_SendBuf[7] = SysParam[SP_DEVADDR+3];
    TCP_SendBuf[8] = SysParam[SP_DEVADDR+4];

    TCP_SendBuf[9]  = 0x91;
    TCP_SendBuf[10] = 0x14;
    
    //Data  9 �ֽ�
    // IP Addr
    TCP_SendBuf[11] = SysParam[SP_LOCALIP];  
    TCP_SendBuf[12] = SysParam[SP_LOCALIP+1];  
    TCP_SendBuf[13] = SysParam[SP_LOCALIP+2];  
    TCP_SendBuf[14] = SysParam[SP_LOCALIP+3];  
    //Comm mode
    TCP_SendBuf[15] = SysParam[SP_NETMODE];   // TCP
    //Port
    TCP_SendBuf[16] = SysParam[SP_LOCALPORT];   // Զ�̶˿� 
    TCP_SendBuf[17] = SysParam[SP_LOCALPORT+1];
    TCP_SendBuf[18] = SysParam[SP_LOCALPORT];   // ���ض˿� 
    TCP_SendBuf[19] = SysParam[SP_LOCALPORT+1];

    //CRC
    Crc = CRC16(TCP_SendBuf,20);
    TCP_SendBuf[20] = (BYTE)Crc;
    TCP_SendBuf[21] = (BYTE)(Crc>>8);

    TCP_SendBuf[22] = 0x16;  // ����

    TcpSendDataLen = 23;
    Send_Tcp_Data();
}

void GetDevWorkMode(void)
{
    if (ServerLogin == 0)
    {
        return;
    }

    Clear_Tcp_SendBuf();

    TCP_SendBuf[0] = 0x68;
    TCP_SendBuf[1] = 0x01;
    TCP_SendBuf[2] = 0x00;
    TCP_SendBuf[3] = 0x68;
    TCP_SendBuf[4] = SysParam[SP_DEVADDR];
    TCP_SendBuf[5] = SysParam[SP_DEVADDR+1];
    TCP_SendBuf[6] = SysParam[SP_DEVADDR+2];
    TCP_SendBuf[7] = SysParam[SP_DEVADDR+3];
    TCP_SendBuf[8] = SysParam[SP_DEVADDR+4];

    TCP_SendBuf[9]  = 0x91;
    TCP_SendBuf[10] = 0x15;
    
    //Data
    TCP_SendBuf[11] = SysParam[SP_WORKMODE];   // �ͷ���ģʽ
    

    //CRC
    Crc = CRC16(TCP_SendBuf,12);
    TCP_SendBuf[12] = (BYTE)Crc;
    TCP_SendBuf[13] = (BYTE)(Crc>>8);

    TCP_SendBuf[14] = 0x16;  // ����

    TcpSendDataLen = 15;
    Send_Tcp_Data();
}


void GetDevSerialParam(void)
{
    if (ServerLogin == 0)
    {
        return;
    }

    Clear_Tcp_SendBuf();

    TCP_SendBuf[0] = 0x68;
    TCP_SendBuf[1] = 0x04;
    TCP_SendBuf[2] = 0x00;
    TCP_SendBuf[3] = 0x68;
    TCP_SendBuf[4] = SysParam[SP_DEVADDR];
    TCP_SendBuf[5] = SysParam[SP_DEVADDR+1];
    TCP_SendBuf[6] = SysParam[SP_DEVADDR+2];
    TCP_SendBuf[7] = SysParam[SP_DEVADDR+3];
    TCP_SendBuf[8] = SysParam[SP_DEVADDR+4];

    TCP_SendBuf[9]  = 0x91;
    TCP_SendBuf[10] = 0x16;
    
    //Data
    TCP_SendBuf[11] = SysParam[SP_BAUDRATE];   //01:300;02:600;03:1200;04:2400;05:4800;05:9600;06:19200;07:38400;08:115200
    TCP_SendBuf[12] = SysParam[SP_CHECKBIT];   //01:��У��;02:żУ��;03��У��
    TCP_SendBuf[13] = SysParam[SP_DATABIT];   //����λ
    TCP_SendBuf[14] = SysParam[SP_STOPBIT];   //ֹͣλ    

    //CRC
    Crc = CRC16(TCP_SendBuf,15);
    TCP_SendBuf[15] = (BYTE)Crc;
    TCP_SendBuf[16] = (BYTE)(Crc>>8);

    TCP_SendBuf[17] = 0x16;  // ����

    TcpSendDataLen = 18;
    Send_Tcp_Data();
}

void GetDevMobNumber(void)
{
    if (ServerLogin == 0)
    {
        return;
    }

    Clear_Tcp_SendBuf();

    TCP_SendBuf[0] = 0x68;
    TCP_SendBuf[1] = 12;
    TCP_SendBuf[2] = 0x00;
    TCP_SendBuf[3] = 0x68;
    TCP_SendBuf[4] = SysParam[SP_DEVADDR];
    TCP_SendBuf[5] = SysParam[SP_DEVADDR+1];
    TCP_SendBuf[6] = SysParam[SP_DEVADDR+2];
    TCP_SendBuf[7] = SysParam[SP_DEVADDR+3];
    TCP_SendBuf[8] = SysParam[SP_DEVADDR+4];

    TCP_SendBuf[9]  = 0x91;
    TCP_SendBuf[10] = 0x17;
    
    //Data 12
    //��������
    TCP_SendBuf[11] = (SysParam[SP_SELFNUM+9]-0x30)*16 + (SysParam[SP_SELFNUM+10]-0x30);
    TCP_SendBuf[12] = (SysParam[SP_SELFNUM+7]-0x30)*16 + (SysParam[SP_SELFNUM+8]-0x30);
    TCP_SendBuf[13] = (SysParam[SP_SELFNUM+5]-0x30)*16 + (SysParam[SP_SELFNUM+6]-0x30);
    TCP_SendBuf[14] = (SysParam[SP_SELFNUM+3]-0x30)*16 + (SysParam[SP_SELFNUM+4]-0x30);
    TCP_SendBuf[15] = (SysParam[SP_SELFNUM+1]-0x30)*16 + (SysParam[SP_SELFNUM+2]-0x30);
    TCP_SendBuf[16] = (SysParam[SP_SELFNUM]-0x30);
    
    //�������ĺ���  6�ֽ�
    TCP_SendBuf[17] = (SysParam[SP_MOBLENUM+9]-0x30)*16 + (SysParam[SP_MOBLENUM+10]-0x30);  
    TCP_SendBuf[18] = (SysParam[SP_MOBLENUM+7]-0x30)*16 + (SysParam[SP_MOBLENUM+8]-0x30);
    TCP_SendBuf[19] = (SysParam[SP_MOBLENUM+5]-0x30)*16 + (SysParam[SP_MOBLENUM+6]-0x30);
    TCP_SendBuf[20] = (SysParam[SP_MOBLENUM+3]-0x30)*16 + (SysParam[SP_MOBLENUM+4]-0x30);
    TCP_SendBuf[21] = (SysParam[SP_MOBLENUM+1]-0x30)*16 + (SysParam[SP_MOBLENUM+2]-0x30);
    TCP_SendBuf[22] = (SysParam[SP_MOBLENUM]-0x30);
    
    
    
    //CRC
    Crc = CRC16(TCP_SendBuf,23);
    TCP_SendBuf[23] = (BYTE)Crc;
    TCP_SendBuf[24] = (BYTE)(Crc>>8);

    TCP_SendBuf[25] = 0x16;  // ����

    TcpSendDataLen = 26;
    Send_Tcp_Data();
}

void GetDevHeartTime(void)
{
    if (ServerLogin == 0)
    {
        return;
    }

    Clear_Tcp_SendBuf();

    TCP_SendBuf[0] = 0x68;
    TCP_SendBuf[1] = 0x02;
    TCP_SendBuf[2] = 0x00;
    TCP_SendBuf[3] = 0x68;
    TCP_SendBuf[4] = SysParam[SP_DEVADDR];
    TCP_SendBuf[5] = SysParam[SP_DEVADDR+1];
    TCP_SendBuf[6] = SysParam[SP_DEVADDR+2];
    TCP_SendBuf[7] = SysParam[SP_DEVADDR+3];
    TCP_SendBuf[8] = SysParam[SP_DEVADDR+4];

    TCP_SendBuf[9]  = 0x91;
    TCP_SendBuf[10] = 0x18;
    
    //Data
    TCP_SendBuf[11] = SysParam[SP_HEARTTIME]; 
    TCP_SendBuf[12] = SysParam[SP_HEARTTIME+1];  
    
    //CRC
    Crc = CRC16(TCP_SendBuf,13);
    TCP_SendBuf[13] = (BYTE)Crc;
    TCP_SendBuf[14] = (BYTE)(Crc>>8);

    TCP_SendBuf[15] = 0x16;  // ����

    TcpSendDataLen = 16;
    Send_Tcp_Data();
}

void GetDevRepSign(void)
{
    if (ServerLogin == 0)
    {
        return;
    }

    Clear_Tcp_SendBuf();

    TCP_SendBuf[0] = 0x68;
    TCP_SendBuf[1] = 0x01;
    TCP_SendBuf[2] = 0x00;
    TCP_SendBuf[3] = 0x68;
    TCP_SendBuf[4] = SysParam[SP_DEVADDR];
    TCP_SendBuf[5] = SysParam[SP_DEVADDR+1];
    TCP_SendBuf[6] = SysParam[SP_DEVADDR+2];
    TCP_SendBuf[7] = SysParam[SP_DEVADDR+3];
    TCP_SendBuf[8] = SysParam[SP_DEVADDR+4];

    TCP_SendBuf[9]  = 0x91;
    TCP_SendBuf[10] = 0x19;
    
    //Data
    TCP_SendBuf[11] = SysParam[SP_REPENABLE]; 
    

    //CRC
    Crc = CRC16(TCP_SendBuf,12);
    TCP_SendBuf[12] = (BYTE)Crc;
    TCP_SendBuf[13] = (BYTE)(Crc>>8);

    TCP_SendBuf[14] = 0x16;  // ����

    TcpSendDataLen = 15;
    Send_Tcp_Data();
}

void GetDevReadTime(void)
{
    if (ServerLogin == 0)
    {
        return;
    }

    Clear_Tcp_SendBuf();

    TCP_SendBuf[0] = 0x68;
    TCP_SendBuf[1] = 0x02;
    TCP_SendBuf[2] = 0x00;
    TCP_SendBuf[3] = 0x68;
    TCP_SendBuf[4] = SysParam[SP_DEVADDR];
    TCP_SendBuf[5] = SysParam[SP_DEVADDR+1];
    TCP_SendBuf[6] = SysParam[SP_DEVADDR+2];
    TCP_SendBuf[7] = SysParam[SP_DEVADDR+3];
    TCP_SendBuf[8] = SysParam[SP_DEVADDR+4];

    TCP_SendBuf[9]  = 0x91;
    TCP_SendBuf[10] = 0x1A;
    
    //Data
    TCP_SendBuf[11] = SysParam[SP_READTIME];
    TCP_SendBuf[12] = SysParam[SP_READTIME+1];  
    

    //CRC
    Crc = CRC16(TCP_SendBuf,13);
    TCP_SendBuf[13] = (BYTE)Crc;
    TCP_SendBuf[14] = (BYTE)(Crc>>8);

    TCP_SendBuf[15] = 0x16;  // ����

    TcpSendDataLen = 16;
    Send_Tcp_Data();
}

void GetDevMeterCount(void)
{
    if (ServerLogin == 0)
    {
        return;
    }

    Clear_Tcp_SendBuf();

    TCP_SendBuf[0] = 0x68;
    TCP_SendBuf[1] = 0x01;
    TCP_SendBuf[2] = 0x00;
    TCP_SendBuf[3] = 0x68;
    TCP_SendBuf[4] = SysParam[SP_DEVADDR];
    TCP_SendBuf[5] = SysParam[SP_DEVADDR+1];
    TCP_SendBuf[6] = SysParam[SP_DEVADDR+2];
    TCP_SendBuf[7] = SysParam[SP_DEVADDR+3];
    TCP_SendBuf[8] = SysParam[SP_DEVADDR+4];

    TCP_SendBuf[9]  = 0x91;
    TCP_SendBuf[10] = 0x1B;
    
    //Data
    TCP_SendBuf[11] = SysParam[SP_METERCOUNT]; 
    

    //CRC
    Crc = CRC16(TCP_SendBuf,12);
    TCP_SendBuf[12] = (BYTE)Crc;
    TCP_SendBuf[13] = (BYTE)(Crc>>8);

    TCP_SendBuf[14] = 0x16;  // ����

    TcpSendDataLen = 15;
    Send_Tcp_Data();
}

void GetDevMeterParam(void)
{
    if (ServerLogin == 0)
    {
        return;
    }

    Clear_Tcp_SendBuf();

    TCP_SendBuf[0] = 0x68;
    TCP_SendBuf[1] = 0x26;
    TCP_SendBuf[2] = 0x00;
    TCP_SendBuf[3] = 0x68;
    TCP_SendBuf[4] = SysParam[SP_DEVADDR];
    TCP_SendBuf[5] = SysParam[SP_DEVADDR+1];
    TCP_SendBuf[6] = SysParam[SP_DEVADDR+2];
    TCP_SendBuf[7] = SysParam[SP_DEVADDR+3];
    TCP_SendBuf[8] = SysParam[SP_DEVADDR+4];

    TCP_SendBuf[9]  = 0x91;
    TCP_SendBuf[10] = 0x1C;

    // ֻ֧��һ�����
    if (TCP_RecBuf[11] != 0)
    {
        TCP_SendBuf[11] = 0xFF;
    }
    else
    {
        TCP_SendBuf[11] = 0x00;
    }
    
    TCP_SendBuf[12] = 0x01;   // ��Լ���
    TCP_SendBuf[13] = 0x01;   // ����ַ 6 �ֽ�

    TCP_SendBuf[19] = 0x01;   //A���ѹֵ����ֵ
    TCP_SendBuf[20] = 0x01; 
    TCP_SendBuf[21] = 0x01;   //B���ѹֵ����ֵ
    TCP_SendBuf[22] = 0x01; 
    TCP_SendBuf[23] = 0x01;   //C���ѹֵ����ֵ
    TCP_SendBuf[24] = 0x01; 

    TCP_SendBuf[25] = 0x01;   //A�����ֵ����ֵ
    TCP_SendBuf[26] = 0x01; 
    TCP_SendBuf[27] = 0x01;  
    TCP_SendBuf[28] = 0x01; 
    TCP_SendBuf[29] = 0x01;   //B�����ֵ����ֵ
    TCP_SendBuf[30] = 0x01; 
    TCP_SendBuf[31] = 0x01; 
    TCP_SendBuf[32] = 0x01; 
    TCP_SendBuf[33] = 0x01;   //C�����ֵ����ֵ
    TCP_SendBuf[34] = 0x01; 
    TCP_SendBuf[35] = 0x01; 
    TCP_SendBuf[36] = 0x01; 

    TCP_SendBuf[37] = 0x01;   //A�๦��ֵ����ֵ
    TCP_SendBuf[38] = 0x01; 
    TCP_SendBuf[39] = 0x01;  
    TCP_SendBuf[40] = 0x01; 
    TCP_SendBuf[41] = 0x01;   //B�๦��ֵ����ֵ
    TCP_SendBuf[42] = 0x01; 
    TCP_SendBuf[43] = 0x01; 
    TCP_SendBuf[44] = 0x01; 
    TCP_SendBuf[45] = 0x01;   //C�๦��ֵ����ֵ
    TCP_SendBuf[46] = 0x01; 
    TCP_SendBuf[47] = 0x01; 
    TCP_SendBuf[48] = 0x01; 

    

    //CRC
    Crc = CRC16(TCP_SendBuf,49);
    TCP_SendBuf[49] = (BYTE)Crc;
    TCP_SendBuf[50] = (BYTE)(Crc>>8);

    TCP_SendBuf[51] = 0x16;  // ����

    TcpSendDataLen = 52;
    Send_Tcp_Data();
}

void GetDevFlowRate(void)
{
    if (ServerLogin == 0)
    {
        return;
    }

    Clear_Tcp_SendBuf();

    TCP_SendBuf[0] = 0x68;
    TCP_SendBuf[1] = 0x04;
    TCP_SendBuf[2] = 0x00;
    TCP_SendBuf[3] = 0x68;
    TCP_SendBuf[4] = SysParam[SP_DEVADDR];
    TCP_SendBuf[5] = SysParam[SP_DEVADDR+1];
    TCP_SendBuf[6] = SysParam[SP_DEVADDR+2];
    TCP_SendBuf[7] = SysParam[SP_DEVADDR+3];
    TCP_SendBuf[8] = SysParam[SP_DEVADDR+4];

    TCP_SendBuf[9]  = 0x91;
    TCP_SendBuf[10] = 0x1E;
    
    //Data
    //TCP_SendBuf[11] = 0x00;
    //TCP_SendBuf[12] = 0x00;
    //TCP_SendBuf[13] = 0x00;
    //TCP_SendBuf[14] = 0x00;
    *(DWORD *)&TCP_SendBuf[11]  = (*(DWORD *)&SysParam[SP_FLOWRATE])/10;
    

    //CRC
    Crc = CRC16(TCP_SendBuf,15);
    TCP_SendBuf[15] = (BYTE)Crc;
    TCP_SendBuf[16] = (BYTE)(Crc>>8);

    TCP_SendBuf[17] = 0x16;  // ����

    TcpSendDataLen = 18;
    Send_Tcp_Data();
}

void GetDevEventSign(void)
{
    if (ServerLogin == 0)
    {
        return;
    }

    Clear_Tcp_SendBuf();

    TCP_SendBuf[0] = 0x68;
    TCP_SendBuf[1] = 0x08;
    TCP_SendBuf[2] = 0x00;
    TCP_SendBuf[3] = 0x68;
    TCP_SendBuf[4] = SysParam[SP_DEVADDR];
    TCP_SendBuf[5] = SysParam[SP_DEVADDR+1];
    TCP_SendBuf[6] = SysParam[SP_DEVADDR+2];
    TCP_SendBuf[7] = SysParam[SP_DEVADDR+3];
    TCP_SendBuf[8] = SysParam[SP_DEVADDR+4];

    TCP_SendBuf[9]  = 0x91;
    TCP_SendBuf[10] = 0x1F;
    
    //Data
    TCP_SendBuf[11] = 0x01;  
    TCP_SendBuf[12] = 0x00;  
    TCP_SendBuf[13] = 0x00;
    TCP_SendBuf[14] = 0x00;       

    TCP_SendBuf[15] = 0x01;
    TCP_SendBuf[16] = 0x00;
    TCP_SendBuf[17] = 0x00;
    TCP_SendBuf[18] = 0x00;
    

    //CRC
    Crc = CRC16(TCP_SendBuf,19);
    TCP_SendBuf[19] = (BYTE)Crc;
    TCP_SendBuf[20] = (BYTE)(Crc>>8);

    TCP_SendBuf[21] = 0x16;  // ����

    TcpSendDataLen = 22;
    Send_Tcp_Data();
}

void GetDevEvent(void)
{
    if (ServerLogin == 0)
    {
        return;
    }

    //��ʱû��ʵ��
    
}

void GetDevVersion(void)
{
    if (ServerLogin == 0)
    {
        return;
    }

    Clear_Tcp_SendBuf();

    TCP_SendBuf[0] = 0x68;
    TCP_SendBuf[1] = 0x16;
    TCP_SendBuf[2] = 0x00;
    TCP_SendBuf[3] = 0x68;
    TCP_SendBuf[4] = SysParam[SP_DEVADDR];
    TCP_SendBuf[5] = SysParam[SP_DEVADDR+1];
    TCP_SendBuf[6] = SysParam[SP_DEVADDR+2];
    TCP_SendBuf[7] = SysParam[SP_DEVADDR+3];
    TCP_SendBuf[8] = SysParam[SP_DEVADDR+4];

    TCP_SendBuf[9]  = 0x91;
    TCP_SendBuf[10] = 0x21;
    
    //Data
    //�豸���  8 �ֽ�
    TCP_SendBuf[11] = DevIndex[0];
    TCP_SendBuf[12] = DevIndex[1];
    TCP_SendBuf[13] = DevIndex[2];
    TCP_SendBuf[14] = DevIndex[3];
    TCP_SendBuf[15] = DevIndex[4];
    TCP_SendBuf[16] = DevIndex[5];
    TCP_SendBuf[17] = DevIndex[6];
    TCP_SendBuf[18] = DevIndex[7];
    
    // �ɼ����汾�� 7  001.000
    TCP_SendBuf[19] = DevVersion[0];
    TCP_SendBuf[20] = DevVersion[1];
    TCP_SendBuf[21] = DevVersion[2];
    TCP_SendBuf[22] = DevVersion[3];
    TCP_SendBuf[23] = DevVersion[4];
    TCP_SendBuf[24] = DevVersion[5];
    TCP_SendBuf[25] = DevVersion[6];
    
    // �ɼ����������� 7
    TCP_SendBuf[26] = DevRelTime[0];   // ��
    TCP_SendBuf[27] = DevRelTime[1];   // ��
    TCP_SendBuf[28] = DevRelTime[2];   // ʱ
    TCP_SendBuf[29] = DevRelTime[3];   // ��
    TCP_SendBuf[30] = DevRelTime[4];   // ��
    TCP_SendBuf[31] = DevRelTime[5];   // ���λ
    TCP_SendBuf[32] = DevRelTime[6];   // ���λ
         
    //CRC
    Crc = CRC16(TCP_SendBuf,33);
    TCP_SendBuf[33] = (BYTE)Crc;
    TCP_SendBuf[34] = (BYTE)(Crc>>8);

    TCP_SendBuf[35] = 0x16;  // ����

    TcpSendDataLen = 36;
    Send_Tcp_Data();
}

void ChangePassword(void)
{
    if (ServerLogin == 0)
    {
        return;
    }

    Clear_Tcp_SendBuf();

    TCP_SendBuf[0] = 0x68;
    TCP_SendBuf[1] = 0x01;
    TCP_SendBuf[2] = 0x00;
    TCP_SendBuf[3] = 0x68;
    TCP_SendBuf[4] = SysParam[SP_DEVADDR];
    TCP_SendBuf[5] = SysParam[SP_DEVADDR+1];
    TCP_SendBuf[6] = SysParam[SP_DEVADDR+2];
    TCP_SendBuf[7] = SysParam[SP_DEVADDR+3];
    TCP_SendBuf[8] = SysParam[SP_DEVADDR+4];

    TCP_SendBuf[9]  = 0x98;
    TCP_SendBuf[10] = 0x10;
    
    if (DataLength == 6)
    {
        mcpy((BYTE *)&SysParam[SP_PASSWORD],&TCP_RecBuf[StartLoc+11],6);
        //mcpy(DevPassTmp,&TCP_RecBuf[StartLoc+11],6);
        NeedSaveParam = 1;
        TCP_SendBuf[11] = 0x00;  // �ɹ�
    }
    else
    {
        TCP_SendBuf[11] = 0x01;  // ʧ��
        ServerLogin = 0;
    }

    //CRC
    Crc = CRC16(TCP_SendBuf,12);
    TCP_SendBuf[12] = (BYTE)Crc;
    TCP_SendBuf[13] = (BYTE)(Crc>>8);

    TCP_SendBuf[14] = 0x16;  // ����

    TcpSendDataLen = 15;
    Send_Tcp_Data();
}


//�����ϱ�
void RepDevStatus(void)
{
    Clear_Tcp_SendBuf();

    TCP_SendBuf[0] = 0x68;
    TCP_SendBuf[1] = 0x09;
    TCP_SendBuf[2] = 0x00;
    TCP_SendBuf[3] = 0x68;
    TCP_SendBuf[4] = SysParam[SP_DEVADDR];
    TCP_SendBuf[5] = SysParam[SP_DEVADDR+1];
    TCP_SendBuf[6] = SysParam[SP_DEVADDR+2];
    TCP_SendBuf[7] = SysParam[SP_DEVADDR+3];
    TCP_SendBuf[8] = SysParam[SP_DEVADDR+4];

    TCP_SendBuf[9]  = 0x1C;
    TCP_SendBuf[10] = 0x10;
    
    //���� 9 �ֽ�
    TCP_SendBuf[11] = 0x01; 
    
    //CRC
    Crc = CRC16(TCP_SendBuf,20);
    TCP_SendBuf[20] = (BYTE)Crc;
    TCP_SendBuf[21] = (BYTE)(Crc>>8);

    TCP_SendBuf[22] = 0x16;  // ����

    TcpSendDataLen = 23;
    Send_Tcp_Data();
}

//#ifdef CDMA_SRVMODE
#if 0
BYTE RepHeartPack(void)
{
    static BYTE i,Retry;
//    BYTE j;


    if (SysParam[SP_MODTYPE] == CDMA_MC323)
    {
        CurTcpLinkId = 1;
    }
    else
    {
        CurTcpLinkId = 0;
    }
    
    if (!Init_TCPIP())
    {
        return FALSE;
    }
    
    if (!Open_TcpIpLink())
    {
        
        WaitModuleNormal();
        Close_TcpIpLink();
        DebugMsg();
        Clear_Tcp_RecBuf();
        return FALSE;
    }
     
    
    DebugMsg();
    Clear_Tcp_RecBuf();
    
    Clear_Tcp_SendBuf();

    TCP_SendBuf[0] = 0x68;
    TCP_SendBuf[1] = 0x0A;
    TCP_SendBuf[2] = 0x00;
    TCP_SendBuf[3] = 0x68;
    TCP_SendBuf[4] = SysParam[SP_DEVADDR];
    TCP_SendBuf[5] = SysParam[SP_DEVADDR+1];
    TCP_SendBuf[6] = SysParam[SP_DEVADDR+2];
    TCP_SendBuf[7] = SysParam[SP_DEVADDR+3];
    TCP_SendBuf[8] = SysParam[SP_DEVADDR+4];

    TCP_SendBuf[9]  = 0x1C;
    TCP_SendBuf[10] = 0x11;
    
    //���� 10 �ֽ�
    TCP_SendBuf[11] = SysParam[SP_LOCALIP]; 
    TCP_SendBuf[12] = SysParam[SP_LOCALIP+1];
    TCP_SendBuf[13] = SysParam[SP_LOCALIP+2];
    TCP_SendBuf[14] = SysParam[SP_LOCALIP+3];

    TCP_SendBuf[15] = SysParam[SP_NETMODE];  // ����ͨ�ŷ�ʽ

    TCP_SendBuf[16] = SysParam[SP_SERVERPORT];   // Զ�̶˿�
    TCP_SendBuf[17] = SysParam[SP_SERVERPORT+1];

    TCP_SendBuf[18] = SysParam[SP_LOCALPORT];   // ���ض˿�
    TCP_SendBuf[19] = SysParam[SP_LOCALPORT+1];

    //�����ź�ǿ��
    TCP_SendBuf[20] = SignPower;
    
    //CRC
    Crc = CRC16(TCP_SendBuf,21);
    TCP_SendBuf[21] = (BYTE)Crc;
    TCP_SendBuf[22] = (BYTE)(Crc>>8);

    TCP_SendBuf[23] = 0x16;  // ����

    TcpSendDataLen = 24;

    Retry = 0;

again:    
    Send_Tcp_Data();

    Sleep(500);
    WDog();

    // ������������������
    i = 0;
    HeartPackArrived = 0;
    while(i<5)  // 5�������ݷ���������
    {
        if (RecValidTcpFrame() == DEVFRAME)  
        {
            if ((TCP_RecBuf[StartLoc+9] == 0x9C)      // ������Ӧ������
                 && (TCP_RecBuf[StartLoc+10] == 0x11))
            {
                //for(j=StartLoc;j<TCP_RecLen;j++)
                //printf("%02x",TCP_RecBuf[j]);
                HandleDevFrame(); 
                Clear_Tcp_RecBuf();
                if (HeartPackArrived == 1)
                {
                    //DebugStr("close ip\n"); 
                    Close_TcpIpLink();
                    return TRUE;
                }
            }
        }

        Sleep(1000);
        WDog();
        i+=1;
    }

    Clear_Tcp_RecBuf();
    
    Retry++;
    if (Retry < 3)
    {
        goto again;
    }
    

    
    Close_TcpIpLink();
    return FALSE;
}
#else
// ֻ��link1 �ϱ�����
BYTE RepHeartPack(void)
{
    static BYTE i,Retry;
    static BYTE Frame;


    if (SysParam[SP_MODTYPE] == CDMA_MC323)
    {
        CurTcpLinkId = 1;
    }
    else
    {
        CurTcpLinkId = 0;
    }
    
    if (Init_TCPIP() != TRUE)
    {
        return FALSE;
    }
    
    if (Open_TcpIpLink() != TRUE)
    {
        DebugStr("Open_TcpIpLink Fail!\r\n");
        WaitModuleNormal();
        Close_TcpIpLink();
        DebugMsg();
        Clear_Tcp_RecBuf();
        return FALSE;
    }
     
    
    DebugMsg();
    Clear_Tcp_RecBuf();
    
    Clear_Tcp_SendBuf();

    TCP_SendBuf[0] = 0x68;
    TCP_SendBuf[1] = 0x0A;
    TCP_SendBuf[2] = 0x00;
    TCP_SendBuf[3] = 0x68;
    TCP_SendBuf[4] = SysParam[SP_DEVADDR];
    TCP_SendBuf[5] = SysParam[SP_DEVADDR+1];
    TCP_SendBuf[6] = SysParam[SP_DEVADDR+2];
    TCP_SendBuf[7] = SysParam[SP_DEVADDR+3];
    TCP_SendBuf[8] = SysParam[SP_DEVADDR+4];

    TCP_SendBuf[9]  = 0x1C;
    TCP_SendBuf[10] = 0x11;
    
    //���� 10 �ֽ�
    TCP_SendBuf[11] = SysParam[SP_LOCALIP]; 
    TCP_SendBuf[12] = SysParam[SP_LOCALIP+1];
    TCP_SendBuf[13] = SysParam[SP_LOCALIP+2];
    TCP_SendBuf[14] = SysParam[SP_LOCALIP+3];

    TCP_SendBuf[15] = SysParam[SP_NETMODE];  // ����ͨ�ŷ�ʽ

    TCP_SendBuf[16] = SysParam[SP_SERVERPORT];   // Զ�̶˿�
    TCP_SendBuf[17] = SysParam[SP_SERVERPORT+1];

    TCP_SendBuf[18] = SysParam[SP_LOCALPORT];   // ���ض˿�
    TCP_SendBuf[19] = SysParam[SP_LOCALPORT+1];

    //�����ź�ǿ��
    TCP_SendBuf[20] = SignPower;
    
    //CRC
    Crc = CRC16(TCP_SendBuf,21);
    TCP_SendBuf[21] = (BYTE)Crc;
    TCP_SendBuf[22] = (BYTE)(Crc>>8);

    TCP_SendBuf[23] = 0x16;  // ����

    TcpSendDataLen = 24;

    Retry = 0;

again:    
    Send_Tcp_Data();

    Sleep(500);

    // ������������������
    i = 0;
    HeartPackArrived = 0;
    while(i<15)  // 15�������ݷ���������
    {
  
        Frame = RecValidTcpFrame();
        printf("Frame=%d\r\n",Frame);
        if (Frame != FALSE)  
        {
            return Frame;
        }
        
        Sleep(1000);
       
        i+=1;
    }

    Clear_Tcp_RecBuf();
    
    Retry++;
    if (Retry < 3)
    {
        goto again;
    }
       
    Close_TcpIpLink();
    return FALSE;
}
#endif


void RepDevData(void)
{
    
    Clear_Tcp_SendBuf();

    TCP_SendBuf[0] = 0x68;
    TCP_SendBuf[1] = 0x09;  // n
    TCP_SendBuf[2] = 0x00;
    TCP_SendBuf[3] = 0x68;
    TCP_SendBuf[4] = SysParam[SP_DEVADDR];
    TCP_SendBuf[5] = SysParam[SP_DEVADDR+1];
    TCP_SendBuf[6] = SysParam[SP_DEVADDR+2];
    TCP_SendBuf[7] = SysParam[SP_DEVADDR+3];
    TCP_SendBuf[8] = SysParam[SP_DEVADDR+4];

    TCP_SendBuf[9]  = 0x1C;
    TCP_SendBuf[10] = 0x12;
    
    //���� n �ֽ�
    TCP_SendBuf[11] = 0x01; 
    
    //CRC
    Crc = CRC16(TCP_SendBuf,20);
    TCP_SendBuf[20] = (BYTE)Crc;
    TCP_SendBuf[21] = (BYTE)(Crc>>8);

    TCP_SendBuf[22] = 0x16;  // ����

    TcpSendDataLen = 23;
    Send_Tcp_Data();
}

//�����ϱ�Ӧ��
void RetDevStatus(void)
{
    if (ServerLogin == 0)
    {
        return;
    }
    
    if (DataLength == 1)
    {
        if (TCP_RecBuf[StartLoc+11] == 0x00)
        {
        
            // �ɹ�
        }
        else
        {
            // ʧ��
        }
    }
    else
    {
        // ʧ��
    }
}

void RetHeartPack(void)
{
    if (DataLength == 1)
    {
        if (TCP_RecBuf[StartLoc+11] == 0x00)
        {
            // �ɹ�
            HeartPackArrived = 1;
        }
        else
        {
            // ʧ��
            HeartPackArrived = 0;
        }
    }
    else
    {
        // ʧ��
        HeartPackArrived = 0;
    }
}

void RetDevData(void)
{
    if (ServerLogin == 0)
    {
        return;
    }

    if (DataLength == 1)
    {
        if (TCP_RecBuf[StartLoc+11] == 0x00)
        {
            // �ɹ�
        }
        else
        {
            // ʧ��
        }
    }
    else
    {
        // ʧ��
    }
}


void SetDevTime(void)
{
    static WORD Year;
    
    if (ServerLogin == 0)
    {
        return;
    }

    Clear_Tcp_SendBuf();

    TCP_SendBuf[0] = 0x68;
    TCP_SendBuf[1] = 0x01;
    TCP_SendBuf[2] = 0x00;
    TCP_SendBuf[3] = 0x68;
    TCP_SendBuf[4] = SysParam[SP_DEVADDR];
    TCP_SendBuf[5] = SysParam[SP_DEVADDR+1];
    TCP_SendBuf[6] = SysParam[SP_DEVADDR+2];
    TCP_SendBuf[7] = SysParam[SP_DEVADDR+3];
    TCP_SendBuf[8] = SysParam[SP_DEVADDR+4];

    TCP_SendBuf[9]  = 0x94;
    TCP_SendBuf[10] = 0x10;
    
    if (DataLength == 7)
    {
        Year = TCP_RecBuf[StartLoc+17];
        Year <<= 8;
        Year += TCP_RecBuf[StartLoc+16];
        Year = Year % 100;
        SysTime[0] = Year/10 * 16 + Year%10;  // ��
        SysTime[2] = TCP_RecBuf[StartLoc+15]/10 * 16 + TCP_RecBuf[StartLoc+15]%10;  // ��
        SysTime[3] = TCP_RecBuf[StartLoc+14]/10 * 16 + TCP_RecBuf[StartLoc+14]%10;; // ��
        SysTime[4] = TCP_RecBuf[StartLoc+13]/10 * 16 + TCP_RecBuf[StartLoc+13]%10;  // ʱ
        SysTime[5] = TCP_RecBuf[StartLoc+12]/10 * 16 + TCP_RecBuf[StartLoc+12]%10;  // ��
        SysTime[6] = TCP_RecBuf[StartLoc+11]/10 * 16 + TCP_RecBuf[StartLoc+11]%10;  // ��

        SetSysTime();
        
        TCP_SendBuf[11] = 0x00;  // �ɹ�
    }
    else
    {
        TCP_SendBuf[11] = 0x01;  // ʧ��
    }

    //CRC
    Crc = CRC16(TCP_SendBuf,12);
    TCP_SendBuf[12] = (BYTE)Crc;
    TCP_SendBuf[13] = (BYTE)(Crc>>8);

    TCP_SendBuf[14] = 0x16;  // ����

    TcpSendDataLen = 15;
    Send_Tcp_Data();
}

void SetDevCommMode(void)
{
    if (ServerLogin == 0)
    {
        return;
    }

    Clear_Tcp_SendBuf();

    TCP_SendBuf[0] = 0x68;
    TCP_SendBuf[1] = 0x01;
    TCP_SendBuf[2] = 0x00;
    TCP_SendBuf[3] = 0x68;
    TCP_SendBuf[4] = SysParam[SP_DEVADDR];
    TCP_SendBuf[5] = SysParam[SP_DEVADDR+1];
    TCP_SendBuf[6] = SysParam[SP_DEVADDR+2];
    TCP_SendBuf[7] = SysParam[SP_DEVADDR+3];
    TCP_SendBuf[8] = SysParam[SP_DEVADDR+4];

    TCP_SendBuf[9]  = 0x94;
    TCP_SendBuf[10] = 0x11;
    
    if (DataLength == 1)
    {
        SysParam[SP_COMMMODE] = TCP_RecBuf[StartLoc+11];   // �豸����ģʽ
        NeedSaveParam = 1;
        TCP_SendBuf[11] = 0x00;  // �ɹ�
    }
    else
    {
        TCP_SendBuf[11] = 0x01;  // ʧ��
    }

    //CRC
    Crc = CRC16(TCP_SendBuf,12);
    TCP_SendBuf[12] = (BYTE)Crc;
    TCP_SendBuf[13] = (BYTE)(Crc>>8);

    TCP_SendBuf[14] = 0x16;  // ����

    TcpSendDataLen = 15;
    Send_Tcp_Data();
}

void SetDevWirelessParam(void)
{
    if (ServerLogin == 0)
    {
        return;
    }

    Clear_Tcp_SendBuf();

    TCP_SendBuf[0] = 0x68;
    TCP_SendBuf[1] = 0x01;
    TCP_SendBuf[2] = 0x00;
    TCP_SendBuf[3] = 0x68;
    TCP_SendBuf[4] = SysParam[SP_DEVADDR];
    TCP_SendBuf[5] = SysParam[SP_DEVADDR+1];
    TCP_SendBuf[6] = SysParam[SP_DEVADDR+2];
    TCP_SendBuf[7] = SysParam[SP_DEVADDR+3];
    TCP_SendBuf[8] = SysParam[SP_DEVADDR+4];

    TCP_SendBuf[9]  = 0x94;
    TCP_SendBuf[10] = 0x12;
    
    if (DataLength == 0x14)
    {
       
        //NeedSaveParam = 1;
        TCP_SendBuf[11] = 0x00;  // �ɹ�
    }
    else
    {
        TCP_SendBuf[11] = 0x01;  // ʧ��
    }

    //CRC
    Crc = CRC16(TCP_SendBuf,12);
    TCP_SendBuf[12] = (BYTE)Crc;
    TCP_SendBuf[13] = (BYTE)(Crc>>8);

    TCP_SendBuf[14] = 0x16;  // ����

    TcpSendDataLen = 15;
    Send_Tcp_Data();
}

void SetSrvNetParam(void)
{
    if (ServerLogin == 0)
    {
        return;
    }

    Clear_Tcp_SendBuf();

    TCP_SendBuf[0] = 0x68;
    TCP_SendBuf[1] = 0x01;
    TCP_SendBuf[2] = 0x00;
    TCP_SendBuf[3] = 0x68;
    TCP_SendBuf[4] = SysParam[SP_DEVADDR];
    TCP_SendBuf[5] = SysParam[SP_DEVADDR+1];
    TCP_SendBuf[6] = SysParam[SP_DEVADDR+2];
    TCP_SendBuf[7] = SysParam[SP_DEVADDR+3];
    TCP_SendBuf[8] = SysParam[SP_DEVADDR+4];

    TCP_SendBuf[9]  = 0x94;
    TCP_SendBuf[10] = 0x13;
    
    if (DataLength == 0x09)
    {
        mcpy(&SysParam[SP_SERVERIP], &TCP_RecBuf[StartLoc+11],4);
        SysParam[SP_NETMODE]      = TCP_RecBuf[StartLoc+15];
        SysParam[SP_SERVERPORT]   = TCP_RecBuf[StartLoc+16];
        SysParam[SP_SERVERPORT+1] = TCP_RecBuf[StartLoc+17];
        NeedSaveParam = 1;
        TCP_SendBuf[11] = 0x00;  // �ɹ�
    }
    else
    {
        TCP_SendBuf[11] = 0x01;  // ʧ��
    }

    //CRC
    Crc = CRC16(TCP_SendBuf,12);
    TCP_SendBuf[12] = (BYTE)Crc;
    TCP_SendBuf[13] = (BYTE)(Crc>>8);

    TCP_SendBuf[14] = 0x16;  // ����

    TcpSendDataLen = 15;
    Send_Tcp_Data();
}

void SetDevNetParam(void)
{
    if (ServerLogin == 0)
    {
        return;
    }

    Clear_Tcp_SendBuf();

    TCP_SendBuf[0] = 0x68;
    TCP_SendBuf[1] = 0x01;
    TCP_SendBuf[2] = 0x00;
    TCP_SendBuf[3] = 0x68;
    TCP_SendBuf[4] = SysParam[SP_DEVADDR];
    TCP_SendBuf[5] = SysParam[SP_DEVADDR+1];
    TCP_SendBuf[6] = SysParam[SP_DEVADDR+2];
    TCP_SendBuf[7] = SysParam[SP_DEVADDR+3];
    TCP_SendBuf[8] = SysParam[SP_DEVADDR+4];

    TCP_SendBuf[9]  = 0x94;
    TCP_SendBuf[10] = 0x14;
    
    if (DataLength == 0x09)
    {
        mcpy(&SysParam[SP_LOCALIP], &TCP_RecBuf[StartLoc+11],4);
        SysParam[SP_NETMODE]     = TCP_RecBuf[StartLoc+15];
        SysParam[SP_LOCALPORT]   = TCP_RecBuf[StartLoc+16];
        SysParam[SP_LOCALPORT+1] = TCP_RecBuf[StartLoc+17];
        NeedSaveParam = 1;
        TCP_SendBuf[11] = 0x00;  // �ɹ�
    }
    else
    {
        TCP_SendBuf[11] = 0x01;  // ʧ��
    }

    //CRC
    Crc = CRC16(TCP_SendBuf,12);
    TCP_SendBuf[12] = (BYTE)Crc;
    TCP_SendBuf[13] = (BYTE)(Crc>>8);

    TCP_SendBuf[14] = 0x16;  // ����

    TcpSendDataLen = 15;
    Send_Tcp_Data();
}

void SetDevWorkMode(void)
{
    if (ServerLogin == 0)
    {
        return;
    }

    Clear_Tcp_SendBuf();

    TCP_SendBuf[0] = 0x68;
    TCP_SendBuf[1] = 0x01;
    TCP_SendBuf[2] = 0x00;
    TCP_SendBuf[3] = 0x68;
    TCP_SendBuf[4] = SysParam[SP_DEVADDR];
    TCP_SendBuf[5] = SysParam[SP_DEVADDR+1];
    TCP_SendBuf[6] = SysParam[SP_DEVADDR+2];
    TCP_SendBuf[7] = SysParam[SP_DEVADDR+3];
    TCP_SendBuf[8] = SysParam[SP_DEVADDR+4];

    TCP_SendBuf[9]  = 0x94;
    TCP_SendBuf[10] = 0x15;
    
    if (DataLength == 0x01)
    {
        SysParam[SP_WORKMODE]  = TCP_RecBuf[StartLoc+11];
        NeedSaveParam = 1;
        TCP_SendBuf[11] = 0x00;  // �ɹ�
    }
    else
    {
        TCP_SendBuf[11] = 0x01;  // ʧ��
    }

    //CRC
    Crc = CRC16(TCP_SendBuf,12);
    TCP_SendBuf[12] = (BYTE)Crc;
    TCP_SendBuf[13] = (BYTE)(Crc>>8);

    TCP_SendBuf[14] = 0x16;  // ����

    TcpSendDataLen = 15;
    Send_Tcp_Data();
}

void SetDevSerialParam(void)
{
    if (ServerLogin == 0)
    {
        return;
    }

    Clear_Tcp_SendBuf();

    TCP_SendBuf[0] = 0x68;
    TCP_SendBuf[1] = 0x01;
    TCP_SendBuf[2] = 0x00;
    TCP_SendBuf[3] = 0x68;
    TCP_SendBuf[4] = SysParam[SP_DEVADDR];
    TCP_SendBuf[5] = SysParam[SP_DEVADDR+1];
    TCP_SendBuf[6] = SysParam[SP_DEVADDR+2];
    TCP_SendBuf[7] = SysParam[SP_DEVADDR+3];
    TCP_SendBuf[8] = SysParam[SP_DEVADDR+4];

    TCP_SendBuf[9]  = 0x94;
    TCP_SendBuf[10] = 0x16;
    
    if (DataLength == 0x04)
    {
        SysParam[SP_BAUDRATE] = TCP_RecBuf[StartLoc+11];
        SysParam[SP_CHECKBIT] = TCP_RecBuf[StartLoc+12];
        SysParam[SP_DATABIT] = TCP_RecBuf[StartLoc+13];
        SysParam[SP_STOPBIT] = TCP_RecBuf[StartLoc+14];
        NeedSaveParam = 1;
        TCP_SendBuf[11] = 0x00;  // �ɹ�
    }
    else
    {
        TCP_SendBuf[11] = 0x01;  // ʧ��
    }

    //CRC
    Crc = CRC16(TCP_SendBuf,12);
    TCP_SendBuf[12] = (BYTE)Crc;
    TCP_SendBuf[13] = (BYTE)(Crc>>8);

    TCP_SendBuf[14] = 0x16;  // ����

    TcpSendDataLen = 15;
    Send_Tcp_Data();
}

void SetDevMobNumber(void)
{
    if (ServerLogin == 0)
    {
        return;
    }

    Clear_Tcp_SendBuf();

    TCP_SendBuf[0] = 0x68;
    TCP_SendBuf[1] = 0x01;
    TCP_SendBuf[2] = 0x00;
    TCP_SendBuf[3] = 0x68;
    TCP_SendBuf[4] = SysParam[SP_DEVADDR];
    TCP_SendBuf[5] = SysParam[SP_DEVADDR+1];
    TCP_SendBuf[6] = SysParam[SP_DEVADDR+2];
    TCP_SendBuf[7] = SysParam[SP_DEVADDR+3];
    TCP_SendBuf[8] = SysParam[SP_DEVADDR+4];

    TCP_SendBuf[9]  = 0x94;
    TCP_SendBuf[10] = 0x17;
    
    if (DataLength == 0x0C)
    {
        //Data 12
        //��������
        SysParam[SP_SELFNUM+10] = TCP_RecBuf[StartLoc+11]%16 + 0x30;
        SysParam[SP_SELFNUM+9]  = TCP_RecBuf[StartLoc+11]/16 + 0x30;
        SysParam[SP_SELFNUM+8]  = TCP_RecBuf[StartLoc+12]%16 + 0x30;
        SysParam[SP_SELFNUM+7]  = TCP_RecBuf[StartLoc+12]/16 + 0x30;
        SysParam[SP_SELFNUM+6]  = TCP_RecBuf[StartLoc+13]%16 + 0x30;
        SysParam[SP_SELFNUM+5]  = TCP_RecBuf[StartLoc+13]/16 + 0x30;
        SysParam[SP_SELFNUM+4]  = TCP_RecBuf[StartLoc+14]%16 + 0x30;
        SysParam[SP_SELFNUM+3]  = TCP_RecBuf[StartLoc+14]/16 + 0x30;
        SysParam[SP_SELFNUM+2]  = TCP_RecBuf[StartLoc+15]%16 + 0x30;
        SysParam[SP_SELFNUM+1]  = TCP_RecBuf[StartLoc+15]/16 + 0x30;
        SysParam[SP_SELFNUM+0]  = TCP_RecBuf[StartLoc+16]%16 + 0x30;
        SysParam[SP_SELFNUM+11] = 0;
        
        //�������ĺ���  6�ֽ�
        SysParam[SP_MOBLENUM+10] = TCP_RecBuf[StartLoc+17]%16 + 0x30;
        SysParam[SP_MOBLENUM+9]  = TCP_RecBuf[StartLoc+17]/16 + 0x30;
        SysParam[SP_MOBLENUM+8]  = TCP_RecBuf[StartLoc+18]%16 + 0x30;
        SysParam[SP_MOBLENUM+7]  = TCP_RecBuf[StartLoc+18]/16 + 0x30;
        SysParam[SP_MOBLENUM+6]  = TCP_RecBuf[StartLoc+19]%16 + 0x30;
        SysParam[SP_MOBLENUM+5]  = TCP_RecBuf[StartLoc+19]/16 + 0x30;
        SysParam[SP_MOBLENUM+4]  = TCP_RecBuf[StartLoc+20]%16 + 0x30;
        SysParam[SP_MOBLENUM+3]  = TCP_RecBuf[StartLoc+20]/16 + 0x30;
        SysParam[SP_MOBLENUM+2]  = TCP_RecBuf[StartLoc+21]%16 + 0x30;
        SysParam[SP_MOBLENUM+1]  = TCP_RecBuf[StartLoc+21]/16 + 0x30;
        SysParam[SP_MOBLENUM+0]  = TCP_RecBuf[StartLoc+22]%16 + 0x30;
        SysParam[SP_MOBLENUM+11] = 0;
        
        NeedSaveParam = 1;
        
        TCP_SendBuf[11] = 0x00;  // �ɹ�
    }
    else
    {
        TCP_SendBuf[11] = 0x01;  // ʧ��
    }

    //CRC
    Crc = CRC16(TCP_SendBuf,12);
    TCP_SendBuf[12] = (BYTE)Crc;
    TCP_SendBuf[13] = (BYTE)(Crc>>8);

    TCP_SendBuf[14] = 0x16;  // ����

    TcpSendDataLen = 15;
    Send_Tcp_Data();
}

void SetDevHeartTime(void)
{
    if (ServerLogin == 0)
    {
        return;
    }

    Clear_Tcp_SendBuf();

    TCP_SendBuf[0] = 0x68;
    TCP_SendBuf[1] = 0x01;
    TCP_SendBuf[2] = 0x00;
    TCP_SendBuf[3] = 0x68;
    TCP_SendBuf[4] = SysParam[SP_DEVADDR];
    TCP_SendBuf[5] = SysParam[SP_DEVADDR+1];
    TCP_SendBuf[6] = SysParam[SP_DEVADDR+2];
    TCP_SendBuf[7] = SysParam[SP_DEVADDR+3];
    TCP_SendBuf[8] = SysParam[SP_DEVADDR+4];

    TCP_SendBuf[9]  = 0x94;
    TCP_SendBuf[10] = 0x18;
    
    if (DataLength == 0x02)
    {
        SysParam[SP_HEARTTIME] = TCP_RecBuf[StartLoc+11]; 
        SysParam[SP_HEARTTIME+1]   = TCP_RecBuf[StartLoc+12];  
        NeedSaveParam = 1;
        TCP_SendBuf[11] = 0x00;  // �ɹ�
    }
    else
    {
        TCP_SendBuf[11] = 0x01;  // ʧ��
    }

    //CRC
    Crc = CRC16(TCP_SendBuf,12);
    TCP_SendBuf[12] = (BYTE)Crc;
    TCP_SendBuf[13] = (BYTE)(Crc>>8);

    TCP_SendBuf[14] = 0x16;  // ����

    TcpSendDataLen = 15;
    Send_Tcp_Data();
}

void SetDevRepSign(void)
{
    if (ServerLogin == 0)
    {
        return;
    }

    Clear_Tcp_SendBuf();

    TCP_SendBuf[0] = 0x68;
    TCP_SendBuf[1] = 0x01;
    TCP_SendBuf[2] = 0x00;
    TCP_SendBuf[3] = 0x68;
    TCP_SendBuf[4] = SysParam[SP_DEVADDR];
    TCP_SendBuf[5] = SysParam[SP_DEVADDR+1];
    TCP_SendBuf[6] = SysParam[SP_DEVADDR+2];
    TCP_SendBuf[7] = SysParam[SP_DEVADDR+3];
    TCP_SendBuf[8] = SysParam[SP_DEVADDR+4];

    TCP_SendBuf[9]  = 0x94;
    TCP_SendBuf[10] = 0x19;
    
    if (DataLength == 0x01)
    {
       
        
        TCP_SendBuf[11] = 0x00;  // �ɹ�
    }
    else
    {
        TCP_SendBuf[11] = 0x01;  // ʧ��
    }

    //CRC
    Crc = CRC16(TCP_SendBuf,12);
    TCP_SendBuf[12] = (BYTE)Crc;
    TCP_SendBuf[13] = (BYTE)(Crc>>8);

    TCP_SendBuf[14] = 0x16;  // ����

    TcpSendDataLen = 15;
    Send_Tcp_Data();
}

void SetDevReadTime(void)
{
    if (ServerLogin == 0)
    {
        return;
    }

    Clear_Tcp_SendBuf();

    TCP_SendBuf[0] = 0x68;
    TCP_SendBuf[1] = 0x01;
    TCP_SendBuf[2] = 0x00;
    TCP_SendBuf[3] = 0x68;
    TCP_SendBuf[4] = SysParam[SP_DEVADDR];
    TCP_SendBuf[5] = SysParam[SP_DEVADDR+1];
    TCP_SendBuf[6] = SysParam[SP_DEVADDR+2];
    TCP_SendBuf[7] = SysParam[SP_DEVADDR+3];
    TCP_SendBuf[8] = SysParam[SP_DEVADDR+4];

    TCP_SendBuf[9]  = 0x94;
    TCP_SendBuf[10] = 0x1A;
    
    if (DataLength == 0x02)
    {
        
        TCP_SendBuf[11] = 0x00;  // �ɹ�
    }
    else
    {
        TCP_SendBuf[11] = 0x01;  // ʧ��
    }

    //CRC
    Crc = CRC16(TCP_SendBuf,12);
    TCP_SendBuf[12] = (BYTE)Crc;
    TCP_SendBuf[13] = (BYTE)(Crc>>8);

    TCP_SendBuf[14] = 0x16;  // ����

    TcpSendDataLen = 15;
    Send_Tcp_Data();
}

void SetDevMeterCount(void)
{
    if (ServerLogin == 0)
    {
        return;
    }

    Clear_Tcp_SendBuf();

    TCP_SendBuf[0] = 0x68;
    TCP_SendBuf[1] = 0x01;
    TCP_SendBuf[2] = 0x00;
    TCP_SendBuf[3] = 0x68;
    TCP_SendBuf[4] = SysParam[SP_DEVADDR];
    TCP_SendBuf[5] = SysParam[SP_DEVADDR+1];
    TCP_SendBuf[6] = SysParam[SP_DEVADDR+2];
    TCP_SendBuf[7] = SysParam[SP_DEVADDR+3];
    TCP_SendBuf[8] = SysParam[SP_DEVADDR+4];

    TCP_SendBuf[9]  = 0x94;
    TCP_SendBuf[10] = 0x1B;
    
    if (DataLength == 0x01)
    {   
        
        TCP_SendBuf[11] = 0x00;  // �ɹ�
    }
    else
    {
        TCP_SendBuf[11] = 0x01;  // ʧ��
    }

    //CRC
    Crc = CRC16(TCP_SendBuf,12);
    TCP_SendBuf[12] = (BYTE)Crc;
    TCP_SendBuf[13] = (BYTE)(Crc>>8);

    TCP_SendBuf[14] = 0x16;  // ����

    TcpSendDataLen = 15;
    Send_Tcp_Data();
}

void SetDevMeterParam(void)
{
    if (ServerLogin == 0)
    {
        return;
    }

    Clear_Tcp_SendBuf();

    TCP_SendBuf[0] = 0x68;
    TCP_SendBuf[1] = 0x01;
    TCP_SendBuf[2] = 0x00;
    TCP_SendBuf[3] = 0x68;
    TCP_SendBuf[4] = SysParam[SP_DEVADDR];
    TCP_SendBuf[5] = SysParam[SP_DEVADDR+1];
    TCP_SendBuf[6] = SysParam[SP_DEVADDR+2];
    TCP_SendBuf[7] = SysParam[SP_DEVADDR+3];
    TCP_SendBuf[8] = SysParam[SP_DEVADDR+4];

    TCP_SendBuf[9]  = 0x94;
    TCP_SendBuf[10] = 0x1C;
    
    if (DataLength == 0x26)
    {
       
        
        TCP_SendBuf[11] = 0x00;  // �ɹ�
    }
    else
    {
        TCP_SendBuf[11] = 0x01;  // ʧ��
    }

    //CRC
    Crc = CRC16(TCP_SendBuf,12);
    TCP_SendBuf[12] = (BYTE)Crc;
    TCP_SendBuf[13] = (BYTE)(Crc>>8);

    TCP_SendBuf[14] = 0x16;  // ����

    TcpSendDataLen = 15;
    Send_Tcp_Data();
}

void ClearDevFlowRate(void)
{
    if (ServerLogin == 0)
    {
        return;
    }

    Clear_Tcp_SendBuf();

    TCP_SendBuf[0] = 0x68;
    TCP_SendBuf[1] = 0x01;
    TCP_SendBuf[2] = 0x00;
    TCP_SendBuf[3] = 0x68;
    TCP_SendBuf[4] = SysParam[SP_DEVADDR];
    TCP_SendBuf[5] = SysParam[SP_DEVADDR+1];
    TCP_SendBuf[6] = SysParam[SP_DEVADDR+2];
    TCP_SendBuf[7] = SysParam[SP_DEVADDR+3];
    TCP_SendBuf[8] = SysParam[SP_DEVADDR+4];

    TCP_SendBuf[9]  = 0x94;
    TCP_SendBuf[10] = 0x1E;
    
    if (DataLength == 0x00)
    {
        *(DWORD *)&SysParam[SP_FLOWRATE] = 0;
        NeedSaveParam = 1;
        TCP_SendBuf[11] = 0x00;  // �ɹ�
    }
    else
    {
        TCP_SendBuf[11] = 0x01;  // ʧ��
    }

    //CRC
    Crc = CRC16(TCP_SendBuf,12);
    TCP_SendBuf[12] = (BYTE)Crc;
    TCP_SendBuf[13] = (BYTE)(Crc>>8);

    TCP_SendBuf[14] = 0x16;  // ����

    TcpSendDataLen = 15;
    Send_Tcp_Data();
}


void GetDevUserInfo(void)
{
    if (ServerLogin == 0)
    {
        return;
    }

    Clear_Tcp_SendBuf();

    TCP_SendBuf[0] = 0x68;
    TCP_SendBuf[1] = 0x40;
    TCP_SendBuf[2] = 0x00;
    TCP_SendBuf[3] = 0x68;
    TCP_SendBuf[4] = SysParam[SP_DEVADDR];
    TCP_SendBuf[5] = SysParam[SP_DEVADDR+1];
    TCP_SendBuf[6] = SysParam[SP_DEVADDR+2];
    TCP_SendBuf[7] = SysParam[SP_DEVADDR+3];
    TCP_SendBuf[8] = SysParam[SP_DEVADDR+4];

    TCP_SendBuf[9]  = 0x91;
    TCP_SendBuf[10] = 0x22;
    
 
    //Data  64 �ֽ�
        
    //User name  32byte
    mcpy(&TCP_SendBuf[11],&SysParam[SP_NETUSERNAME],32);
    //Pass Word  32byte
    mcpy(&TCP_SendBuf[43],&SysParam[SP_NETPASSWORD],32);
        
    

    //CRC
    Crc = CRC16(TCP_SendBuf,75);
    TCP_SendBuf[75] = (BYTE)Crc;
    TCP_SendBuf[76] = (BYTE)(Crc>>8);

    TCP_SendBuf[77] = 0x16;  // ����

    TcpSendDataLen = 78;
    Send_Tcp_Data();
}

void GetDevSleepTime(void)
{
    if (ServerLogin == 0)
    {
        return;
    }

    Clear_Tcp_SendBuf();

    TCP_SendBuf[0] = 0x68;
    TCP_SendBuf[1] = 0x03;
    TCP_SendBuf[2] = 0x00;
    TCP_SendBuf[3] = 0x68;
    TCP_SendBuf[4] = SysParam[SP_DEVADDR];
    TCP_SendBuf[5] = SysParam[SP_DEVADDR+1];
    TCP_SendBuf[6] = SysParam[SP_DEVADDR+2];
    TCP_SendBuf[7] = SysParam[SP_DEVADDR+3];
    TCP_SendBuf[8] = SysParam[SP_DEVADDR+4];

    TCP_SendBuf[9]  = 0x91;
    TCP_SendBuf[10] = 0x24;
    
    //Data  3 �ֽ�
    TCP_SendBuf[11] = SysParam[SP_SLEEPTIME];
    TCP_SendBuf[12] = SysParam[SP_SLEEPTIME+1];
    TCP_SendBuf[13] = SysParam[SP_SLEEPTIME+2];

    //CRC
    Crc = CRC16(TCP_SendBuf,14);
    TCP_SendBuf[14] = (BYTE)Crc;
    TCP_SendBuf[15] = (BYTE)(Crc>>8);

    TCP_SendBuf[16] = 0x16;  // ����

    TcpSendDataLen = 17;
    Send_Tcp_Data();
}

void GetDevNetInfo(void)
{
    if (ServerLogin == 0)
    {
        return;
    }

    Clear_Tcp_SendBuf();

    TCP_SendBuf[0] = 0x68;
    TCP_SendBuf[1] = 0x08;
    TCP_SendBuf[2] = 0x00;
    TCP_SendBuf[3] = 0x68;
    TCP_SendBuf[4] = SysParam[SP_DEVADDR];
    TCP_SendBuf[5] = SysParam[SP_DEVADDR+1];
    TCP_SendBuf[6] = SysParam[SP_DEVADDR+2];
    TCP_SendBuf[7] = SysParam[SP_DEVADDR+3];
    TCP_SendBuf[8] = SysParam[SP_DEVADDR+4];

    TCP_SendBuf[9]  = 0x91;
    TCP_SendBuf[10] = 0x25;
    
    //Data  8 �ֽ�
    TCP_SendBuf[11] = 255;  
    TCP_SendBuf[12] = 255;  
    TCP_SendBuf[13] = 0;  
    TCP_SendBuf[14] = 0;  

    TCP_SendBuf[15] = 0;  
    TCP_SendBuf[16] = 0;  
    TCP_SendBuf[17] = 0;  
    TCP_SendBuf[18] = 0;  

    //CRC
    Crc = CRC16(TCP_SendBuf,19);
    TCP_SendBuf[19] = (BYTE)Crc;
    TCP_SendBuf[20] = (BYTE)(Crc>>8);

    TCP_SendBuf[21] = 0x16;  // ����

    TcpSendDataLen = 22;
    Send_Tcp_Data();
}

void GetDevMacAddr(void)
{
    if (ServerLogin == 0)
    {
        return;
    }

    Clear_Tcp_SendBuf();

    TCP_SendBuf[0] = 0x68;
    TCP_SendBuf[1] = 0x07;
    TCP_SendBuf[2] = 0x00;
    TCP_SendBuf[3] = 0x68;
    TCP_SendBuf[4] = SysParam[SP_DEVADDR];
    TCP_SendBuf[5] = SysParam[SP_DEVADDR+1];
    TCP_SendBuf[6] = SysParam[SP_DEVADDR+2];
    TCP_SendBuf[7] = SysParam[SP_DEVADDR+3];
    TCP_SendBuf[8] = SysParam[SP_DEVADDR+4];

    TCP_SendBuf[9]  = 0x91;
    TCP_SendBuf[10] = 0x27;
    
    //Data  7 �ֽ�
    TCP_SendBuf[11] = 0x02;
    TCP_SendBuf[12] = 0x02;  
    TCP_SendBuf[13] = 0x02;  
    TCP_SendBuf[14] = 0x02;  
    TCP_SendBuf[15] = 0x02;  
    TCP_SendBuf[16] = 0x02;  
    TCP_SendBuf[17] = 0x02;  
    

    //CRC
    Crc = CRC16(TCP_SendBuf,18);
    TCP_SendBuf[18] = (BYTE)Crc;
    TCP_SendBuf[19] = (BYTE)(Crc>>8);

    TCP_SendBuf[20] = 0x16;  // ����

    TcpSendDataLen = 21;
    Send_Tcp_Data();
}
#if 0
void GetExdevStatus(void)
{
    static BYTE Hum;
        
    if (ServerLogin == 0)
    {
        return;
    }

    Clear_Tcp_SendBuf();

    TCP_SendBuf[0] = 0x68;
    TCP_SendBuf[1] = 0x11;
    TCP_SendBuf[2] = 0x00;
    TCP_SendBuf[3] = 0x68;
    TCP_SendBuf[4] = SysParam[SP_DEVADDR];
    TCP_SendBuf[5] = SysParam[SP_DEVADDR+1];
    TCP_SendBuf[6] = SysParam[SP_DEVADDR+2];
    TCP_SendBuf[7] = SysParam[SP_DEVADDR+3];
    TCP_SendBuf[8] = SysParam[SP_DEVADDR+4];

    TCP_SendBuf[9]  = 0x91;
    TCP_SendBuf[10] = 0x28;
    
    //Data  17 �ֽ�
  	//if (GET_ALARM(ALM_AC1))
    //{
    //    TCP_SendBuf[11] = 0xFF;
    //}
    //else
    //{
    //    TCP_SendBuf[11] = AirCon1Working();    // �յ�1״̬
    //}
    TCP_SendBuf[12] = SysParam[SP_AC1_RUNTIME];
    TCP_SendBuf[13] = SysParam[SP_AC1_RUNTIME+1];
    TCP_SendBuf[14] = SysParam[SP_AC1_RUNTIME+2];
    TCP_SendBuf[15] = SysParam[SP_AC1_RUNTIME+3];

    if (GET_ALARM(ALM_AC2))
    {
        TCP_SendBuf[16] = 0xFF;
    }
    else
    {
        TCP_SendBuf[16] = AirCon2Working();    // �յ�2״̬
    }
    TCP_SendBuf[17] = SysParam[SP_AC2_RUNTIME];
    TCP_SendBuf[18] = SysParam[SP_AC2_RUNTIME+1];
    TCP_SendBuf[19] = SysParam[SP_AC2_RUNTIME+2];
    TCP_SendBuf[20] = SysParam[SP_AC2_RUNTIME+3];

    if (GET_ALARM(ALM_EXDEV))
    {
        TCP_SendBuf[21] = 0xFF;
    }
    else
    {
        if ((SysParam[SP_TEST_ENABLE] == TRUE) && 
            (SysParam[SP_EXDEV_MODE] != NUIONMODE_NONE) &&
            (SysParam[SP_TEST_TIMEDIS] > 0))
        {
            TCP_SendBuf[21] = SysParam[SP_TEST_STATUS];
        }
        else
        {
            TCP_SendBuf[21] = ExDevWorking();    // �����豸״̬
        }
    }
    TCP_SendBuf[22] = SysParam[SP_EXDEV_RUNTIME];
    TCP_SendBuf[23] = SysParam[SP_EXDEV_RUNTIME+1];
    TCP_SendBuf[24] = SysParam[SP_EXDEV_RUNTIME+2];
    TCP_SendBuf[25] = SysParam[SP_EXDEV_RUNTIME+3];

    if(GetTmp())
    {
        TCP_SendBuf[26] = (BYTE)Tmp0;   // �¶�
    }
    else
    {
        TCP_SendBuf[26] = 0xFF;
    }

    Hum = GetHum();
    if (Hum > 100)
    {
        TCP_SendBuf[27] = 0xFF; 
    }
    else
    {
        TCP_SendBuf[27] = Hum;   // ʪ��
    }
    //CRC
    Crc = CRC16(TCP_SendBuf,28);
    TCP_SendBuf[28] = (BYTE)Crc;
    TCP_SendBuf[29] = (BYTE)(Crc>>8);

    TCP_SendBuf[30] = 0x16;  // ����

    TcpSendDataLen = 31;
    Send_Tcp_Data();
}

void GetExdevPararm(void)
{
    if (ServerLogin == 0)
    {
        return;
    }

    Clear_Tcp_SendBuf();

    TCP_SendBuf[0] = 0x68;
    TCP_SendBuf[1] = 0x0F;
    TCP_SendBuf[2] = 0x00;
    TCP_SendBuf[3] = 0x68;
    TCP_SendBuf[4] = SysParam[SP_DEVADDR];
    TCP_SendBuf[5] = SysParam[SP_DEVADDR+1];
    TCP_SendBuf[6] = SysParam[SP_DEVADDR+2];
    TCP_SendBuf[7] = SysParam[SP_DEVADDR+3];
    TCP_SendBuf[8] = SysParam[SP_DEVADDR+4];

    TCP_SendBuf[9]  = 0x91;
    TCP_SendBuf[10] = 0x29;
    
    //Data  15 �ֽ�
    //AC1
    TCP_SendBuf[11] = SysParam[SP_AC1_CTL];      //�յ�1���Ʒ�ʽ 
    TCP_SendBuf[12] = SysParam[SP_AC1_MODE];     //�յ�1����ģʽ
    TCP_SendBuf[13] = SysParam[SP_AC1_COOL];   //�յ�1���������¶�
    TCP_SendBuf[14] = SysParam[SP_AC1_HOT];   // �յ�1���������¶�
    TCP_SendBuf[15] = SysParam[SP_AC1_TIME];   //�յ�1�������ʱ��    ��λ: ����

    //AC2
    TCP_SendBuf[16] = SysParam[SP_AC2_CTL];  
    TCP_SendBuf[17] = SysParam[SP_AC2_MODE];  
    TCP_SendBuf[18] = SysParam[SP_AC2_COOL];  
    TCP_SendBuf[19] = SysParam[SP_AC2_HOT];  
    TCP_SendBuf[20] = SysParam[SP_AC2_TIME];  

    //DEV
    TCP_SendBuf[21] = SysParam[SP_EXDEV_CTL];    //  �����豸���Ʒ�ʽ 
    TCP_SendBuf[22] = SysParam[SP_EXDEV_MODE];    //  �����豸����ģʽ
    TCP_SendBuf[23] = SysParam[SP_EXDEV_START];   //  �����豸�����¶�
    TCP_SendBuf[24] = SysParam[SP_EXDEV_TIME];   //   �����豸�������ʱ��  

    //�յ��ֻ����� ��λ:Сʱ
    TCP_SendBuf[25] = SysParam[SP_AC_SWTIME];  

  

    //CRC
    Crc = CRC16(TCP_SendBuf,26);
    TCP_SendBuf[26] = (BYTE)Crc;
    TCP_SendBuf[27] = (BYTE)(Crc>>8);

    TCP_SendBuf[28] = 0x16;  // ����

    TcpSendDataLen = 29;
    Send_Tcp_Data();
}

void GetTestModeParam(void)
{
    if (ServerLogin == 0)
    {
        return;
    }

    Clear_Tcp_SendBuf();

    TCP_SendBuf[0] = 0x68;
    TCP_SendBuf[1] = 0x03;
    TCP_SendBuf[2] = 0x00;
    TCP_SendBuf[3] = 0x68;
    TCP_SendBuf[4] = SysParam[SP_DEVADDR];
    TCP_SendBuf[5] = SysParam[SP_DEVADDR+1];
    TCP_SendBuf[6] = SysParam[SP_DEVADDR+2];
    TCP_SendBuf[7] = SysParam[SP_DEVADDR+3];
    TCP_SendBuf[8] = SysParam[SP_DEVADDR+4];

    TCP_SendBuf[9]  = 0x91;
    TCP_SendBuf[10] = 0x2A;
    
    //Data  3 �ֽ�
    TCP_SendBuf[11] = SysParam[SP_TEST_ENABLE];   // �ܺĲ���ģʽʹ��
    TCP_SendBuf[12] = SysParam[SP_TEST_TIMEDIS];  // �л�ʱ����  : ��
    TCP_SendBuf[13] = SysParam[SP_TEST_HOUR];     // �л�ʱ��  : ʱ�䵽Сʱ

    //CRC
    Crc = CRC16(TCP_SendBuf,14);
    TCP_SendBuf[14] = (BYTE)Crc;
    TCP_SendBuf[15] = (BYTE)(Crc>>8);

    TCP_SendBuf[16] = 0x16;  // ����

    TcpSendDataLen = 17;
    Send_Tcp_Data();
}

#endif


void GetDevIpMode(void)
{
    if (ServerLogin == 0)
    {
        return;
    }

    Clear_Tcp_SendBuf();

    TCP_SendBuf[0] = 0x68;
    TCP_SendBuf[1] = 0x01;
    TCP_SendBuf[2] = 0x00;
    TCP_SendBuf[3] = 0x68;
    TCP_SendBuf[4] = SysParam[SP_DEVADDR];
    TCP_SendBuf[5] = SysParam[SP_DEVADDR+1];
    TCP_SendBuf[6] = SysParam[SP_DEVADDR+2];
    TCP_SendBuf[7] = SysParam[SP_DEVADDR+3];
    TCP_SendBuf[8] = SysParam[SP_DEVADDR+4];

    TCP_SendBuf[9]  = 0x91;
    TCP_SendBuf[10] = 0x26;
    
    //Data  1 �ֽ�  
    TCP_SendBuf[11] = 0x02;   //1: ��̬�� 2: DHCP  3: PPPOE

    //CRC
    Crc = CRC16(TCP_SendBuf,12);
    TCP_SendBuf[12] = (BYTE)Crc;
    TCP_SendBuf[13] = (BYTE)(Crc>>8);

    TCP_SendBuf[14] = 0x16;  // ����

    TcpSendDataLen = 15;
    Send_Tcp_Data();
}

void SetDevUserInfo(void)
{
    if (ServerLogin == 0)
    {
        return;
    }

    Clear_Tcp_SendBuf();

    TCP_SendBuf[0] = 0x68;
    TCP_SendBuf[1] = 0x01;
    TCP_SendBuf[2] = 0x00;
    TCP_SendBuf[3] = 0x68;
    TCP_SendBuf[4] = SysParam[SP_DEVADDR];
    TCP_SendBuf[5] = SysParam[SP_DEVADDR+1];
    TCP_SendBuf[6] = SysParam[SP_DEVADDR+2];
    TCP_SendBuf[7] = SysParam[SP_DEVADDR+3];
    TCP_SendBuf[8] = SysParam[SP_DEVADDR+4];

    TCP_SendBuf[9]  = 0x94;
    TCP_SendBuf[10] = 0x22;
    
    if (DataLength == 0x32)
    {
        
        TCP_SendBuf[11] = 0x00;  // �ɹ�
    }
    else
    {
        TCP_SendBuf[11] = 0x01;  // ʧ��
    }

    //CRC
    Crc = CRC16(TCP_SendBuf,12);
    TCP_SendBuf[12] = (BYTE)Crc;
    TCP_SendBuf[13] = (BYTE)(Crc>>8);

    TCP_SendBuf[14] = 0x16;  // ����

    TcpSendDataLen = 15;
    Send_Tcp_Data();
}

void SetDevAddress(void)
{
    if (ServerLogin == 0)
    {
        return;
    }

    Clear_Tcp_SendBuf();

    TCP_SendBuf[0] = 0x68;
    TCP_SendBuf[1] = 0x01;
    TCP_SendBuf[2] = 0x00;
    TCP_SendBuf[3] = 0x68;
    TCP_SendBuf[4] = SysParam[SP_DEVADDR];
    TCP_SendBuf[5] = SysParam[SP_DEVADDR+1];
    TCP_SendBuf[6] = SysParam[SP_DEVADDR+2];
    TCP_SendBuf[7] = SysParam[SP_DEVADDR+3];
    TCP_SendBuf[8] = SysParam[SP_DEVADDR+4];

    TCP_SendBuf[9]  = 0x94;
    TCP_SendBuf[10] = 0x23;
    
    if (DataLength == 0x05)
    {
        //SysParam[SP_DEVADDR]   = TCP_RecBuf[StartLoc+11];
        //SysParam[SP_DEVADDR+1] = TCP_RecBuf[StartLoc+12];
        //SysParam[SP_DEVADDR+2] = TCP_RecBuf[StartLoc+13];
        //SysParam[SP_DEVADDR+3] = TCP_RecBuf[StartLoc+14];
        //SysParam[SP_DEVADDR+4] = TCP_RecBuf[StartLoc+15];
        mcpy(DevAddrTmp,&TCP_RecBuf[StartLoc+11],5);
       
            
        NeedSaveParam = 1;
        TCP_SendBuf[11] = 0x00;  // �ɹ�
    }
    else
    {
        TCP_SendBuf[11] = 0x01;  // ʧ��
    }

    //CRC
    Crc = CRC16(TCP_SendBuf,12);
    TCP_SendBuf[12] = (BYTE)Crc;
    TCP_SendBuf[13] = (BYTE)(Crc>>8);

    TCP_SendBuf[14] = 0x16;  // ����

    TcpSendDataLen = 15;
    Send_Tcp_Data();
}

void SetDevSleepTime(void)
{
    if (ServerLogin == 0)
    {
        return;
    }

    Clear_Tcp_SendBuf();

    TCP_SendBuf[0] = 0x68;
    TCP_SendBuf[1] = 0x01;
    TCP_SendBuf[2] = 0x00;
    TCP_SendBuf[3] = 0x68;
    TCP_SendBuf[4] = SysParam[SP_DEVADDR];
    TCP_SendBuf[5] = SysParam[SP_DEVADDR+1];
    TCP_SendBuf[6] = SysParam[SP_DEVADDR+2];
    TCP_SendBuf[7] = SysParam[SP_DEVADDR+3];
    TCP_SendBuf[8] = SysParam[SP_DEVADDR+4];

    TCP_SendBuf[9]  = 0x94;
    TCP_SendBuf[10] = 0x24;
    
    if (DataLength == 0x03)
    {
        SysParam[SP_SLEEPTIME] = TCP_RecBuf[StartLoc+11];
        SysParam[SP_SLEEPTIME+1] = TCP_RecBuf[StartLoc+12];
        SysParam[SP_SLEEPTIME+2] = TCP_RecBuf[StartLoc+13];
        SysParam[SP_SLEEPTIME+3] = 0;
        NeedSaveParam = 1;
        TCP_SendBuf[11] = 0x00;  // �ɹ�
    }
    else
    {
        TCP_SendBuf[11] = 0x01;  // ʧ��
    }

    //CRC
    Crc = CRC16(TCP_SendBuf,12);
    TCP_SendBuf[12] = (BYTE)Crc;
    TCP_SendBuf[13] = (BYTE)(Crc>>8);

    TCP_SendBuf[14] = 0x16;  // ����

    TcpSendDataLen = 15;
    Send_Tcp_Data();
}

void SetDevNetInfo(void)
{
    if (ServerLogin == 0)
    {
        return;
    }

    Clear_Tcp_SendBuf();

    TCP_SendBuf[0] = 0x68;
    TCP_SendBuf[1] = 0x01;
    TCP_SendBuf[2] = 0x00;
    TCP_SendBuf[3] = 0x68;
    TCP_SendBuf[4] = SysParam[SP_DEVADDR];
    TCP_SendBuf[5] = SysParam[SP_DEVADDR+1];
    TCP_SendBuf[6] = SysParam[SP_DEVADDR+2];
    TCP_SendBuf[7] = SysParam[SP_DEVADDR+3];
    TCP_SendBuf[8] = SysParam[SP_DEVADDR+4];

    TCP_SendBuf[9]  = 0x94;
    TCP_SendBuf[10] = 0x25;
    
    if (DataLength == 0x08)
    {
        
        TCP_SendBuf[11] = 0x00;  // �ɹ�
    }
    else
    {
        TCP_SendBuf[11] = 0x01;  // ʧ��
    }

    //CRC
    Crc = CRC16(TCP_SendBuf,12);
    TCP_SendBuf[12] = (BYTE)Crc;
    TCP_SendBuf[13] = (BYTE)(Crc>>8);

    TCP_SendBuf[14] = 0x16;  // ����

    TcpSendDataLen = 15;
    Send_Tcp_Data();
}

void SetDevIpMode(void)
{
    if (ServerLogin == 0)
    {
        return;
    }

    Clear_Tcp_SendBuf();

    TCP_SendBuf[0] = 0x68;
    TCP_SendBuf[1] = 0x01;
    TCP_SendBuf[2] = 0x00;
    TCP_SendBuf[3] = 0x68;
    TCP_SendBuf[4] = SysParam[SP_DEVADDR];
    TCP_SendBuf[5] = SysParam[SP_DEVADDR+1];
    TCP_SendBuf[6] = SysParam[SP_DEVADDR+2];
    TCP_SendBuf[7] = SysParam[SP_DEVADDR+3];
    TCP_SendBuf[8] = SysParam[SP_DEVADDR+4];

    TCP_SendBuf[9]  = 0x94;
    TCP_SendBuf[10] = 0x26;
    
    if (DataLength == 0x01)
    {
        
        TCP_SendBuf[11] = 0x00;  // �ɹ�
    }
    else
    {
        TCP_SendBuf[11] = 0x01;  // ʧ��
    }

    //CRC
    Crc = CRC16(TCP_SendBuf,12);
    TCP_SendBuf[12] = (BYTE)Crc;
    TCP_SendBuf[13] = (BYTE)(Crc>>8);

    TCP_SendBuf[14] = 0x16;  // ����

    TcpSendDataLen = 15;
    Send_Tcp_Data();
}

#if 0
void SetExdevParam(void)
{
    if (ServerLogin == 0)
    {
        return;
    }

    Clear_Tcp_SendBuf();

    TCP_SendBuf[0] = 0x68;
    TCP_SendBuf[1] = 0x01;
    TCP_SendBuf[2] = 0x00;
    TCP_SendBuf[3] = 0x68;
    TCP_SendBuf[4] = SysParam[SP_DEVADDR];
    TCP_SendBuf[5] = SysParam[SP_DEVADDR+1];
    TCP_SendBuf[6] = SysParam[SP_DEVADDR+2];
    TCP_SendBuf[7] = SysParam[SP_DEVADDR+3];
    TCP_SendBuf[8] = SysParam[SP_DEVADDR+4];

    TCP_SendBuf[9]  = 0x94;
    TCP_SendBuf[10] = 0x27;
    
    if (DataLength == 0x0F)
    {
        SysParam[SP_AC1_CTL]  = TCP_RecBuf[StartLoc+11];
        SysParam[SP_AC1_MODE] = TCP_RecBuf[StartLoc+12];
        SysParam[SP_AC1_COOL] = TCP_RecBuf[StartLoc+13];
        SysParam[SP_AC1_HOT]  = TCP_RecBuf[StartLoc+14];
        SysParam[SP_AC1_TIME] = TCP_RecBuf[StartLoc+15];

        SysParam[SP_AC2_CTL]  = TCP_RecBuf[StartLoc+16];
        SysParam[SP_AC2_MODE] = TCP_RecBuf[StartLoc+17];
        SysParam[SP_AC2_COOL] = TCP_RecBuf[StartLoc+18];
        SysParam[SP_AC2_HOT]  = TCP_RecBuf[StartLoc+19];
        SysParam[SP_AC2_TIME] = TCP_RecBuf[StartLoc+20];

        SysParam[SP_EXDEV_CTL]   = TCP_RecBuf[StartLoc+21];
        SysParam[SP_EXDEV_MODE]  = TCP_RecBuf[StartLoc+22];
        SysParam[SP_EXDEV_START] = TCP_RecBuf[StartLoc+23];
        SysParam[SP_EXDEV_TIME]  = TCP_RecBuf[StartLoc+24];

        SysParam[SP_AC_SWTIME]   = TCP_RecBuf[StartLoc+25];
        
        NeedSaveParam = 1;
        TCP_SendBuf[11] = 0x00;  // �ɹ�
    }
    else
    {
        TCP_SendBuf[11] = 0x01;  // ʧ��
    }

    //CRC
    Crc = CRC16(TCP_SendBuf,12);
    TCP_SendBuf[12] = (BYTE)Crc;
    TCP_SendBuf[13] = (BYTE)(Crc>>8);

    TCP_SendBuf[14] = 0x16;  // ����

    TcpSendDataLen = 15;
    Send_Tcp_Data();
}

void SetTestModeParam(void)
{
    if (ServerLogin == 0)
    {
        return;
    }

    Clear_Tcp_SendBuf();

    TCP_SendBuf[0] = 0x68;
    TCP_SendBuf[1] = 0x01;
    TCP_SendBuf[2] = 0x00;
    TCP_SendBuf[3] = 0x68;
    TCP_SendBuf[4] = SysParam[SP_DEVADDR];
    TCP_SendBuf[5] = SysParam[SP_DEVADDR+1];
    TCP_SendBuf[6] = SysParam[SP_DEVADDR+2];
    TCP_SendBuf[7] = SysParam[SP_DEVADDR+3];
    TCP_SendBuf[8] = SysParam[SP_DEVADDR+4];

    TCP_SendBuf[9]  = 0x94;
    TCP_SendBuf[10] = 0x28;
    
    if (DataLength == 0x03)
    {
        SysParam[SP_TEST_ENABLE]  = TCP_RecBuf[StartLoc+11];
        SysParam[SP_TEST_TIMEDIS] = TCP_RecBuf[StartLoc+12];
        SysParam[SP_TEST_HOUR]    = TCP_RecBuf[StartLoc+13];
        NeedSaveParam = 1;
        if (1 == SysParam[SP_TEST_ENABLE])
        {
            SetNextSwitchTime();
        }
        TCP_SendBuf[11] = 0x00;  // �ɹ�
    }
    else
    {
        TCP_SendBuf[11] = 0x01;  // ʧ��
    }

    //CRC
    Crc = CRC16(TCP_SendBuf,12);
    TCP_SendBuf[12] = (BYTE)Crc;
    TCP_SendBuf[13] = (BYTE)(Crc>>8);

    TCP_SendBuf[14] = 0x16;  // ����

    TcpSendDataLen = 15;
    Send_Tcp_Data();
}
#endif


void SleepDevice(void)
{
    Clear_Tcp_SendBuf();
    
    TCP_SendBuf[0] = 0x68;
    TCP_SendBuf[1] = 0x01;
    TCP_SendBuf[2] = 0x00;
    TCP_SendBuf[3] = 0x68;
    TCP_SendBuf[4] = SysParam[SP_DEVADDR];
    TCP_SendBuf[5] = SysParam[SP_DEVADDR+1];
    TCP_SendBuf[6] = SysParam[SP_DEVADDR+2];
    TCP_SendBuf[7] = SysParam[SP_DEVADDR+3];
    TCP_SendBuf[8] = SysParam[SP_DEVADDR+4];

    TCP_SendBuf[9]  = 0x92;
    TCP_SendBuf[10] = 0x11;
    
    if (DataLength == 6)
    {
        if (mcmp(&TCP_RecBuf[StartLoc+11],&SysParam[SP_PASSWORD],6))
        {
            TCP_SendBuf[11] = 0x00;  // �ɹ�
            NeedSleep   = 1;
            ServerLogin = 0;  // ���ߵ�ͬʱ�˳���½
        }
        else
        {
            TCP_SendBuf[11] = 0x01;  // ʧ��
        }
    }
    else if (DataLength == 0)
    {
        TCP_SendBuf[11] = 0x00;  // �ɹ�
        //NeedReset = 1;
        NeedSleep   = 1;
        ServerLogin = 0;  // ���ߵ�ͬʱ�˳���½
    }
    else
    {
        TCP_SendBuf[11] = 0x01;  // ʧ��
    }
    
    //TCP_SendBuf[11] = 0x00;  // �ɹ�
    //NeedSleep   = 1;
    //ServerLogin = 0;  // ���ߵ�ͬʱ�˳���½
   

    //CRC
    Crc = CRC16(TCP_SendBuf,12);
    TCP_SendBuf[12] = (BYTE)Crc;
    TCP_SendBuf[13] = (BYTE)(Crc>>8);

    TCP_SendBuf[14] = 0x16;  // ����

    TcpSendDataLen = 15;
    //if (SysParam[SP_MODTYPE] == CDMA_MC323)
    //{
    Send_Tcp_Data();
    //}
}



