#ifndef    __DEVI2C_H__ 
#define    __DEVI2C_H__ 

#if 0
#include    <hw_types.h> 
#include    <hw_memmap.h>
#include    <pin_map.h>
#include    <i2c.h> 

#define BYTE unsigned char
#define WORD unsigned short
#define DWORD unsigned long
#define uint unsigned int

//    ���� I2C �����ṹ�� 
typedef struct 
{ 
  unsigned char ucSLA;      //    �ӻ���ַ������ 7 λ����ַ��������д����λ�� 
  unsigned long ulAddr;     //    ���ݵ�ַ 
  unsigned int uiLen;      //    ���ݵ�ַ���ȣ�ȡֵ 1��2 �� 4�� 
  char *pcData;        //    ָ���շ����ݻ�������ָ�� 
  unsigned int uiSize;      //    �շ����ݳ��� 
} tI2CM_DEVICE; 
 
//    �� tI2CM_DEVICE �ṹ�������ʼ�������������ݳ�Ա 
extern void I2CM_DeviceInitSet(tI2CM_DEVICE *pDevice, unsigned char ucSLA, 
                          unsigned long ulAddr, 
                          unsigned int uiLen, 
                          char *pcData, 
                          unsigned int uiSize); 
 
//    �� tI2CM_DEVICE �ṹ����������������շ���صĳ�Ա�����ݵ�ַ�����ݻ����������ݴ�С�� 
extern void I2CM_DeviceDataSet(tI2CM_DEVICE *pDevice, unsigned long ulAddr, 
                      char *pcData, 
                      unsigned int uiSize); 
 
//    I2C ������ʼ�� 
extern void I2CM_Init(void); 
 
//    I2C �������ͻ�������� 
extern unsigned long I2CM_SendRecv(tI2CM_DEVICE *pDevice, tBoolean bFlag); 
 
//    ����꺯������������ 
#define    I2CM_DataSend(pDevice)    I2CM_SendRecv(pDevice, false) 
 
//    ����꺯������������ 
#define    I2CM_DataRecv(pDevice)    I2CM_SendRecv(pDevice, true) 
#endif

#include "inc/hw_memmap.h"	  
#include "inc/hw_types.h" 
#include "inc/hw_ints.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "main.h"


#define BYTE unsigned char
#define WORD unsigned short
#define DWORD unsigned long
#define uint unsigned int

/*************************************************************************/ 
/*************************************************************************/ 
#define  SCL_PERIPH     SYSCTL_PERIPH_GPIOB
#define  SCL_PORT       GPIO_PORTB_BASE
#define  SCL_PIN        GPIO_PIN_2//SCL

#define  SDA_PERIPH     SYSCTL_PERIPH_GPIOB
#define  SDA_PORT       GPIO_PORTB_BASE
#define  SDA_PIN        GPIO_PIN_3      //SDA

#define SCL_H           GPIOPinWrite (SCL_PORT, SCL_PIN, SCL_PIN)
#define SCL_L           GPIOPinWrite (SCL_PORT, SCL_PIN, 0)
#define SDA_H           GPIOPinWrite (SDA_PORT, SDA_PIN, SDA_PIN)
#define SDA_L           GPIOPinWrite (SDA_PORT, SDA_PIN, 0)

#define SDA_in          GPIOPinTypeGPIOInput(SDA_PORT, SDA_PIN)     //SDA�ĳ�����ģʽ
#define SDA_out         GPIOPinTypeGPIOOutput(SDA_PORT,SDA_PIN)     //SDA������ģʽ
#define SDA_val         GPIOPinRead(SDA_PORT,SDA_PIN)               //SDA��λֵ
/*************************************************************************/ 
/*************************************************************************/ 
//#define   FMSLAVE_ADDR       0xAC   // 1010_1100
#define   FMSLAVE_ADDR       0xA8   // 1010_1100


#define   delay_us(x)   SysCtlDelay(x*(TheSysClock/3000000));       //��ʱuS
#define   delay_ms(x)   SysCtlDelay(x*(TheSysClock/3000));          //��ʱmS
/*************************************************************************/ 
/*************************************************************************/ 

//����ΪI2C��������
/*************************************************************************/ 
/*************************************************************************/ 
/*************************************************************************/  
void I2C_FM24C_Init(void);


//����ΪFM24C04Ӧ�ó���
/*************************************************************************/ 
/*************************************************************************/ 
extern void  Write_FM24C(BYTE sla,WORD suba,BYTE *s,BYTE no);
//���ֽ����ݶ���
extern void  Read_FM24C(BYTE sla,WORD suba,BYTE *s,BYTE num);
//
extern void  Clear_FM24C(BYTE sla,WORD suba,BYTE no);

/*************************************************************************************************
**					����������				END FILE
**************************************************************************************************/

 
#endif    //    __DEVI2C_H__ 




