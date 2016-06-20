

#include <netBuf.h>
#include <includes.h>


#define NET_BUF_MAGIC 0x12345678

static uint netBufAllCount;
static uint netBufFreeCount;
static NET_BUF_ID bufNodeHeader;
static NET_BUF_ID bufNodeHeaderTemp;


#define ROUND_UP(x, align)  (((uint) (x) + (align - 1)) & ~(align - 1))
#define ROUND_DOWN(x, align)    ((uint)(x) & ~(align - 1))
#define ALIGNED(x, align)   (((uint)(x) & (align - 1)) == 0)


/*******************************************************************************
*������:
����netBufInit()
*����:
*����:
*���:
*˵��:
*******************************************************************************/
uint netBufInit ( uint baseAddr, uint memSize )
{
    uint i = 0;
    NET_BUF_ID temp = NULL;

    bufNodeHeader = ( NET_BUF_ID ) ROUND_UP ( baseAddr, sizeof ( uint ) );

    memSize -= ( ROUND_UP ( baseAddr, sizeof ( uint ) ) - baseAddr );

    netBufAllCount = memSize / sizeof ( NET_BUF );
    netBufFreeCount = netBufAllCount;

    if ( netBufAllCount == 0 )
    {
        return false;
    }

    temp = bufNodeHeader;

    for ( i = 1; i < netBufAllCount; i++ )
    {
        temp->pNext = bufNodeHeader + i;
        temp->magicNum = NET_BUF_MAGIC;
        temp = temp->pNext;
    }

    temp->magicNum = NET_BUF_MAGIC;
    temp->pNext = NULL;

    bufNodeHeaderTemp = bufNodeHeader;

    return true;
}
/*******************************************************************************
*������:
����netBufGet()
*����:
*����:
*���:
*˵��:
*******************************************************************************/
NET_BUF_ID netBufGet()
{
#if OS_CRITICAL_METHOD == 3u                     /* Allocate storage for CPU status register           */
    OS_CPU_SR  cpu_sr = 0u;
#endif


    NET_BUF_ID temp = NULL;
    // unsigned int lockKey = 0;


    if ( bufNodeHeader == NULL )
    {
        //  printf("\n!!! netBufGet() fail \n");
        return NULL;
    }

    // lockKey = intLock();
    OS_ENTER_CRITICAL();

    temp = bufNodeHeader;
    bufNodeHeader = bufNodeHeader->pNext;
    netBufFreeCount--;

    // intUnlock ( lockKey );
    OS_EXIT_CRITICAL();

    temp->dataLen = 0;
    temp->pNext = NULL;

    return temp;
}

/*************************************
netBufFree ������س���,����netBufFreeCount����
����,���ó�����Բ�������ֵ
*************************************/
/*******************************************************************************
*������:
����netBufFree()
*����:
*����:
*���:
*˵��:
*******************************************************************************/
uint netBufFree ( NET_BUF_ID pBuf )
{
#if OS_CRITICAL_METHOD == 3u                     /* Allocate storage for CPU status register           */
    OS_CPU_SR  cpu_sr = 0u;
#endif

    //  unsigned int lockKey = 0;
    //NET_BUF_ID temp = NULL;

    if ( pBuf == NULL )
    {
        return false;
    }

    if ( pBuf->magicNum != NET_BUF_MAGIC )
    {
        return false;
    }

    //  lockKey = intLock();
    OS_ENTER_CRITICAL();
#if 0// ifdef SYS_DEBUG
    temp = bufNodeHeader;

    while ( temp )
    {
        if ( temp == pBuf )
            break;

        temp = temp->pNext;
    }

    if ( temp == pBuf )
    {
        //  intUnlock ( lockKey );
        OS_EXIT_CRITICAL();
        return false;
    }

#endif
    pBuf->pNext = bufNodeHeader;
    bufNodeHeader = pBuf;
    netBufFreeCount++;

    // intUnlock ( lockKey );
    OS_EXIT_CRITICAL();

    return true;
}



uint bufAllCountGet()
{
    return netBufAllCount;
}

uint bufFreeCountGet()
{
    return netBufFreeCount;
}
/*******************************************************************************
*������:
����netBufShow()
*����:
*����:
*���:
*˵��:
*******************************************************************************/
void netBufShow ( void )
{
    uint i = 0;
    NET_BUF_ID pBuf = NULL;

    pBuf = bufNodeHeaderTemp;

    printf ( "\r\nnetFree:%d/%d" , netBufFreeCount , netBufAllCount ) ;

    for ( i = 0; i < netBufAllCount; i++ )
    {
        pBuf = bufNodeHeaderTemp + i;

        printf ( "\n\r\n\rbufnode:0x%8X", ( uint ) pBuf );
        printf ( "\n\r+---------------------------------------------------------------------------+" );
        printf ( "\n\r|        magic        |      address      |        pNext      |   dataLen   |" );
        printf ( "\n\r|      %8X       |      %8x     |      %8X     |   %8X  |",
                 pBuf->magicNum, pBuf->addr, ( uint ) pBuf->pNext,
                 pBuf->dataLen );
        printf ( "\n\r+---------------------------------------------------------------------------+" );
    }
}
