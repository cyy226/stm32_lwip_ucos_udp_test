/*************************************************************
�ɶ���ջ����������޹�˾   ��Ȩ����
����ˮ���·�ϵͳ


�ļ���:
��  ��:    �˹���
��  ��:    ʵʱ�� HT1381
�޶���¼: bn

**************************************************************/
#include "includes.h"
#include "Comm.h"

extern BYTE SysTime[7];

void TimerDly(void)
{
   volatile  int i;
    for(i=0;i<20;i++)
        ;
}

/*******************************************************************************
*������:
����()
*����: ��ʱ��
*����:
*���:
*˵��:
*******************************************************************************/
static void timerSda ( int bHigh )
{
   
    if ( !bHigh )
    {
        signalOut ( GPIO_PORTB_BASE , GPIO_PIN_3,  bHigh );
    }
    else
    {
        signalOut ( GPIO_PORTB_BASE , GPIO_PIN_3 ,GPIO_PIN_3);
    }
  
}

static void timerScl ( int bHigh )
{ 
    GPIOPinWrite ( GPIO_PORTB_BASE,  GPIO_PIN_2 , bHigh? GPIO_PIN_2:0);
}

static int timerGetSda ( void )
{
    int tmp;
    
    tmp = signalIn ( GPIO_PORTB_BASE , GPIO_PIN_3 ) ? 1 : 0;
 
    return tmp;
    
}

static void timerRST ( int bHigh )
{
    GPIOPinWrite ( GPIO_PORTF_BASE,  GPIO_PIN_0 , bHigh? GPIO_PIN_0:0);
}

/*******************************************************************************
*������:
����()
*����: д��1byte 
*����:
*���:
*˵��:
*******************************************************************************/
static void Write_Timer_Byte ( BYTE Data )
{
    static BYTE i;

    timerScl ( 0 );//I2CSCL = 0;
    for ( i = 0; i < 8; i++ )
    {
        if ( ( Data & 0x01 ) == 0 )
        {
            timerSda ( 0 );// I2CSDA = 0; 
        }
        else
        {
            timerSda ( 1 );// I2CSDA = 1; 
        }

        timerScl ( 1 );//I2CSCL = 0; 
        TimerDly();
        timerScl ( 0 );//I2CSCL = 0;
        TimerDly();
        Data = Data >> 1;
    }
}

/*******************************************************************************
*������:
����()
*����: �� 1 byte
*����:
*���:
*˵��:
*******************************************************************************/
static BYTE Read_Timer_Byte ( void )
{
    static BYTE InByte = 0, Timer_Count;

    InByte = 0;
    
    for ( Timer_Count = 0; Timer_Count < 8; Timer_Count++ )
    {      
        if ( timerGetSda() )//��Ϊ��д��������Ѿ������˸��½��أ�����һ��bit�����ˡ�
        {
            InByte |= ( 1 << Timer_Count );
        }      
        timerScl ( 1 );
        TimerDly(); 
        timerScl ( 0 );// �½��ض��� 
        TimerDly();
        
    }
    
    return ( InByte );
}

/*******************************************************************************
*������:
����()
*����: ��ʵʱʱ��
*����:
*���:
*˵��:
*******************************************************************************/

void ReadRealtime ( void )
{
    static BYTE i;
 
    timerScl ( 0 );
    timerRST ( 0 );
    TimerDly();
    
    timerRST ( 1 );
    TimerDly();
    Write_Timer_Byte ( 0xBF );    //Read, Burst Mode

    for ( i = 0; i < 7; i++ )
    {
        SysTime[6-i] = Read_Timer_Byte();
    }
    timerScl ( 1 );
    timerRST ( 0 );
    TimerDly();
}

/*******************************************************************************
*������:
����()
*����: дʵʱʱ��
*����:
*���:
*˵��:
*******************************************************************************/
void WriteRealtime ( void )
{
    timerScl ( 0 );
    timerRST ( 0 );
    TimerDly();
    
    timerRST ( 1 );
    TimerDly();
    Write_Timer_Byte ( 0x8E );   //дʹ��
    Write_Timer_Byte ( 0x00 );    
    timerRST ( 0 );
    TimerDly();
   
    timerRST ( 1 );
    TimerDly();
    Write_Timer_Byte ( 0XBE );       //����д
    Write_Timer_Byte ( SysTime[6] & 0x7F ); //OSC enable for sure.CH=0;
    Write_Timer_Byte ( SysTime[5] );
    Write_Timer_Byte ( SysTime[4] & 0x7F );   //24 Hour Mode 
    Write_Timer_Byte ( SysTime[3] );
    Write_Timer_Byte ( SysTime[2] );
    Write_Timer_Byte ( SysTime[1]);  // ��
    Write_Timer_Byte ( SysTime[0] );//year
    Write_Timer_Byte ( 0x00 );     
    timerRST ( 0 );
    TimerDly();
    
    timerRST ( 1 );
    TimerDly();
    Write_Timer_Byte ( 0x8E );     //Write,Single Mode
    Write_Timer_Byte ( 0x80 );     // д����
    timerRST ( 0 );

}



/*******************************************************************************
*������:
����()
*����: ��ʼ��
*����:
*���:
*˵��:
*******************************************************************************/
void Init_Rtc ( void )
{
    timerScl ( 0 );
    timerRST ( 0 );
    TimerDly();
    timerRST ( 1 );
    Write_Timer_Byte ( 0x8E );   //дʹ��
    Write_Timer_Byte ( 0x00 );   //
    timerRST ( 0 );
    TimerDly();
    
    timerRST ( 1 );
    Write_Timer_Byte ( 0x80 );   //Write,Single Mode
    Write_Timer_Byte ( 0x00 );   //CH=0;��������
    timerRST ( 0 );
   
}
