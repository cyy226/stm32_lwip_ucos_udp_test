/*************************************************************
�ɶ���ջ����������޹�˾   ��Ȩ����

�ļ���:  SwUpdate.h
��  ��:  �˹���
��  ��:  �������ģ��
�޶���¼:   

**************************************************************/

#ifndef __SWUPDATA_H__
#define __SWUPDATA_H__

#define FLASH_BANK_SIZE 1024

void SwUpdateRequest(void);
void SwUpdateData(void);
void SwUpdateFinish(void);
void SoftWareUpdate(void);
void ClearSwDownloadInfo(void);
#endif
