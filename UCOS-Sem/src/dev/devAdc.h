#ifndef __DEV_ADC_H__
#define __DEV_ADC_H__

extern short gAdcValAry[4];

//�¶ȴ�����0�¶�ֵ
#define TEMP_VAL_0 gAdcValAry[1]
//�¶ȴ�����1�¶�ֵ
#define TEMP_VAL_1 gAdcValAry[2]

//ʪ�ȴ�����0��ʪ��
#define HUM_VAL_0 gAdcValAry[0]
//ʪ�ȴ�����0��ʪ��
#define HUM_VAL_1 gAdcValAry[3]

void adcInit ( void );
void adcGet4Channel( signed short * shVals );
char getTempFromAdcTab2 ( signed short adc30 );
char getTempFromAdcTab ( signed short adc30 );
char getHUM ( signed short adcVal );
void adcVotsGet ( void );
short adcToVoltMs ( signed short sh );

#endif

