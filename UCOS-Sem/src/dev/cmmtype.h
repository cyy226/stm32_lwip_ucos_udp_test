/*
* Copyright (c) 2006,���գ��Ĵ���ͨ�ż������޹�˾
* All rights reserved.
*
* �ļ����ƣ� cmm.c
* �ļ���ʶ�� �����ù���ƻ���
* ����ժҪ�� ��Ҫ�������ļ�������
* �ʻ����:  MCU��Micro Controller Unit����д��ָ����Ƭ��
*            DEV����ʾ�豸������·����

* ԭ���ߣ�   �ŵ�ǿ
* ������ڣ� 2008��08��30��
*/

/*
$Log: ctype.h,v $
Revision 1.3  2011/11/03 04:52:06  zhangdq
RH03 cmm�������Ź�����

Revision 1.2  2011/05/20 08:31:28  zhangdq
RH03 CMM��Ƭ����Ԫ���Ե��Ĵ��ύ

Revision 1.1  2011/04/29 01:05:26  zhangdq
RH03 CMM��Ƭ���״��ύ

*/

#ifndef CTYPE_H
#define CTYPE_H
typedef unsigned char tBoolean;
#define  bool  uint

#ifndef true
#define true 1
#endif

#ifndef false
#define false 0
#endif

#ifndef ERROR
#define ERROR  (-1)
#endif



#define BOOLEAN tBoolean


/********************************************************************************************************
*                       Date types(Compiler specific)  �������ͣ��ͱ�������أ�                         *
********************************************************************************************************/

#define uint unsigned int
#define uchar unsigned char
#define ushort unsigned short
#define ulong unsigned long

#if 0
typedef unsigned char  uint8;          // Unsigned  8 bit quantity  �޷���8λ���ͱ���
typedef signed   char  int8;           // Signed    8 bit quantity  �з���8λ���ͱ���
typedef unsigned short uint16;         // Unsigned 16 bit quantity  �޷���16λ���ͱ���
typedef signed   short int16;          // Signed   16 bit quantity  �з���16λ���ͱ���
typedef unsigned int   uint32;         // Unsigned 32 bit quantity  �޷���32λ���ͱ���
typedef signed   int   int32;          // Signed   32 bit quantity  �з���32λ���ͱ���
typedef float           fp32;          // Single precision floating point �����ȸ�������32λ���ȣ�
typedef double          fp64;          // Double precision floating point ˫���ȸ�������64λ���ȣ�


typedef unsigned char  INT8U;                   /* �޷���8λ���ͱ���                        */
typedef signed   char  INT8;                    /* �з���8λ���ͱ���                        */
typedef unsigned short INT16U;                  /* �޷���16λ���ͱ���                       */
typedef signed   short INT16;                   /* �з���16λ���ͱ���                       */
typedef unsigned int   INT32U;                  /* �޷���32λ���ͱ���                       */
typedef signed   int   INT32;                   /* �з���32λ���ͱ���                       */
typedef float          FP32;                    /* �����ȸ�������32λ���ȣ�                 */
typedef double         FP64;                    /* ˫���ȸ�������64λ���ȣ�                 */
#endif

/*

#define  INT8U unsigned char

#define  uint16 unsigned short
#define  INT16U unsigned short
#define  INT32U unsigned int
#define  uint32 unsigned int


#define  int8   signed char
#define  INT8  signed char

#define  int16   signed short
#define  INT16  signed short

#define  int32   signed int
#define  INT32  signed int

#define fp32      float
#define fp64      double
#define FP32      float
#define FP64      double

*/

#define WORD unsigned short
#define BYTE unsigned char

#define  uint8 unsigned char

#ifndef TRUE
#define TRUE  1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef NULL
#define NULL  0
#endif

#endif

