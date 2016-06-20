/*************************************************************
�ɶ���ջ����������޹�˾   ��Ȩ����

�ļ���:  Sms.c
��  ��:  �˹���
��  ��:  ���Ŵ���ģ��
�޶���¼:   

**************************************************************/


#include "includes.h"
#include "driverlib/uart.h"
#include "Hw_uart.h"
#include "Comm.h"
#include "Cdma1x.h"
#include "Sms.h"
#include "Dev.h"
#include "Gsm.h"
#include "WlModule.h"
#include "main.h"
#include "Em485.h"
#include "WlModule.h"
#include "DLT645.h"
#include "devTcpip.h"
#include "devTimer.h"

extern BYTE DevVersion[];   // �豸�汾
extern const BYTE DevRelTime[];

//extern BYTE SysParam[];

extern BYTE Uart0_Buffer[];
extern WORD Uart0_Sav_Num;
extern BYTE NoCenter;
extern BYTE PowerStr[];
//extern DWORD Power_Int;  // ��������
//extern BYTE Power_Dec;   // С������
extern BYTE DeviceStatus;
//extern char Tmp0;

extern void sendUart1Term ( unsigned char *buff , int len );
extern void sendUart0Term ( unsigned char *buff , int len );

extern void ClearUart0Buf(void);

#ifdef USE_ADC

//extern BYTE T0_De;
//extern BYTE T0_Neg; // ���¶�
#else
BYTE T0_De  = 0;
BYTE T0_Neg = 0;
#endif
//extern BYTE EmId[];

extern BYTE IpAddrStr[];
extern BYTE PortStr[];

extern BYTE NeedConnSrv;

//BYTE Sms_Index = 0;   // ԭ���Ĵ���ʽ�����⣬���ܴ���λ����10�Ժ�Ķ���
BYTE Sms_IndexStr[3] = {0}; // ��ദ��999������

WORD Sms_buf_addr = 0;   // ��ǰָ��λ��
//BYTE SMS_index[23];   //SIM���ж��ŵ�����б�


BYTE SmsNumber[12];       //  �ظ����ŵ绰����
BYTE CenterNumber[12];   // �������ĺ���
BYTE SmsContent[SMSMSGCOUNT];       //  �����ı�����
BYTE SmsContent2[SMSMSGCOUNT*2];   //  PDU��������

BYTE SmsLength  = 0;      //  ���ų���
BYTE SmsLength2 = 0;      //  PDU���ų���

BYTE SmsTimeStr[21] = {0};


extern BYTE SysTime[];
    

BYTE NewMsgRec(void)
{
    if (SysParam[SP_MODTYPE] == CDMA_MC323)
    {
        return NewMsg_C();
    }
    else if (SysParam[SP_MODTYPE] == GSM_MG323)
    {
        return NewMsg_G();
    }
    else
    {
        return FALSE;
    }
        
}

void Clear_Sms_Content(void)
{
    static WORD i;
    for (i=0;i<SMSMSGCOUNT;i++)
    {
        SmsContent[i] = 0;
    }

    for (i=0;i<SMSMSGCOUNT*2;i++)
    {
        SmsContent2[i] = 0;
    }

}

#if 1
//���ı�����ת��ΪPDU��ʽ
BYTE ChangeContent(void)
{
    static WORD i,n;

    for (i=0;i<SMSMSGCOUNT*2;i++)
    {
        SmsContent2[i] = 0;
    }
    
    SmsLength2 = 0;
    // �̶��ֶ�
    mcpy(&SmsContent2[SmsLength2],"089168",6);
    SmsLength2 = 6;

    //���ĺ���
    mcpy(&SmsContent2[SmsLength2],CenterNumber,12);
    SmsLength2 += 12;
    
    mcpy(&SmsContent2[SmsLength2],"11000D9168",10);
    SmsLength2 += 10;
    
    //Ŀ�ĵ绰����
    //SmsNumber
    mcpy(&SmsContent2[SmsLength2],SmsNumber,12);
    SmsLength2 += 12;
    

    //�̶�
    mcpy(&SmsContent2[SmsLength2],"0000C2",6);   // 8 bit ���� ����Ч��5����
    SmsLength2 += 6;

    //���ݳ���
    SmsContent2[SmsLength2]   = HexToBcd(SmsLength / 16);
    SmsContent2[SmsLength2+1] = HexToBcd(SmsLength % 16);
    SmsLength2 += 2;

    n = Encode7bit(SmsContent, SmsLength);
    
    //��Ϣ���� -- BCD��
    for (i=0;i<n;i++)
    {
        SmsContent2[SmsLength2]   = HexToBcd(SmsContent[i] / 16);
        SmsContent2[SmsLength2+1] = HexToBcd(SmsContent[i] % 16);
        SmsLength2 += 2;
    }

    return n;
}
#else
//���ı�����ת��ΪPDU��ʽ
void ChangeContent(void)
{
    static BYTE i;

    for (i=0;i<SMSMSGCOUNT*2;i++)
    {
        SmsContent2[i] = 0;
    }
    
    SmsLength2 = 0;
    // �̶��ֶ�
    mcpy(&SmsContent2[SmsLength2],"0781",4);
    SmsLength2 += 4;

    //���ĺ���
    mcpy(&SmsContent2[SmsLength2],CenterNumber,12);
    SmsLength2 += 12;
    
    mcpy(&SmsContent2[SmsLength2],"11000B",6);
    SmsLength2 += 6;
    
    //Ŀ�ĵ绰����
    //SmsNumber
    mcpy(&SmsContent2[SmsLength2],SmsNumber,12);
    SmsLength2 += 12;
    

    //�̶�
    mcpy(&SmsContent2[SmsLength2],"0004C2",6);   // 8 bit ���� ����Ч��5����
    SmsLength2 += 6;

    //���ݳ���
    SmsContent2[SmsLength2]   = HexToBcd(SmsLength / 16);
    SmsContent2[SmsLength2+1] = HexToBcd(SmsLength % 16);
    SmsLength2 += 2;
    
    //��Ϣ���� -- BCD��
    for (i=0;i<SmsLength;i++)
    {
        SmsContent2[SmsLength2]   = HexToBcd(SmsContent[i] / 16);
        SmsContent2[SmsLength2+1] = HexToBcd(SmsContent[i] % 16);
        SmsLength2 += 2;
    }
}


#endif



void Send_String( BYTE *Word_ptr)
{
    //BYTE i=0;                // pointer to string to transmit
    static BYTE len;

    len = 0;
    //���㳤��
    while(Word_ptr[len] != 0)
    {
        len++;
    }

    Sms_SendStr(Word_ptr,len);
}




//����յ����������Ƿ�����Ҫ������
BYTE Check_string_rxed(BYTE *String_ptr)
{
    BYTE *data_ptr;
    BYTE Out_range = 0;

    Sms_buf_addr = 0;
search:    
    data_ptr = String_ptr;                                //ָ��ָ���ַ����׵�ַ
    while(Sms_buf_addr < Sms_BUFFER_LEN)                 //��0x1200��0x1300��Χ����Ѱ�ַ���
    {
        if(*data_ptr == Sms_Buffer[Sms_buf_addr])  
        {
            
            break;                                //����յ��ַ������ֽڣ����˳�ѭ��
        }
        Sms_buf_addr++;

        if(Sms_buf_addr == Sms_BUFFER_LEN)
        {
            return 0;                             //���δ�յ��ַ������ֽڣ��򷵻�ֵ0
        }
    }

    Sms_buf_addr++;

    if(Sms_buf_addr == Sms_BUFFER_LEN) 
    {
        Sms_buf_addr = 0;
        Out_range = 1;
    }
    
    data_ptr++;
    while(*data_ptr != '\0')                              //�������е�һ���������ַ������ֽڶԱ�
    {                                                       //������Ƚϵ�'\0'�ַ�����˵����ȫƥ�䣬�����ɹ�
        if(Sms_Buffer[Sms_buf_addr] != *data_ptr)             //�ַ���һ���뻺���������ݲ�ƥ�䣬���������
        {
            data_ptr = String_ptr;
            if(Out_range == 1) return 0;
        	goto search;
        }
        Sms_buf_addr++;
        if(Sms_buf_addr == Sms_BUFFER_LEN) 
        {
            Sms_buf_addr = 0;
            Out_range = 1;
        }
        data_ptr++;   
    }
    return 1;                                             //�����ɹ�������ֵ1
}


void RemoveFrontData(void)
{
    static WORD i,j;
    j = 0;
    for (i=Sms_buf_addr;i<Sms_BUFFER_LEN;i++)
    {
        Sms_Buffer[j++] =  Sms_Buffer[i];
    }

    for (i=j;i<Sms_BUFFER_LEN;i++)
    {
        Sms_Buffer[i] = 0;
    }

    Sms_sav_num -= Sms_buf_addr;
}


//����Э�鴦�� ----->
BYTE Check_Valid_SmsCmd(void)
{
    //Uart1_SendStr(Uart2_Buffer, Uart2_sav_num);
    return 0;
}


//��õ�n�����ŵ�λ��
WORD GetSignLoc(BYTE Sign,BYTE Index)
{
    static WORD i,j;

    j = 0;
    for (i=0;i<Sms_BUFFER_LEN;i++)
    {
        if (Sms_Buffer[i] == Sign)
        {
            j ++;
            if (j == Index)
            {
                return i;
            }
        }
    }
    return 0xFFFF;
}


void Get_Sms_Time(void)
{
    static BYTE i;
    i = GetSignLoc(',',4);
    if (i == 0xFF)
    {
        return;
    }
    
    if (Sms_Buffer[i+1] == '"')
    {
        //��
        SysTime[0] = (Sms_Buffer[i+2]-0x30)*0x10 + (Sms_Buffer[i+3]-0x30);
        //��
        SysTime[2] = (Sms_Buffer[i+5]-0x30)*0x10 + (Sms_Buffer[i+6]-0x30);
        //��
        SysTime[3] = (Sms_Buffer[i+8]-0x30)*0x10 + (Sms_Buffer[i+9]-0x30);
        //ʱ
        SysTime[4] = (Sms_Buffer[i+11]-0x30)*0x10 + (Sms_Buffer[i+12]-0x30);
        //��
        SysTime[5] = (Sms_Buffer[i+14]-0x30)*0x10 + (Sms_Buffer[i+15]-0x30);
        //��
        SysTime[6] = (Sms_Buffer[i+17]-0x30)*0x10 + (Sms_Buffer[i+18]-0x30);

        mcpy(SmsTimeStr,&Sms_Buffer[i+2],20);
        SmsTimeStr[20] = 0;
    }
}




BYTE Decode7bit(BYTE* pSrc, BYTE* pDst, BYTE nSrcLength)
{    
    static BYTE nSrc;        // Դ�ַ����ļ���ֵ    
    static BYTE nDst;        // Ŀ����봮�ļ���ֵ    
    static BYTE nByte;       // ��ǰ���ڴ���������ֽڵ���ţ���Χ��0-6    
    static BYTE nLeft;    // ��һ�ֽڲ��������        

    // ����ֵ��ʼ��    
    nSrc = 0;    
    nDst = 0;        

    // �����ֽ���źͲ������ݳ�ʼ��    
    nByte = 0;    
    nLeft = 0;        

    // ��Դ����ÿ7���ֽڷ�Ϊһ�飬��ѹ����8���ֽ�    
    // ѭ���ô�����̣�ֱ��Դ���ݱ�������    
    // ������鲻��7�ֽڣ�Ҳ����ȷ����    
    while(nSrc<nSrcLength)    
    {        
        // ��Դ�ֽ��ұ߲��������������ӣ�ȥ�����λ���õ�һ��Ŀ������ֽ�        
        *pDst = ((*pSrc << nByte) | nLeft) & 0x7f;   // �����ֽ�ʣ�µ���߲��֣���Ϊ�������ݱ�������        
        nLeft = *pSrc >> (7-nByte);      // �޸�Ŀ�괮��ָ��ͼ���ֵ        
        pDst++;        
        nDst++;            // �޸��ֽڼ���ֵ        
        nByte++;            // ����һ������һ���ֽ�        
        if(nByte == 7)        
        {            

            // ����õ�һ��Ŀ������ֽ�            
            *pDst = nLeft;                // �޸�Ŀ�괮��ָ��ͼ���ֵ            
            pDst++;            
            nDst++;                // �����ֽ���źͲ������ݳ�ʼ��            
            nByte = 0;            
            nLeft = 0;        
        }            

        // �޸�Դ����ָ��ͼ���ֵ        
        pSrc++;        
        nSrc++;    
    }        
    *pDst = 0;        

    // ����Ŀ�괮����    
    return nDst;

}


BYTE Encode7bit(BYTE *Src, BYTE Len)
{
    static BYTE i,j,tmp,n;

    n = Len;
    for (i=0;i<n-1;i++)
    {
        tmp = Src[i+1] << (7 - (i%7));
        Src[i] |= tmp;
        Src[i+1] >>= ((i%7) + 1);

        
        if (((i+1)%7) == 0)
        {
            //����һ���ֽڣ��Ѻ����������ǰ��
            for(j=i+1;j<n-1;j++)
            {
                Src[j] = Src[j+1];
            }

            Src[j+1] = 0; // ���һ���ֽ�����
            n--;
        }
    }

    return n;
}

/*
AT+CMGL=4

+CMGL: 1,0,,24
0891683108200845F7240D91683183185600F2210811509131918123044F60597D
0891683108200845F8240D91683183185600F221001150917145542302C618

OK
*/
BYTE Handle_PduSms(void)
{
    static BYTE i,k,len;
    i = GetSignLoc(0x0A,2);
    if (i == 0xFF)
    {
        return FALSE;
    }
    
    if ((Sms_Buffer[i+1] != '0') || (Sms_Buffer[i+2] != '8'))
    {
        printf("--Err1\r\n");
        return FALSE;
    }

    //�������ĺ���
    mcpy(CenterNumber,&Sms_Buffer[i+7],12);
    
    
    //27 �������
    mcpy(SmsNumber,&Sms_Buffer[i+27],12);
    
    //41 
    if ((Sms_Buffer[i+41] != '0') || (Sms_Buffer[i+42] != '0'))  
    {
        printf("--Err2\r\n");
       
        return FALSE;  // ���ı����ŷ���
    }

    //43 ʱ��
    //��
    SysTime[0] = (Sms_Buffer[i+44]-0x30)*16 + (Sms_Buffer[i+43]-0x30);
    //��
    SysTime[2] = (Sms_Buffer[i+46]-0x30)*16 + (Sms_Buffer[i+45]-0x30);
    //��
    SysTime[3] = (Sms_Buffer[i+48]-0x30)*16 + (Sms_Buffer[i+47]-0x30);
    //ʱ
    SysTime[4] = (Sms_Buffer[i+50]-0x30)*16 + (Sms_Buffer[i+49]-0x30);
    //��
    SysTime[5] = (Sms_Buffer[i+52]-0x30)*16 + (Sms_Buffer[i+51]-0x30);
    //��
    SysTime[6] = (Sms_Buffer[i+54]-0x30)*16 + (Sms_Buffer[i+53]-0x30);

    // 57 len
    len = BcdToHex(Sms_Buffer[i+57],Sms_Buffer[i+58]);

    //59 ����
    for (k=0;k<len;k++)
    {
        SmsContent2[k] = BcdToHex(Sms_Buffer[i+59+k*2],Sms_Buffer[i+60+k*2]);
    }

    SmsLength = Decode7bit(SmsContent2,SmsContent,len);
    SmsLength = len;    

    printf((char *)SmsContent);

//    SetSysTime();

    return TRUE;
    
}


//���ն���
BYTE ReceiveSMS(void)
{
    static BYTE ret = 0;
    Clear_Sms_Content();

    if (SysParam[SP_MODTYPE] == CDMA_MC323)
    {
        ret = Read_SMS();
        if (ret == TRUE)
        {
            if (!Get_Phone_Number())
            {
                Del_all_SMS();  
                Clear_Sms_buf();
                return FALSE;
            }
                
            
            if (!Get_Sms_Content())
            {
                Del_all_SMS();  
                Clear_Sms_buf();
                return FALSE;
            }
            
            Del_all_SMS();
            
            
            Clear_Sms_buf();
            return TRUE;
        }
        else
        {
            Del_all_SMS();    
            Clear_Sms_buf();
            return ret;
        }
    }
    else if (SysParam[SP_MODTYPE] == GSM_MG323)
    {

        ret = Handle_PduSms();
        if (ret == TRUE)
        {
            return ret;
        }
        else
        {
            Del_all_SMS();    
            Clear_Sms_buf();
            return ret;
        }
    }
    else
    {
        return FALSE;
    }
}



//����Э�鴦��
BYTE CheckParamF0(void)
{
    static BYTE i;

    if (SmsLength < 22)
    {
        return FALSE;
    }
    
    for (i=3;i<14;i++)
    {
        if ((SmsContent[i] < '0') || (SmsContent[i] > '9'))
        {
            return FALSE;
        }
    }

    for (i=15;i<19;i++)
    {
        if ((SmsContent[i] < '0') || (SmsContent[i] > '9'))
        {
            return FALSE;
        }
    }

    for (i=20;i<22;i++)
    {
        if ((SmsContent[i] < '0') || (SmsContent[i] > '9'))
        {
            return FALSE;
        }
    }

    return TRUE;
}


void SysHelp(void)  // ?
{
    static WORD year;
    Clear_Sms_Content();
    mcpy(SmsContent, "GSM COL Vx.x\r\nCopyright By HOPEP\r\n20xx-xx-xx\r\n",46);
    SmsContent[9]  = DevVersion[2];
    SmsContent[11] = DevVersion[4];

    year = (WORD)DevRelTime[6];
    year = (year << 8) + DevRelTime[5];
    SmsContent[36] = (year%100)/10+0x30;  // ��
    SmsContent[37] = (year%100)%10+0x30;
    
    SmsContent[39] = DevRelTime[4]/10+0x30;  // ��
    SmsContent[40] = DevRelTime[4]%10+0x30;

    SmsContent[42] = DevRelTime[3]/10+0x30;  // ��
    SmsContent[43] = DevRelTime[3]%10+0x30;
    
    SmsLength = 46;
    if (SysParam[SP_MODTYPE] == GSM_MG323)
    {
        Send_Pdu_SMS();
    }
    else if (SysParam[SP_MODTYPE] == CDMA_MC323)
    {
        Send_Txt_SMS(SmsNumber, SmsContent);
    }
}



WORD GetRepDay(void)
{
    static WORD ret;
    ret = (SmsContent[15] - 0x30)*1000 + (SmsContent[16] - 0x30)*100 + 
          (SmsContent[17] - 0x30)*10 +    (SmsContent[18] - 0x30);
    return ret;
}

BYTE GetRepHour(void)
{
    static BYTE ret;
    ret = (SmsContent[20] - 0x30)*10 +  (SmsContent[21] - 0x30);
    return ret;
}


void SetNextPepTime(void)
{
    static WORD Day;

//    GetSysTime();
    
    Day = DateToDay();
    Day += *(WORD *)&SysParam[SP_DAYDELTA];

    *(WORD *)&SysParam[SP_NEXTREPDAY] = Day;

}


void SetSysParam(void)  //F0
{
    static WORD Day;
    static BYTE Hour;

    if (SmsLength == 2)
    {
        ReportSysParam();
        return;
    }
    
    if (CheckParamF0())
    {
        //  �㱨�绰����
        mcpy((BYTE*)&SysParam[SP_MOBLENUM],&SmsContent[3],11);
        SysParam[SP_MOBLENUM+11] = 0;
        
        Day = GetRepDay();
        Hour = GetRepHour();

        //  ʱ����
        *(WORD *)&SysParam[SP_DAYDELTA] = Day;
        SysParam[SP_REPTIME] = Hour;

        SetNextPepTime();

        SaveSysParam();

        NoCenter = 0;

        mcpy(&SmsContent[SmsLength],"(Ok)\r\n",6);
        SmsLength += 6;
        if (SysParam[SP_MODTYPE] == GSM_MG323)
        {
            Send_Pdu_SMS();
        }
        else if (SysParam[SP_MODTYPE] == CDMA_MC323)
        {
            Send_Txt_SMS(SmsNumber, SmsContent);
        }
    }
    else
    {
        Clear_Sms_Content();
        mcpy(SmsContent, "F0:Param Error! e.g. F0:13988888888,1234-08",43);
        SmsLength = 43;
        if (SysParam[SP_MODTYPE] == GSM_MG323)
        {
            Send_Pdu_SMS();
        }
        else if (SysParam[SP_MODTYPE] == CDMA_MC323)
        {
            Send_Txt_SMS(SmsNumber, SmsContent);
        }
    }
    
}


BYTE GetPowerString(BYTE *pBuf,EMeterData_t *pe)
{
    static BYTE t1;
    static DWORD t2;

    for (t1=0;t1<10;t1++)
    {
        pBuf[t1] = 0;
    }
    //printf("pe->=%d\r\n",pe->EE);
    t1 = (BYTE)(pe->EE/10000000);
    t2 = pe->EE%10000000;
    if (t1 != 0)   // ʮ��
    {
        pBuf[0] = t1+0x30;

        t1 = (BYTE)(t2/1000000);
        t2 = t2 % 1000000;
        pBuf[1] = t1+0x30;

        t1 = (BYTE)(t2/100000);
        t2 = t2 % 100000;
        pBuf[2] = t1+0x30;

        t1 = (BYTE)(t2/10000);
        t2 = t2 % 10000;
        pBuf[3] = t1+0x30;   

        t1 = (BYTE)(t2/1000);
        t2 = t2 % 1000;
        pBuf[4] = t1+0x30;
        
        t1 = (BYTE)(t2/100);
        t2 = t2 % 100;
        pBuf[5]=t1+0x30;
        
        pBuf[6] = '.';
        t1 = (BYTE)(t2/10);
        t2 = t2 % 10;
        pBuf[7] = t1+0x30;
        pBuf[8] = t2+0x30;
        
        return 9;
    }
    
    
    t1 = (BYTE)(pe->EE/1000000);
    t2 = pe->EE%1000000;
    if (t1 != 0)  // ��
    {
        pBuf[0] = t1+0x30;

        t1 = (BYTE)(t2/100000);
        t2 = t2 % 100000;
        pBuf[1] = t1+0x30;

        t1 = (BYTE)(t2/10000);
        t2 = t2 % 10000;
        pBuf[2] = t1+0x30;

        t1 = (BYTE)(t2/1000);
        t2 = t2 % 1000;
        pBuf[3] = t1+0x30; 

        t1 = (BYTE)(t2/100);
        t2 = t2 % 100;
        pBuf[4] = t1+0x30;
       
        pBuf[5] = '.';
        t1 = (BYTE)(t2/10);
        t2 = t2 % 10;
        pBuf[6] = t1+0x30;
        pBuf[7] = t2+0x30;

        return 8;
    }


    
    t1 = (BYTE)(pe->EE/100000);
    t2 = pe->EE%100000;
    if (t1 != 0)  // ǧ
    {
        pBuf[0] = t1+0x30;

        t1 = (BYTE)(t2/10000);
        t2 = t2 % 10000;
        pBuf[1] = t1+0x30;

        t1 = (BYTE)(t2/1000);
        t2 = t2 % 1000;
        pBuf[2] = t1+0x30; 

        t1 = (BYTE)(t2/100);
        t2 = t2 % 100;
        pBuf[3] = t1+0x30;
        
        pBuf[3] = t2+0x30;
        
        pBuf[4] = '.';
        t1 = (BYTE)(t2/10);
        t2 = t2 % 10;
        pBuf[5] = t1+0x30;
        pBuf[6] = t2+0x30;
       
        return 7;
    }

        
    t1 = (BYTE)(pe->EE/10000);
    t2 = pe->EE%10000;
    if (t1 != 0)  // ��
    {
        pBuf[0] = t1+0x30;

        t1 = (BYTE)(t2/1000);
        t2 = t2 % 1000;
        pBuf[1] = t1+0x30; 

        t1 = (BYTE)(t2/100);
        t2 = t2 % 100;
        pBuf[2] = t1+0x30; 
        
        pBuf[3] = '.';
        t1 = (BYTE)(t2/10);
        t2 = t2 % 10;
        pBuf[4] = t1+0x30;
        pBuf[5] = t2+0x30;
        
        return 6;
    }

            
    t1 = (BYTE)(pe->EE/1000);
    t2 = pe->EE%1000;
    if (t1 != 0)  // ʮ
    {
        pBuf[0] = t1+0x30; 

        t1 = (BYTE)(t2/100);
        t2 = t2 % 100;
        pBuf[1] = t1+0x30;
        
        pBuf[2] = '.';
        t1 = (BYTE)(t2/10);
        t2 = t2 % 10;
        pBuf[3] = t1+0x30;
        pBuf[4] = t2+0x30;
      
        return 5;
    }
    else    // ��λ
    {
        t1 = (BYTE)(pe->EE/100);
        t2 = pe->EE%100;
        pBuf[0] = t1+0x30;
        
        pBuf[1] = '.';
        t1 = (BYTE)(t2/10);
        t2 = t2 % 10;
        pBuf[2] = t1+0x30;
        pBuf[3] = t2+0x30;
        
        return 4;
    }
    
}



BYTE ReportCurentPower(void)   // F1
{
    static BYTE len[3];
    //static DWORD t2;

    Clear_Sms_Content();

    SmsContent[0] = 'F';
    SmsContent[1] = '1';
    SmsContent[2] = ':';
    SmsContent[3] = '\r';
    SmsContent[4] = '\n';
    len[0]= GetPowerString(PowerStr,&EMeterDat[0]);
   
    len[1] = GetPowerString(PowerStr+10,&EMeterDat[1]);
 
    len[2] = GetPowerString(PowerStr+20,&EMeterDat[2]);
 
    SmsLength = 18+len[0]+len[1]+len[2];
 
    
    if (SmsLength >= SMSMSGCOUNT)
    {
        return FALSE;
    }
    SmsContent[5] = 'E';
    SmsContent[6] = '1';
    SmsContent[7] = ':';
    mcpy(&SmsContent[8],PowerStr,len[0]);
    SmsContent[8+len[0]] = '\r';
    SmsContent[9+len[0]] = '\n';
 
    SmsContent[10+len[0]]='E';
    SmsContent[11+len[0]]='2';
    SmsContent[12+len[0]]=':';
    mcpy(&SmsContent[13+len[0]],(PowerStr+10),len[1]);
    SmsContent[13+len[0]+len[1]]='\r';
    SmsContent[14+len[0]+len[1]]='\n';

    SmsContent[15+len[0]+len[1]]='E';
    SmsContent[16+len[0]+len[1]]='3';
    SmsContent[17+len[0]+len[1]]=':';
    mcpy(&SmsContent[18+len[0]+len[1]],(PowerStr+20),len[2]);
 
    OSTimeDlyHMSM(0,0,0,100);
    
    if (SysParam[SP_MODTYPE] == GSM_MG323)
    {
        return Send_Pdu_SMS();
    }
    else if (SysParam[SP_MODTYPE] == CDMA_MC323)
    {
        return Send_Txt_SMS(SmsNumber, SmsContent);
    }

    return FALSE;
    
}


void ControlDevice(void)  // F2
{
    if (SmsContent[3] == '0')
    {
        //Relay = 0;
        DeviceStatus = 0;
        Clear_Sms_Content();
        mcpy(&SmsContent[0],"F2:0(Ok)\r\n",10);
        SmsLength = 10;
    }
    else if (SmsContent[3] == '1')
    {
        //Relay = 1;
        DeviceStatus = 1;
        Clear_Sms_Content();
        mcpy(&SmsContent[0],"F2:1(Ok)\r\n",10);
        SmsLength = 10;
    }
    else
    {
        Clear_Sms_Content();
        mcpy(&SmsContent[0],"F2:Param Error. e.g. F2:0\r\n",27);
        SmsLength = 27;
    }
    
    
    if (SysParam[SP_MODTYPE] == GSM_MG323)
    {
        Send_Pdu_SMS();
    }
    else if (SysParam[SP_MODTYPE] == CDMA_MC323)
    {
        Send_Txt_SMS(SmsNumber, SmsContent);
    }
}

void RepStatus(void)  //F3
{
    static BYTE Tmp,Hum;
    //static char Tmp;
    GetSysTime();

    

    
    //Hum = GetHum();
    Hum = 60;
    if (Hum > 99)
    {
        Hum = 99;
    }
    
    //Sw = 0;
    Clear_Sms_Content();

    SmsContent[0] = 'F';
    SmsContent[1] = '3';
    SmsContent[2] = ':';

    SmsContent[3] = SysTime[0]/0x10+0x30;
    SmsContent[4] = SysTime[0]%0x10+0x30;
    SmsContent[5] = '/';

    SmsContent[6] = SysTime[1]/0x10+0x30;
    SmsContent[7] = SysTime[1]%0x10+0x30;
    SmsContent[8] = '/';

    SmsContent[9] =  SysTime[2]/0x10+0x30;
    SmsContent[10] = SysTime[2]%0x10+0x30;
    SmsContent[11] = ',';
    
    SmsContent[12] = SysTime[3]/0x10+0x30;
    SmsContent[13] = SysTime[3]%0x10+0x30;
    SmsContent[14] = ':';

    SmsContent[15] = SysTime[4]/0x10+0x30;
    SmsContent[16] = SysTime[4]%0x10+0x30;
    SmsContent[17] = ':';

    SmsContent[18] = SysTime[5]/0x10+0x30;
    SmsContent[19] = SysTime[5]%0x10+0x30;
    SmsContent[20] = ';';

    if (1)//(GetTmp())
    {
        T0_Neg = 0; 
        if (T0_Neg == 1)
        {
            //Tmp = 255 - (BYTE)Tmp0 + 1;
            SmsContent[21] = '-';
            SmsContent[22] = Tmp/10+0x30;
            SmsContent[23] = Tmp%10+0x30;
            SmsContent[24] = '.';
            SmsContent[25] = T0_De%10+0x30;
        }
        else
        {
            //Tmp = (BYTE)Tmp0;
            SmsContent[21] = ' ';
            //SmsContent[22] = Tmp/10+0x30;
            //SmsContent[23] = Tmp%10+0x30;
            SmsContent[22] = 2+0x30;
            SmsContent[23] = 5+0x30;
            SmsContent[24] = '.';
            SmsContent[25] = 5+0x30;
            //SmsContent[25] = T0_De%10+0x30;
        }
    }
    else
    {  // 99.9
        SmsContent[21] = ' ';
        SmsContent[22] = 9+0x30;
        SmsContent[23] = 9+0x30;
        SmsContent[24] = '.';
        SmsContent[25] = 9+0x30;
    }
    
    SmsContent[26] = 'C';
    SmsContent[27] = ';';
    
    SmsContent[28] = Hum/10+0x30;
    SmsContent[29] = Hum%10+0x30;
    SmsContent[30] = '%';
    SmsContent[31] = ';';

    //SmsContent[32] = ExDevWorking() + 0x30;            // �豸״̬
    SmsContent[32] = 1 + 0x30;            // �豸״̬
    
    SmsContent[33] = ';';
    SmsContent[34] = SysParam[SP_LASTERROR] + 0x30;   // ������
    SmsLength = 35;
    if (SysParam[SP_MODTYPE] == GSM_MG323)
    {
        Send_Pdu_SMS();
    }
    else if (SysParam[SP_MODTYPE] == CDMA_MC323)
    {
        Send_Txt_SMS(SmsNumber, SmsContent);
    }
}

void ReportSysParam(void)   
{
    static WORD day;
    
    Clear_Sms_Content();
    SmsContent[0] = 'F';
    SmsContent[1] = '0';
    SmsContent[2] = ':';
    mcpy((BYTE*)&SmsContent[3],(BYTE*)&SysParam[SP_MOBLENUM],11);
    SmsContent[14] = ';';

    day = *(WORD *)&SysParam[SP_DAYDELTA];
    SmsContent[15] = (BYTE)(day/1000 + 0x30);
    SmsContent[16] = (BYTE)((day%1000)/100+0x30);
    SmsContent[17] = (BYTE)(((day%1000)&100)/10+0x30);
    SmsContent[18] = (BYTE)(day%10 + 0x30);

    SmsContent[19] = '-';

    SmsContent[20] = SysParam[SP_REPTIME]/10 + 0x30;
    SmsContent[21] = SysParam[SP_REPTIME]%10 + 0x30;

    SmsLength = 22;
    
    if (SysParam[SP_MODTYPE] == GSM_MG323)
    {
        Send_Pdu_SMS();
    }
    else if (SysParam[SP_MODTYPE] == CDMA_MC323)
    {
        Send_Txt_SMS(SmsNumber, SmsContent);
    }
}


void SetDeviceParam(void)  //F5:0,1
{
    if (SmsLength < 4)
    {
        Clear_Sms_Content();
        mcpy(&SmsContent[0],"F5:Param Error. e.g. F5:0[,1]\r\n",31);
        SmsLength = 31;
        if (SysParam[SP_MODTYPE] == GSM_MG323)
        {
            Send_Pdu_SMS();
        }
        else if (SysParam[SP_MODTYPE] == CDMA_MC323)
        {
            Send_Txt_SMS(SmsNumber, SmsContent);
        }
        return;
    }
    
    if (SmsLength > 5)
    {
        if ((SmsContent[5] != '0') && (SmsContent[5] != '1'))
        {
            Clear_Sms_Content();
            mcpy(&SmsContent[0],"F5:Param Error. e.g. F5:0[,1]\r\n",31);
            SmsLength = 31;
            if (SysParam[SP_MODTYPE] == GSM_MG323)
            {
                Send_Pdu_SMS();
            }
            else if (SysParam[SP_MODTYPE] == CDMA_MC323)
            {
                Send_Txt_SMS(SmsNumber, SmsContent);
            }
            return;
        }
        else
        {
            SysParam[SP_DEVICETAP] = SmsContent[5] - 0x30;
        }
    }
    else
    {
        SysParam[SP_DEVICETAP] = 0;
    }

    if ((SmsContent[3] != '0') && (SmsContent[3] != '1'))
    {
        Clear_Sms_Content();
        mcpy(&SmsContent[0],"F5:Param Error. e.g. F5:0[,1]\r\n",31);
        SmsLength = 31;
        if (SysParam[SP_MODTYPE] == GSM_MG323)
        {
            Send_Pdu_SMS();
        }
        else if (SysParam[SP_MODTYPE] == CDMA_MC323)
        {
            Send_Txt_SMS(SmsNumber, SmsContent);
        }
        return;
    }
    else
    {
        SysParam[SP_DEVICECTL] = SmsContent[3] - 0x30;
    }

    SaveSysParam();


    mcpy(&SmsContent[SmsLength],"(Ok)\r\n",6);
    SmsLength += 6;
    if (SysParam[SP_MODTYPE] == GSM_MG323)
    {
        Send_Pdu_SMS();
    }
    else if (SysParam[SP_MODTYPE] == CDMA_MC323)
    {
        Send_Txt_SMS(SmsNumber, SmsContent);
    }
    
}

// F6 ���ô��ڲ���: ������,У��(ֻ֧��8λ����λ��1λֹͣλ����������)
// 150��300��600��1200��2400��4800��9600��19200��38400��57600�� 115200
// E(żУ��),O(��У��)
// ��9600E(������9600��żУ��); 1200(������1200����У��)
void SetUartParam(void)
{
    static BYTE ChkBit,BaudRt,BaudLen;
    static BYTE NeedReboot;
    
    if (SmsContent[SmsLength-1] != '0')
    {
        if (SmsContent[SmsLength-1] == 'E')
        {
            ChkBit = CHECK_EVEN;
        }
        else if (SmsContent[SmsLength-1] == 'O')
        {
            ChkBit = CHECK_ODD;
        }
        else
        {
            ChkBit = 0xFF;
        }
        BaudLen = SmsLength - 4;
    }
    else
    {
        ChkBit = CHECK_NONE;
        BaudLen = SmsLength - 3;
    }

    if (BaudLen == 3)
    {
        if (mcmp(&SmsContent[3], "150", BaudLen))
        {
            BaudRt = BAUDRATE_150;
        }
        else if (mcmp(&SmsContent[3], "300", BaudLen))
        {
            BaudRt = BAUDRATE_300;
        }
        else if (mcmp(&SmsContent[3], "600", BaudLen))
        {
            BaudRt = BAUDRATE_600;
        }
        else 
        {
            BaudRt = 0xFF;
        }
    }
    else if (BaudLen == 4)
    {
        if (mcmp(&SmsContent[3], "1200", BaudLen))
        {
            BaudRt = BAUDRATE_1200;
        }
        else if (mcmp(&SmsContent[3], "2400", BaudLen))
        {
            BaudRt = BAUDRATE_2400;
        }
        else if (mcmp(&SmsContent[3], "4800", BaudLen))
        {
            BaudRt = BAUDRATE_4800;
        }
        else if (mcmp(&SmsContent[3], "9600", BaudLen))
        {
            BaudRt = BAUDRATE_9600;
        }
        else 
        {
            BaudRt = 0xFF;
        }
    }
    else if (BaudLen == 5)
    {
        if (mcmp(&SmsContent[3], "19200", BaudLen))
        {
            BaudRt = BAUDRATE_19200;
        }
        else if (mcmp(&SmsContent[3], "38400", BaudLen))
        {
            BaudRt = BAUDRATE_38400;
        }
        else if (mcmp(&SmsContent[3], "57600", BaudLen))
        {
            BaudRt = BAUDRATE_57600;
        }
        else 
        {
            BaudRt = 0xFF;
        }
    }
    else if (BaudLen == 6)
    {
        if (mcmp(&SmsContent[3], "115200", BaudLen))
        {
            BaudRt = BAUDRATE_115200;
        }
        else 
        {
            BaudRt = 0xFF;
        }
    }
    else
    {
        BaudRt = 0xFF;
    }

    if ((BaudRt != 0xFF) && (ChkBit != 0xFF))
    {
        SysParam[SP_CHECKBIT] = ChkBit;
        SysParam[SP_BAUDRATE] = BaudRt;
        SaveSysParam();

        mcpy(&SmsContent[SmsLength],"(Ok)\r\n",6);
        SmsLength += 6;

        NeedReboot = 1;
    }
    else
    {
        mcpy(&SmsContent[SmsLength],"(Fail)\r\n",8);
        SmsLength += 8;

        NeedReboot = 0;
    }

    if (SysParam[SP_MODTYPE] == GSM_MG323)
    {
        Send_Pdu_SMS();
    }
    else if (SysParam[SP_MODTYPE] == CDMA_MC323)
    {
        Send_Txt_SMS(SmsNumber, SmsContent);
    }


    Sleep(2000);
    //WDog();
    Sleep(3000);
    //WDog();

    if (NeedReboot)
    {
        SysReset();
    }
}

// F6 ����������
void SetHostName(void)
{
    static BYTE j; 
    if (SmsLength-3 < 32)
    {
        for (j=SP_HOSTNAME;j<SP_HOSTNAME+32;j++)
        {
            SysParam[j] = 0;
        }

        j = 0;
        for (j=0;j<SmsLength-3;j++)
        {            
            SysParam[j+SP_HOSTNAME] = SmsContent[j+3];           
        }
        
        //mcpy(&SysParam[SP_HOSTNAME],&SmsContent[3],SmsLength-3);
        SaveSysParam();

        mcpy(&SmsContent[SmsLength],"(Ok)\r\n",6);
        SmsLength += 6;
    }
    else
    {
        mcpy(&SmsContent[SmsLength],"(Fail)\r\n",8);
        SmsLength += 8;
    }
    
    if (SysParam[SP_MODTYPE] == GSM_MG323)
    {
        Send_Pdu_SMS();
    }
    else if (SysParam[SP_MODTYPE] == CDMA_MC323)
    {
        Send_Txt_SMS(SmsNumber, SmsContent);
    }
}



//F7: ϵͳ����-- ϵͳ��λ,����/�ر�HTTP, �ָ�Ĭ�ϲ�����SOCKETʹ��[��ѡ],IP��ַģʽ
// CDMA��ʱ��֧��HTTP��ʽ
void SysConfig(void)  // F7:0[,0,0,0,0]
{    
    static BYTE NeedReboot;

    // ����һ������
    // check Param
    if (SmsLength < 4)
    {
        goto Error;
    }

    if ((SmsContent[3] != '0') && (SmsContent[3] != '1'))
    {
        goto Error;
    }

    if (SmsLength >= 6)
    {
        if ((SmsContent[5] != '0') && (SmsContent[5] != '1'))
        {
            goto Error;
        }
    }

    if (SmsLength >= 8)
    {
        if ((SmsContent[7] != '0') && (SmsContent[7] != '1'))
        {
            goto Error;
        }
    }

    if (SmsLength >= 10)
    {
        if ((SmsContent[9] != '0') && (SmsContent[9] != '1'))
        {
            goto Error;
        }
    }

    if (SmsLength >= 12)
    {
        if ((SmsContent[11] != '1') && (SmsContent[11] != '2')  && (SmsContent[11] != '3'))
        {
            goto Error;
        }
    }

    NeedReboot = 0;
    

    //�ָ�Ĭ�ϲ���
    if (SmsLength >= 8)
    {
        if (SmsContent[7] == '1')
        {
            setParamToDefault();
            NeedReboot = 1;
        }
    }

    //HTTP mode  -- �ò�������Ҫ����
    if (SmsLength >= 6)
    {
        if (SysParam[SP_ENABLEHTTP] != (SmsContent[5] - 0x30))
        {
            NeedReboot = 1;
        }
        
        if (SmsContent[5] == '0')
        {
            SysParam[SP_ENABLEHTTP] = 0;
        }
        else if (SmsContent[5] == '1')
        {
            SysParam[SP_ENABLEHTTP] = 1;
        }
    }
    
    
    //�Ƿ������Ͼ�����ģʽ Socket Mode  -- �ò�������Ҫ����
    if (SmsLength >= 10)
    {
        if (SysParam[SP_ENABLESOCKET] != (SmsContent[9] - 0x30))
        {
            NeedReboot = 1;
        }
        
        if (SmsContent[9] == '1')
        {
            SysParam[SP_ENABLESOCKET] = 1;
        }
        else if (SmsContent[9] == '0')
        {
            SysParam[SP_ENABLESOCKET] = 0;
        }
    }

    //IP��ַģʽ -- �ò�������Ҫ����
    if (SmsLength >= 12)
    {
        if (SysParam[SP_DEVIPMODE] != (SmsContent[11] - 0x30))
        {
            NeedReboot = 1;
        }
        
        if (SmsContent[11] == '1')
        {
            SysParam[SP_DEVIPMODE] = IPMODE_FIXED;
        }
        else if (SmsContent[11] == '2')
        {
            SysParam[SP_DEVIPMODE] = IPMODE_DHCP;
        }
        else if (SmsContent[11] == '3')
        {
            SysParam[SP_DEVIPMODE] = IPMODE_PPPOE;
        }
    }
    
    SaveSysParam();

    if (SmsContent[3] == '1')
    {
        NeedReboot = 1;
    }
    

    mcpy(&SmsContent[SmsLength],"(Ok)\r\n",6);
    SmsLength += 6;
    
    if (SysParam[SP_MODTYPE] == GSM_MG323)
    {
        Send_Pdu_SMS();
    }
    else if (SysParam[SP_MODTYPE] == CDMA_MC323)
    {
        Send_Txt_SMS(SmsNumber, SmsContent);
    }

    Sleep(2000);
    //WDog();
    Sleep(3000);
    //WDog();
    
    //ϵͳ��λ
    if (NeedReboot == 1)
    {
        printf("Sys Resetting...\r\n");
        SysReset();
    }

    return;

Error:
    Clear_Sms_Content();
    mcpy(&SmsContent[0],"F7:Param Error. e.g. F7:0,0,0\r\n",31);
    SmsLength = 31;
    if (SysParam[SP_MODTYPE] == GSM_MG323)
    {
        Send_Pdu_SMS();
    }
    else if (SysParam[SP_MODTYPE] == CDMA_MC323)
    {
        Send_Txt_SMS(SmsNumber, SmsContent);
    }
}
#if 0
//F8:���ص��ID
void GetEmId(void)
{
    static BYTE  i;
    
    Clear_Sms_Content();
    
    SmsContent[0] = 'F';
    SmsContent[1] = '8';
    SmsContent[2] = ':';

    for (i=0;i<6;i++)
    {
        SmsContent[3+i*2]   = HexToBcd(EmId[5-i]/0x10);
        SmsContent[3+i*2+1] = HexToBcd(EmId[5-i]%0x10);
    }
    
    SmsLength = 15;
    if (SysParam[SP_MODTYPE] == GSM_MG323)
    {
        Send_Pdu_SMS();
    }
    else if (SysParam[SP_MODTYPE] == CDMA_MC323)
    {
        Send_Txt_SMS(SmsNumber, SmsContent);
    }
}
#endif
//F8:���ص��ID
void GetEmId(void)
{
    static BYTE  i;
    
    Clear_Sms_Content();
    
    SmsContent[0] = 'F';
    SmsContent[1] = '8';
    SmsContent[2] = ':';
    SmsContent[3] = 'N';
    SmsContent[4] = 'O';
    SmsContent[5] = '1';
    SmsContent[6] = '.';
    for (i=0;i<6;i++)
    {
        SmsContent[7+i*2]   = HexToBcd(SysParam[SP_METERADDR1+5-i]/0x10);
        SmsContent[7+i*2+1] = HexToBcd(SysParam[SP_METERADDR1+5-i]%0x10);
    }
    SmsContent[19] = '\r';
    SmsContent[20] = '\n';
    
    SmsContent[21] = 'N';
    SmsContent[22] = 'O';
    SmsContent[23] = '2';
    SmsContent[24] = '.';
    for (i=0;i<6;i++)
    {
        SmsContent[25+i*2]   = HexToBcd(SysParam[SP_METERADDR2+5-i]/0x10);
        SmsContent[25+i*2+1] = HexToBcd(SysParam[SP_METERADDR2+5-i]%0x10);
    }
    SmsContent[37] = '\r';
    SmsContent[38] = '\n';
    
    SmsContent[39] = 'N';
    SmsContent[40] = 'O';
    SmsContent[41] = '3';
    SmsContent[42] = '.';
    for (i=0;i<6;i++)
    {
        SmsContent[43+i*2]   = HexToBcd(SysParam[SP_METERADDR3+5-i]/0x10);
        SmsContent[43+i*2+1] = HexToBcd(SysParam[SP_METERADDR3+5-i]%0x10);
    }
    SmsLength = 54;
    if (SysParam[SP_MODTYPE] == GSM_MG323)
    {
        Send_Pdu_SMS();
    }
    else if (SysParam[SP_MODTYPE] == CDMA_MC323)
    {
        Send_Txt_SMS(SmsNumber, SmsContent);
    }
}


//F8: Socketģʽ��ȡIP��ַ
void GetCurIpAdd(void)
{
    static BYTE len,temp;
    
    IpToStr((BYTE*)&SysParam[SP_LOCALIP]);
    PortToStr((BYTE*)&SysParam[SP_LOCALPORT]);
    
    Clear_Sms_Content();

    SmsContent[0] = 'F';
    SmsContent[1] = '8';
    SmsContent[2] = ':';

    len = StrLen((char *)IpAddrStr);
    SmsLength = 3 + len;
    if (SmsLength >= SMSMSGCOUNT)
    {
        goto Fail_0;
    }

    mcpy(&SmsContent[3],IpAddrStr,len);
    

    SmsContent[SmsLength] = ';';
    SmsLength ++;

    len = StrLen((char *)PortStr);
    temp = SmsLength + len;
    if (temp >= SMSMSGCOUNT)
    {
        goto Fail_0;
    }

    mcpy(&SmsContent[SmsLength],PortStr,len);
    SmsLength += len;
    
    if (SysParam[SP_MODTYPE] == GSM_MG323)
    {
        Send_Pdu_SMS();
    }
    else if (SysParam[SP_MODTYPE] == CDMA_MC323)
    {
        Send_Txt_SMS(SmsNumber, SmsContent);
    }
    return;

Fail_0:
    mcpy(&SmsContent[3],"(Fail)\r\n",8);
    SmsLength += 8;
    if (SysParam[SP_MODTYPE] == GSM_MG323)
    {
        Send_Pdu_SMS();
    }
    else if (SysParam[SP_MODTYPE] == CDMA_MC323)
    {
        Send_Txt_SMS(SmsNumber, SmsContent);
    }
}


//F9:���÷�����IP��ַ�Ͷ˿�
void SetServerAddr(void)
{
    static BYTE i,loc;

    i = 0;
    while(i<SmsLength)
    {
        if (SmsContent[i] == ',')
        {
            break;
        }
        i++;
    }

    mset(IpAddrStr,0,16);
    mset(PortStr,0,6);
    
    mcpy(IpAddrStr,&SmsContent[3],i-3);             // Ip Address
   
    if (!CheckValidIpAddr())
    {
        goto Fail;
    }

    if (!StrToIpAdd((BYTE*)&SysParam[SP_SERVERIP]))
    {
        goto Fail;
    }

    loc = i;

    i++;
    while(i<SmsLength)
    {
        if (SmsContent[i] == ',')
        {
            break;
        }
        i++;
    }

    mcpy(PortStr,&SmsContent[loc+1],i-loc-1);  // Server Port
    if (!CheckValidPort())
    {
        goto Fail;
    }

    if (!StrToPort((BYTE*)&SysParam[SP_SERVERPORT]))
    {
        goto Fail;
    }
    
    loc = i;
    mcpy(PortStr,&SmsContent[loc+1],SmsLength-loc-1);  // Local Port
    if (!CheckValidPort())
    {
        goto Fail;
    }

    if (!StrToPort((BYTE*)&SysParam[SP_LOCALPORT]))
    {
        goto Fail;
    }

    SaveSysParam();
            
    mcpy(&SmsContent[SmsLength],"(Ok)\r\n",6);
    SmsLength += 6;
    if (SysParam[SP_MODTYPE] == GSM_MG323)
    {
        Send_Pdu_SMS();
    }
    else if (SysParam[SP_MODTYPE] == CDMA_MC323)
    {
        Send_Txt_SMS(SmsNumber, SmsContent);
    }

    SysReset();


Fail:
    mcpy(&SmsContent[SmsLength],"(Fail)\r\n",8);
    SmsLength += 8;
    if (SysParam[SP_MODTYPE] == GSM_MG323)
    {
        Send_Pdu_SMS();
    }
    else if (SysParam[SP_MODTYPE] == CDMA_MC323)
    {
        Send_Txt_SMS(SmsNumber, SmsContent);
    }
}



//���Ż��Ѳ�����ע������
void SmsWakeup(void)
{
    static BYTE i,loc;

    i = 0;
    while(i<SmsLength)
    {
        if (SmsContent[i] == ',')
        {
            break;
        }
        i++;
    }
    if (i>=SmsLength)
    {
        goto Fail;
    }

    mset(IpAddrStr,0,16);
    mset(PortStr,0,6);
    
    mcpy(IpAddrStr,&SmsContent[3],i-3);             // Server Ip Address
    if (!CheckValidIpAddr())
    {
        goto Fail;
    }

    if (!StrToIpAdd((BYTE*)&SysParam[SP_SERVERIP]))
    {
        goto Fail;
    }

    loc = i;

    mcpy(PortStr,&SmsContent[loc+1],SmsLength-loc-1);  // Server Port
    if (!CheckValidPort())
    {
        goto Fail;
    }

    if (!StrToPort((BYTE*)&SysParam[SP_SERVERPORT]))
    {
        goto Fail;
    }
    
    SaveSysParam();
            
    
    NeedConnSrv = 1;
    return;
    

Fail:
    NeedConnSrv = 0;
    mcpy(&SmsContent[SmsLength],"(Fail)\r\n",8);
    SmsLength += 8;
    if (SysParam[SP_MODTYPE] == GSM_MG323)
    {
        Send_Pdu_SMS();
    }
    else if (SysParam[SP_MODTYPE] == CDMA_MC323)
    {
        Send_Txt_SMS(SmsNumber, SmsContent);
    }
}



void SetUserInfo()
{
    static BYTE i,j; 
    if (SmsLength-3 < 65)
    {
        j = 0;
        while(SmsContent[j+3] != ';')
        {
            j++;
            if (j > SmsLength-3)
            {
                goto Fail2;   // û�з���";"�߷��ش���
            }
        }

        
        for (j=SP_NETUSERNAME;j<SP_NETUSERNAME+32;j++)
        {
            SysParam[j] = 0;
        }

        for (j=SP_NETPASSWORD;j<SP_NETPASSWORD+32;j++)
        {
            SysParam[j] = 0;
        }

        j = 0;
        while(SmsContent[j+3] != ';')
        {
            SysParam[j+SP_NETUSERNAME] = SmsContent[j+3];      
            j++;
            if (j > SmsLength-3)
            {
                goto Fail2;
            }
        }

        j++;
        i = 0;
        for (;j<SmsLength-3;j++)
        {
            SysParam[i+SP_NETPASSWORD] = SmsContent[j+3];
            i++;
        }
        
        
        SaveSysParam();

        if ((SmsLength + 6) > SMSMSGCOUNT)
        {
            SmsLength -= 6;
        }

        mcpy(&SmsContent[SmsLength],"(Ok)\r\n",6);
        SmsLength += 6;

        //Send_Pdu_SMS();
        //Send_Txt_SMS(SmsNumber, SmsContent);

        if (SysParam[SP_MODTYPE] == GSM_MG323)
        {
            Send_Pdu_SMS();
        }
        else if (SysParam[SP_MODTYPE] == CDMA_MC323)
        {
            Send_Txt_SMS(SmsNumber, SmsContent);
        }

        SysReset();
    }

Fail2:
    {
        if ((SmsLength + 8) > SMSMSGCOUNT)
        {
            SmsLength -= 8;
        }
        
        mcpy(&SmsContent[SmsLength],"(Fail)\r\n",8);
        SmsLength += 8;

        //Send_Pdu_SMS();
        //Send_Txt_SMS(SmsNumber, SmsContent);

        if (SysParam[SP_MODTYPE] == GSM_MG323)
        {
            Send_Pdu_SMS();
        }
        else if (SysParam[SP_MODTYPE] == CDMA_MC323)
        {
            Send_Txt_SMS(SmsNumber, SmsContent);
        }
    }
    
    
}


void DealSmsContent(void)
{
    static BYTE SmsCmd;

    if (SmsContent[0] == '?')
    {
        SysHelp();
        return;
    }

    if (SmsContent[0] != 'F')
    {
        return;
    }

    SmsCmd = SmsContent[1];

    switch (SmsCmd)
    {   
        
        case '0':   
            //���ò���
            //F0:13912345678,dddd-hh
            //SetSysParam();

            //���Ż���
            //F0:192.168.1.1,8000  // ������IP�Ͷ˿�
            SmsWakeup();
        break;
        #if 0
        //F1
        case '1':   //����ǰ����
            if (SysParam[SP_ENABLESOCKET] == 1)
            {    
                
                ReportCurentPower();
            }
            else  
            {
                //GetCurIpAdd();
                Clear_Sms_Content();
                mcpy(&SmsContent[0],"Socket mode don't support the CMD!\r\n",36);
                SmsLength = 36;
                if (SysParam[SP_MODTYPE] == GSM_MG323)
                {
                    Send_Pdu_SMS();
                }
                else if (SysParam[SP_MODTYPE] == CDMA_MC323)
                {
                    Send_Txt_SMS(SmsNumber, SmsContent);
                }
            }
        break;
        #endif
        //F2:0/1
        case '2':   //�����ⲿ�豸
         //   ControlDevice();
        break;

        //F3
        case '3':   //��ȡ��������״̬
            RepStatus();
        break;

        //F4
        case '4':   //���ô��ڲ���
            SetUartParam();
        break;

        #if 0
        //��������: ���ⲿ�豸����ʱ��ǰ1��Сʱȡ��
        //F5:1[,0/1]  �Ƿ���ݶ�������ʱ���������,�Ƿ���Ҫ͵��Ϊ��ѡ����
        case '5':  //͵�� ^_^
            SetDeviceParam();
        break;
        #endif
        
        case '6':
            //���÷���������
            //SetHostName();
            SetUserInfo();
        break;
        

        //ϵͳ���ÿ���:GPRS/�ָ�Ĭ�ϲ���/ϵͳ��λ
        case '7':
            SysConfig();
        break;

        //���ص��ID
        case '8':
            if (SysParam[SP_ENABLESOCKET] == 0)
            {
                GetEmId();
            }
            else
            {
                GetCurIpAdd();
                /*
                Clear_Sms_Content();
                mcpy(&SmsContent[0],"Socket mode don't support the CMD!\r\n",36);
                SmsLength = 36;
                Send_Txt_SMS(SmsNumber, SmsContent);
                */
            }
        break;

        //���÷�����IP��ַ�Ͷ˿ڼ����ض˿�
        //F9:192.168.1.1,5000,9000
        case '9':
            SetServerAddr();
        break;

        //��������������Ҳ�����أ���ֹ��������
    }

    Clear_Sms_Content();
}

// 5����������ź�����
void SmsMsgHandle(void)
{
    static BYTE ret;
    
        if (NewMsgRec())
        {
            DebugMsg();
            //Clear_Sms_buf();
       
            //���ն���
            ret = ReceiveSMS();
            
            if (ret == TRUE)
            {
                if (SmsContent[0] != 0)
                {
                    DealSmsContent();
                }
            }
            else if (ret == TIMEOUT)
            {
                // ģ���쳣����ģ����и�λ
                Del_all_SMS();
                DebugMsg();
                SetLastError(ERR_GSMFAIL); 
                SysReset();
                //NetModulState = InitState;           
            }
        }
}

//1Сʱ����
void Task_1h(void)
{
    static BYTE h=0;
    
    //����������
    if (!Check_Net(0))
    {
        SetLastError(ERR_GSMFAIL);
        SysReset();
    }

    //��̬IPʱ��һ��Сʱע��һ�����磬��ֹ�����޷�����
    if (SysParam[SP_DEVIPMODE] == IPMODE_FIXED)
    {
        Close_TcpIpLink();
    }

    /*
    //�����
    if (SysParam[SP_ENABLESOCKET] == 0)
    {
        ret = GetCurrentPower();
        if (!ret)
        {
            SetLastError(ERR_EMFAIL);
            SysReset();
        }
    }
    */

    

    //24СʱУʱһ��ϵͳʱ��
    h ++;
    if (h >= 24)
    {
        h    = 0;
        
        if (Get_Modul_Time())
        {
            SetSysTime();
        }
        else
        {
            SetLastError(ERR_RTCFAIL);
        }
    }   
}

// <-----------


