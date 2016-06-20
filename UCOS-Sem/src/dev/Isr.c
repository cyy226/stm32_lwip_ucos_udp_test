
#include "includes.h"

/*******************************************************************************
*������:
����**isr
*����:���ж���ת��������벿������
*����:
*���:
*˵��:
*******************************************************************************/
void sysTickIsr ( void )
{
    #if OS_CRITICAL_METHOD ==3
    OS_CPU_SR  cpu_sr;
    #endif

    OS_ENTER_CRITICAL();
   // OSIntNesting++;
    OSIntEnter();
    OS_EXIT_CRITICAL();

    OSTimeTick();                        // Call uC/OS-II's OSTimeTick()  ����uC/OS-II��OSTimeTick()����

    OSIntExit();
}

void uart0Isr ( void )
{
#if OS_CRITICAL_METHOD ==3
    OS_CPU_SR  cpu_sr;
#endif

    OS_ENTER_CRITICAL();
    //OSIntNesting++;
    OSIntEnter();
    OS_EXIT_CRITICAL();
    
    commBaseIsr ( 0 );

    OSIntExit();
}

void uart1Isr ( void )
{

#if OS_CRITICAL_METHOD ==3
    OS_CPU_SR  cpu_sr;
#endif

    OS_ENTER_CRITICAL();
    //OSIntNesting++;
    OSIntEnter();
    OS_EXIT_CRITICAL();

    commBaseIsr ( 1 );

    OSIntExit();
}

void uart2Isr ( void )
{
#if OS_CRITICAL_METHOD ==3
    OS_CPU_SR  cpu_sr;
#endif

    OS_ENTER_CRITICAL();
    //OSIntNesting++;
    OSIntEnter();
    OS_EXIT_CRITICAL();

    commBaseIsr ( 2 );

    OSIntExit();
 }


void i2cSlaveIsr ( void )
{
    //i2cSlaveHandler();
    //I2C_Slave_ISR();
}


/*
void Reset_Handler(void)
{
     __main();
}
*/

void IntDefaultHandler ( void )
{
    while ( 1 );
}
#if 0
/*********************************************************/
/*RN8209���ж�����(PF����,IRQ)                           */
/*********************************************************/
void  GPIO_Port_B_ISR (void)
{
    ulong ulStatus;
#if OS_CRITICAL_METHOD ==3
    OS_CPU_SR  cpu_sr;
#endif
    
    OS_ENTER_CRITICAL();
    OSIntNesting++;
    OS_EXIT_CRITICAL();
    
    ulStatus = GPIOPinIntStatus(GPIO_PORTB_BASE, true);        //    ��ȡ�ж�״̬ 
    GPIOPinIntClear(GPIO_PORTB_BASE, ulStatus);                //    ����ж�״̬����Ҫ 
    
    if(ulStatus & GPIO_PIN_5)
    {
         OSSemPost(RNSemIsr);
    }
     OSIntExit();
}
#endif

