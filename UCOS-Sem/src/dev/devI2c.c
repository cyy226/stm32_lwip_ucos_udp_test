/************************************************************************************/
/*����:������                                                                       */
/*����:I2C�ӿڴ��� ,                                                                 */
/*�޶���¼:                                                                         */
/************************************************************************************/

#if 0
#include    <hw_ints.h> 
#include    <interrupt.h> 
#include    <sysctl.h> 
#include    <gpio.h> 
#include    "devI2c.h"

//    ���幤��״̬ 
#define    STAT_IDLE       0          //    ����״̬ 
#define    STAT_ADDR      1          //    �������ݵ�ַ״̬ 
#define    STAT_DATA      2          //    ���ջ�������״̬ 
#define    STAT_FINISH      3          //    �շ����״̬ 
 
//    ����ȫ�ֱ��� 
static unsigned long I2CM_BASE = I2C0_MASTER_BASE;    //    ���� I2C ������ַ������ʼ�� 
static tI2CM_DEVICE gtDevice;                         //    �������ݽӿ� 
static unsigned char gucStatus = STAT_IDLE;         //    ����״̬ 
static tBoolean gbSendRecv;                         //    �շ�������־��false ���ͣ�true ���� 
static char gcAddr[4];                          //    ���ݵ�ַ���� 
static unsigned int guiAddrIndex;              //    ���ݵ�ַ������������ 
static unsigned int guiDataIndex;              //    ���ݻ������������� 
 
//    �� tI2CM_DEVICE �ṹ�������ʼ�������������ݳ�Ա 
void I2CM_DeviceInitSet(tI2CM_DEVICE *pDevice, unsigned char ucSLA, 
                      unsigned long ulAddr, 
                      unsigned int uiLen, 
                      char *pcData, 
                      unsigned int uiSize) 
{ 
    pDevice->ucSLA = ucSLA; 
    pDevice->ulAddr = ulAddr; 
    pDevice->uiLen = uiLen; 
    pDevice->pcData = pcData; 
    pDevice->uiSize = uiSize; 
} 
 
//    �� tI2CM_DEVICE �ṹ����������������շ���صĳ�Ա�����ݵ�ַ�����ݻ����������ݴ�С�� 
void I2CM_DeviceDataSet(tI2CM_DEVICE *pDevice, unsigned long ulAddr, 
                        char *pcData, 
                        unsigned int uiSize) 
{ 
    pDevice->ulAddr = ulAddr; 
    pDevice->pcData = pcData; 
    pDevice->uiSize = uiSize; 
}

//    I2C ������ʼ�� 
void I2CM_Init(void) 
{ 
    I2CM_DeviceInitSet(&gtDevice, 0, 0, 0, (void *)0, 0); 
 
    if ((I2CM_BASE != I2C0_MASTER_BASE) && (I2CM_BASE != I2C1_MASTER_BASE)) 
    { 
        I2CM_BASE = I2C0_MASTER_BASE; 
    } 
 
    switch (I2CM_BASE) 
    { 
        case I2C0_MASTER_BASE: 
        SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C0);     //    ʹ�� I2C0 ģ�� 
        SysCtlPeripheralEnable(I2C0SCL_PERIPH);       //    ʹ�� SCL ���ڵ� GPIO ģ�� 
        GPIOPinTypeI2C(I2C0SCL_PORT, I2C0SCL_PIN);    //    ������عܽ�Ϊ SCL ���� 
        SysCtlPeripheralEnable(I2C0SDA_PERIPH);       //    ʹ�� SDA ���ڵ� GPIO ģ�� 
        GPIOPinTypeI2C(I2C0SDA_PORT, I2C0SDA_PIN);    //    ������عܽ�Ϊ SDA ���� 
        IntEnable(INT_I2C0);                   //    ʹ�� I2C0 �ж� 
        break; 
        /*
        case I2C1_MASTER_BASE: 
        SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C1);     //    ʹ�� I2C1 ģ�� 
        SysCtlPeripheralEnable(I2C1SCL_PERIPH);       //    ʹ�� SCL ���ڵ� GPIO ģ�� 
        GPIOPinTypeI2C(I2C1SCL_PORT, I2C1SCL_PIN);    //    ������عܽ�Ϊ SCL ���� 
        SysCtlPeripheralEnable(I2C1SDA_PERIPH);       //    ʹ�� SDA ���ڵ� GPIO ģ�� 
        GPIOPinTypeI2C(I2C1SDA_PORT, I2C1SDA_PIN);    //    ������عܽ�Ϊ SDA ���� 
        IntEnable(INT_I2C1);               //    ʹ�� I2C1 �ж� 
        break; 
        */
        default: 
        break; 
    } 
 
    I2CMasterInit(I2CM_BASE, false);            //    I2C ����ģ���ʼ����100kbps 
    I2CMasterIntEnable(I2CM_BASE);            //    ʹ�� I2C ��ģ���ж� 
    IntMasterEnable( );                     //    ʹ�ܴ������ж� 
    I2CMasterEnable(I2CM_BASE);              //    ʹ�� I2C ���� 
} 

//��ʽһ:ʹ���жϴ���  
//    ���ܣ�I2C �������ͻ�������� 
//    ������pDevice ��ָ�� tI2CM_DEVICE �ͽṹ�������ָ�� 
//                bFlag ȡֵ false ��ʾ���Ͳ�����ȡֵ true ��ʾ���ղ��� 
//    ���أ�I2C_MASTER_ERR_NONE    û�д��� 
//          I2C_MASTER_ERR_ADDR_ACK  ��ַӦ����� 
//          I2C_MASTER_ERR_DATA_ACK  ����Ӧ����� 
//          I2C_MASTER_ERR_ARB_LOST   ������ͨ���ٲ�ʧ�� 
//    ���͸�ʽ��S | SLA+W | addr[1��4] | data[1��n] | P 
//    ���ո�ʽ��S | SLA+W | addr[1��4] | Sr | SLA+R | data[1��n] | P 
unsigned long I2CM_SendRecv(tI2CM_DEVICE *pDevice, tBoolean bFlag) 
{ 
    //    ���ݵ�ַ���Ȼ��շ����ݴ�С����Ϊ 0������ִ���κβ��� 
    if ((pDevice->uiLen <= 0) || (pDevice->uiSize <= 0)) 
    { 
        return(I2C_MASTER_ERR_NONE); 
    } 
 
    gtDevice = *pDevice; 
    if (gtDevice.uiLen > 4) gtDevice.uiLen = 4;          //    ���ݵ�ַ���Ȳ��ܳ��� 4B 
 
    gbSendRecv = bFlag;                  //    ���ȫ�ֱ�����ʼ�� 
    guiAddrIndex = 0; 
    guiDataIndex = 0; 
 
    switch (gtDevice.uiLen)                //    �����ݵ�ַ�ֽ������ 
    { 
        case 1:                      //    1 �ֽ����ݵ�ַ 
        gcAddr[0] = (char)(gtDevice.ulAddr); 
        break; 
 
        case 2:                      //    2 �ֽ����ݵ�ַ 
        gcAddr[0] = (char)(gtDevice.ulAddr >> 8); 
        gcAddr[1] = (char)(gtDevice.ulAddr); 
        break; 
 
        case 3:                      //    3 �ֽ����ݵ�ַ 
        gcAddr[0] = (char)(gtDevice.ulAddr >> 16); 
        gcAddr[1] = (char)(gtDevice.ulAddr >> 8); 
        gcAddr[2] = (char)(gtDevice.ulAddr); 
        break; 
 
        case 4:                      //    4 �ֽ����ݵ�ַ 
        gcAddr[0] = (char)(gtDevice.ulAddr >> 24); 
        gcAddr[1] = (char)(gtDevice.ulAddr >> 16); 
        gcAddr[2] = (char)(gtDevice.ulAddr >> 8); 
        gcAddr[3] = (char)(gtDevice.ulAddr); 
        break; 
 
        default: 
        break; 
    }

    //����Ƕ�����ͨ�ţ�����Ҫ���ȵȴ����߿��� 
    //while (I2CMasterBusBusy(I2CM_BASE));        //    �ȴ����߿��� 
 
    I2CMasterSlaveAddrSet(I2CM_BASE, gtDevice.ucSLA, false);  //    ���ôӻ���ַ��д���� 
    I2CMasterDataPut(I2CM_BASE, gcAddr[guiAddrIndex++]);    //    ��ʼ�������ݵ�ַ 
    gucStatus = STAT_ADDR;                //    ����״̬���������ݵ�ַ 
 
    //    �������ͻ��������ʼ 
    I2CMasterControl(I2CM_BASE, I2C_MASTER_CMD_BURST_SEND_START); 
 
    while (gucStatus != STAT_IDLE);             //    �ȴ����߲������ 
 
    return(I2CMasterErr(I2CM_BASE));            //    ���ؿ��ܵĴ���״̬ 
} 


// I2C �жϷ����� 
void I2C_ISR(void) 
{ 
    unsigned long ulStatus; 
 
    ulStatus = I2CMasterIntStatus(I2CM_BASE, true);        //    ��ȡ�ж�״̬ 
    I2CMasterIntClear(I2CM_BASE);             //    ����ж�״̬ 
 
    if (I2CMasterErr(I2CM_BASE) != I2C_MASTER_ERR_NONE)  //    ���������� 
    { 
        gucStatus = STAT_IDLE; 
        return; 
    } 
 
    if (!ulStatus) return; 
 
    switch (gucStatus) 
    { 
        case STAT_ADDR:                  //    �������ݵ�ַ״̬ 
        if (guiAddrIndex < gtDevice.uiLen)          //    �����ݵ�ַδ������� 
        {    
            //    �����������ݵ�ַ 
            I2CMasterDataPut(I2CM_BASE, gcAddr[guiAddrIndex++]); 
            //    �������ͻ�����ͼ��� 
            I2CMasterControl(I2CM_BASE, I2C_MASTER_CMD_BURST_SEND_CONT); 
 
            break; 
        } 
        else 
        { 
            gucStatus = STAT_DATA;            //    ����״̬���շ����� 
 
            if (gbSendRecv)                //    ���ǽ��ղ��� 
            { 
                //���ôӻ���ַ�������� 
                I2CMasterSlaveAddrSet(I2CM_BASE, gtDevice.ucSLA, true); 
 
                if (gtDevice.uiSize == 1)          //    ��ֻ׼������ 1 ���ֽ� 
                { 
                    gucStatus = STAT_FINISH;        //    ����״̬�����ս��� 
 
                    //����������ν��� 
                    I2CMasterControl(I2CM_BASE, I2C_MASTER_CMD_SINGLE_RECEIVE); 
                } 
                else 
                { 
                    //    �������ͻ��������ʼ 
                    I2CMasterControl(I2CM_BASE, 
                    I2C_MASTER_CMD_BURST_RECEIVE_START); 
                } 
 
                break; 
            } 
        } 
 
        //    ֱ�ӽ�����һ�� case ��� 
 
        case STAT_DATA:                  //    �շ�����״̬ 
        if (gbSendRecv)                  //    ���ǽ��ղ��� 
        { 
            //    ��ȡ���յ������� 
            gtDevice.pcData[guiDataIndex++] = I2CMasterDataGet(I2CM_BASE); 
 
            if (guiDataIndex + 1 < gtDevice.uiSize)       //    ������δ������� 
            { 
                //    �������ͻ�����ռ��� 
                I2CMasterControl(I2CM_BASE, I2C_MASTER_CMD_BURST_RECEIVE_CONT); 
            } 
            else 
            { 
                gucStatus = STAT_FINISH;          //    ����״̬��������� 
 
                //    �������ͻ��������� 
                I2CMasterControl(I2CM_BASE, I2C_MASTER_CMD_BURST_RECEIVE_FINISH); 
            } 
        } 
        else 
        { 
             //    �������� 
            I2CMasterDataPut(I2CM_BASE, gtDevice.pcData[guiDataIndex++]); 
 
            if (guiDataIndex < gtDevice.uiSize)        //    ������δ������� 
            { 
                //    �������ͻ�����ͼ��� 
                I2CMasterControl(I2CM_BASE, I2C_MASTER_CMD_BURST_SEND_CONT); 
            } 
            else 
            { 
                gucStatus = STAT_FINISH;          //    ����״̬��������� 
 
                //    �������ͻ��������� 
                I2CMasterControl(I2CM_BASE, I2C_MASTER_CMD_BURST_SEND_FINISH); 
            } 
    } 
 
    break; 
 
    case STAT_FINISH:                  //    �շ����״̬ 
    if (gbSendRecv)                  //    ���ǽ��ղ��� 
    { 
        //    ��ȡ������յ������� 
        gtDevice.pcData[guiDataIndex] = I2CMasterDataGet(I2CM_BASE); 
    } 
 
    gucStatus = STAT_IDLE;              //    ����״̬������ 
    break; 
 
    default: 
    break; 
    } 
} 


//��ʽ��:��ʹ���ж�
void FMWriteData(BYTE FMSLAVE_ADDR, WORD WRITE_ADDR, BYTE *pDat, BYTE datlen)
{
    BYTE ucIndex;
    I2CMasterInitExpClk(I2C0_MASTER_BASE, SysCtlClockGet(), false); 
    
    //  ָ���ӻ���ַ 
    I2CMasterSlaveAddrSet(I2C0_MASTER_BASE, FMSLAVE_ADDR, 0); //д
   
    //  �����ӵ�ַHbyte
    I2CMasterDataPut(I2C0_MASTER_BASE, (BYTE)WRITE_ADDR>>8); 
 
    //  ��������״̬ 
    I2CMasterControl(I2C0_MASTER_BASE, I2C_MASTER_CMD_BURST_SEND_START); 
     
    //  �ȴ����ݷ��ͽ��� 
    while(I2CMasterBusy(I2C0_MASTER_BASE)); 


    //  �����ӵ�ַLbyte
    I2CMasterDataPut(I2C0_MASTER_BASE, (BYTE)WRITE_ADDR); 
    I2CMasterControl(I2C0_MASTER_BASE, I2C_MASTER_CMD_BURST_SEND_CONT);
    //  �ȴ����ݷ��ͽ��� 
    while(I2CMasterBusy(I2C0_MASTER_BASE)); 
               
    //  �������� 
    for(ucIndex = 0; ucIndex < datlen; ucIndex++)
    { 
        //  ��Ҫ���͵����ݷŵ����ݼĴ��� 
        I2CMasterDataPut(I2C0_MASTER_BASE, pDat[ucIndex]); 
 
        //  ������������ 
        I2CMasterControl(I2C0_MASTER_BASE, I2C_MASTER_CMD_BURST_SEND_CONT); 
 
        //  �ȴ����ݷ��ͽ���
        while(I2CMasterBusy(I2C0_MASTER_BASE)); 
    } 
    // 
    //  ���ͽ������� 
    // 
    I2CMasterControl(I2C0_MASTER_BASE, I2C_MASTER_CMD_BURST_SEND_FINISH); 
 
    // 
    //  �ȴ�����ͽ��� 
    // 
    while(I2CMasterBusy(I2C0_MASTER_BASE)); 
}

void FMReadData(BYTE FMSLAVE_ADDR, WORD READ_ADDR, BYTE *pDat, BYTE datlen)
{

    BYTE ucIndex;
    //  ָ���ӻ���ַ 
    I2CMasterSlaveAddrSet(I2C0_MASTER_BASE, FMSLAVE_ADDR, 0); //д
   
    //  �����ӵ�ַHbyte 
    I2CMasterDataPut(I2C0_MASTER_BASE, (BYTE)READ_ADDR>>8); 
 
    //  ��������״̬ 
    I2CMasterControl(I2C0_MASTER_BASE, I2C_MASTER_CMD_BURST_SEND_START); 
     
    //  �ȴ����ݷ��ͽ��� 
    while(I2CMasterBusy(I2C0_MASTER_BASE)); 


    //  �����ӵ�ַLbyte 
    I2CMasterDataPut(I2C0_MASTER_BASE, (BYTE)READ_ADDR); 
    //  ��������״̬
    I2CMasterControl(I2C0_MASTER_BASE, I2C_MASTER_CMD_BURST_SEND_CONT);
    
    //  �ȴ����ݷ��ͽ��� 
    while(I2CMasterBusy(I2C0_MASTER_BASE)); 
 

    //  ָ���ӻ���ַ����
    I2CMasterSlaveAddrSet(I2C0_MASTER_BASE, FMSLAVE_ADDR, 1); 
    //
    I2CMasterControl(I2C0_MASTER_BASE, I2C_MASTER_CMD_BURST_RECEIVE_START);
        
    for(ucIndex = 0; ucIndex < datlen-1; ucIndex++)
    {
        //  �ȴ����ݽ������ 
        while(I2CMasterBusy(I2C0_MASTER_BASE));
        pDat[ucIndex]=I2CMasterDataGet(I2C0_MASTER_BASE);
        I2CMasterControl(I2C0_MASTER_BASE, I2C_MASTER_CMD_BURST_RECEIVE_CONT);
    }

    //  �ȴ����ݽ������ 
    while(I2CMasterBusy(I2C0_MASTER_BASE));
    pDat[ucIndex+1]=I2CMasterDataGet(I2C0_MASTER_BASE);

    I2CMasterControl(I2C0_MASTER_BASE, I2C_MASTER_CMD_BURST_RECEIVE_FINISH);
}
#endif

#include    <hw_ints.h> 
#include    <interrupt.h> 
#include    <sysctl.h> 
#include    <gpio.h> 
#include    "devI2c.h"
#include    "rn8209x.h"

//��ʽ��:ʹ��IO��ģ��I2C
//����ΪI2C��������
/*************************************************************************/ 
/*************************************************************************/ 
/*************************************************************************/ 
void I2C_FM24C_Init(void)
{

    SysCtlPeripheralEnable(SCL_PERIPH);
    GPIOPinTypeGPIOOutput(SCL_PORT,SCL_PIN); 

    SysCtlPeripheralEnable(SDA_PERIPH);
    GPIOPinTypeGPIOOutput(SDA_PORT,SDA_PIN); 
    
    SCL_H;
    SDA_H;
    SDA_out;
   
}

void I2CGPIOINIT()
{
    GPIOPinTypeGPIOOutput(SCL_PORT,SCL_PIN);
    GPIOPinTypeGPIOOutput(SDA_PORT,SDA_PIN);
    SCL_H;
    SDA_H;
    SDA_out;
    
}
/*******************************************
�������ƣ�start
��    �ܣ����IIC����ʼ��������
��    ������
����ֵ  ����
********************************************/
static void start(void)
{
      SCL_H;
      SDA_H;
      SysCtlDelay(5*(TheSysClock/3000000));//һ��ѭ����ʱռ��3������
      SDA_L;
      SysCtlDelay(5*(TheSysClock/3000000));
      SCL_L;
      //SysCtlDelay(4*(TheSysClock/3000000));
}
/*******************************************
�������ƣ�stop
��    �ܣ����IIC����ֹ��������
��    ������
����ֵ  ����
********************************************/
static void stop(void)
{
      SDA_L;
      SCL_H;
      SysCtlDelay(5*(TheSysClock/3000000));
      SDA_H;
      SysCtlDelay(5*(TheSysClock/3000000));
      SCL_L;
}
/*******************************************
�������ƣ�mack
��    �ܣ����IIC������Ӧ�����
��    ������
����ֵ  ����
********************************************/
static void mack(void)
{
      SDA_L;
      SysCtlDelay(2*(TheSysClock/3000000));
      SCL_H;
      SysCtlDelay(5*(TheSysClock/3000000));
      SCL_L;
//      SysCtlDelay(5*(TheSysClock/3000000));
      
}
/*******************************************
�������ƣ�mnack
��    �ܣ����IIC��������Ӧ�����
��    ������
����ֵ  ����
********************************************/
static void mnack(void)
{
      SDA_H;
      SysCtlDelay(2*(TheSysClock/3000000));
      SCL_H;
      SysCtlDelay(5*(TheSysClock/3000000));
      SCL_L;
//      SysCtlDelay(5*(TheSysClock/3000000));
       
}

/**********���Ӧ���źź���******************/
/*�������ֵΪ1��֤����Ӧ���źţ���֮û��*/
/*******************************************
�������ƣ�check
��    �ܣ����ӻ���Ӧ�����
��    ������
����ֵ  ���ӻ��Ƿ���Ӧ��1--�У�0--��
********************************************/
static void ack(void)
{
    unsigned char  k;
    k=0;
    SDA_in;
    SCL_H;
    SysCtlDelay(5*(TheSysClock/3000000));
    while((GPIOPinRead(SDA_PORT,SDA_PIN)&SDA_PIN)&&(k<250)) k++;
    SDA_out;
    SCL_L;
//    SysCtlDelay(5*(TheSysClock/3000000));
}

/*******************************************
�������ƣ�write1byte
��    �ܣ���IIC���߷���һ���ֽڵ�����
��    ����wdata--���͵�����
����ֵ  ����
********************************************/
static void write1byte(unsigned char  wdata)
{
      unsigned char  i;
      
      for(i=0;i<8;i++)
      {
         SCL_L; //SCL=0;
         SysCtlDelay(2*(TheSysClock/3000000));//�ȴ�5����������	
         if((wdata&0x80)==0x80)
             SDA_H;//SDA=1;
         else
             SDA_L;//SDA=0;
             
         SCL_H;//SCL=1;
         wdata=wdata<<1;
         SysCtlDelay(3*(TheSysClock/3000000));//�ȴ�5����������	

      }
      SCL_L;//SCL=0;
}
/*******************************************
�������ƣ�read1byte
��    �ܣ���IIC���߶�ȡһ���ֽ�
��    ������
����ֵ  ����ȡ������
********************************************/
static unsigned char  read1byte(void)
{
#if 1 //SDA,SCL��û����������
    unsigned char  rdata = 0x00,i;
    unsigned char  flag;

    for(i = 0;i < 8;i++)
    {
      SDA_H;
      SCL_H;
      
      SDA_in;
      SysCtlDelay(5*(TheSysClock/3000000));
      flag = SDA_val;
      rdata <<= 1;
      if(flag)  rdata |= 0x01;
      SDA_out;
      SCL_L;
      SysCtlDelay(5*(TheSysClock/3000000));
    }    
    return rdata;
#else//SDA,SCL�����˵���
    unsigned char retc,i;  
    retc=0; 
 
    SDA_in;
    for(i=0;i<8;i++)
    {        
        SCL_L;//SCL=0;      
        SysCtlDelay(5*(TheSysClock/3000000));//�ȴ�5����������
        SCL_H;//SCL=1;       
        SysCtlDelay(5*(TheSysClock/3000000));//�ȴ�5����������
            
        retc=retc<<1;
        if((GPIOPinRead(SDA_PORT,SDA_PIN)&SDA_PIN))retc=retc+1; 
        SysCtlDelay(4*(TheSysClock/3000000));//�ȴ�5����������	     
    }
    SCL_L;//SCL=0;    
    SysCtlDelay(5*(TheSysClock/3000000));//�ȴ�5����������	
    SDA_out;
    return(retc);
#endif
}

//����ΪFM24CL04Ӧ�ó���
/***********************************************************************************/ 
/*��ΪFM24CL04�ڲ��ռ��СΪ512byte,������Word ��ַ,0x0000--0x01FF,���Ƿ�***********/ 
/*����2ҳ��ÿҳ256byte��С������0x00--0xFF�Ĳ����ǶԵ�0Ҷ�Ĳ���������0x0100--0x01FF*/ 
/*�Ĳ����ǶԵ�1ҳ�Ĳ�������д�������ַʱӦע��*************************************/ 
/***********************************************************************************/ 

//���ֽ�����д��
void  Write_FM24C(BYTE sla,WORD suba,BYTE *s,BYTE no)
{
    unsigned char  i,slabuf;
    unsigned int subabuf;

    I2C_FM24C_Init(); //ӦΪ��ʱ�ӹ���GPIO������ÿ�ζ�дǰʹ��ǰ��ʼ��
    
    slabuf = sla;   //A0=0(page=0)
    subabuf = suba;
    if(subabuf >= 0x100)
    {
        slabuf = sla|0x02;//A0=1(page = 1);
    }
    start();             
    write1byte(slabuf);    //��������A0,R/W=0 ����д         
    ack();
    write1byte((BYTE)subabuf);      //�����ڵ�Ԫ��ַ,ȡ��8λ��           
    ack();
    for(i=0;i<no;i++)
    {   
        write1byte(*s);               
        ack();
        s++;
    } 
    stop();             

} 
//���ֽ����ݶ���
void  Read_FM24C(BYTE sla,WORD suba,BYTE *s,BYTE num)
{
    unsigned char  i,slabuf;
    unsigned int subabuf;

    I2C_FM24C_Init();
    
    slabuf = sla;
    subabuf = suba;
    if(subabuf>=0x100)
    {
        slabuf = sla |0x02;//A0=1;page = 1;
    }
    start();             
    write1byte(slabuf);     //��������A0,R/W=0 ����д        
    ack();
    write1byte((BYTE)subabuf);      //�����ڵ�Ԫ��ַ ���ֽ�          
    ack();
    
    start();   
    write1byte(slabuf+1);  //��������A0,R/W=1 �����  
    ack();
    
    for(i=0;i<num-1;i++)
    {   
        *s=read1byte();               
        mack();                
        s++;
    } 
    *s=read1byte();
    mnack();                
    stop();                
}

//����������ֽ�����
void  Clear_FM24C(BYTE sla,WORD suba,BYTE no)
{
    unsigned char  i,slabuf;
    unsigned int subabuf;

    I2C_FM24C_Init();
    
    slabuf = sla;   //A0=0(page=0)
    subabuf = suba;
    if(subabuf>=0x100)
    {
        slabuf = sla|0x02;//A0=1(page = 1);
    }
    start();             
    write1byte(slabuf);    //��������A0,R/W=0 ����д         
    ack();
    write1byte((BYTE)subabuf);      //�����ڵ�Ԫ��ַ,ȡ��8λ��           
    ack();
    for(i=0;i<no;i++)
    {   
        write1byte(0x00);               
        ack();
    } 
    stop();                 
}

/*************************************************************************************************
**                                  END FILE
**************************************************************************************************/

 
