
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "../include/config.h"
#include "../include/TCPServer.h"
#include "../include/nodescenter.h"

CNodesCenter::CNodesCenter():CTCPServer()
{
    memset(m_pucPacketQueue, 0, sizeof(m_pucPacketQueue));
    m_iQHead = 0;
    m_iQTail = 0;

    memset(m_pucPacketBuf, 0, sizeof(m_pucPacketBuf));
    memset(m_iBufSize, 0, sizeof(m_iBufSize));
    memset(m_iBytesInBuf, 0, sizeof(m_iBytesInBuf));
    memset(m_iFlag, 0, sizeof(m_iFlag));
}

CNodesCenter::~CNodesCenter()
{
    int i;
    for(i=0; i<ACTTCPSVR_MAXCONN; i++)
    {
        if(m_pucPacketBuf[i]) delete m_pucPacketBuf[i];
    }
}

int CNodesCenter::Init()
{
    ACTDBG_INFO("Init Nodes Center.")

    ClearQueue();

    return 0;
}

int CNodesCenter::ClearQueue()
{
    int i;
    if(m_iQHead == m_iQTail)
    {
        m_iQHead = m_iQTail = 0;
        memset(m_pucPacketQueue, 0, sizeof(m_pucPacketQueue));
        return 0;
    }

    for(i=m_iQHead; i!=m_iQTail; i++,i%=NODESCENTER_MAXQUEUE)
    {
        if(m_pucPacketQueue[i])
            delete m_pucPacketQueue[i];
    }
    memset(m_pucPacketQueue, 0, sizeof(m_pucPacketQueue));
    return 0;
}

int CNodesCenter::Push(int iConn)
{
    unsigned char *pPacket = m_pucPacketBuf[iConn];
    int iSize = m_iBytesInBuf[iConn];

    if(pPacket == NULL)
    {
        ACTDBG_WARNING("Push: null packet.")
        return 0;
    }

    if(iSize <= 0)
    {
        ACTDBG_WARNING("Push: packet zero length.")
        return 0;
    }

    if(((m_iQTail+1) % NODESCENTER_MAXQUEUE) == m_iQHead)
    {
        ACTDBG_ERROR("Push: Full, abandon packet. Serial<%d>, From<%d>, To<%d>.")
        return -1;
    }

    unsigned char *pP = new unsigned char[iSize];
    if(pP == NULL)
    {
        ACTDBG_ERROR("Push: not enough memory")
        return -1;
    }

    memcpy(pP, pPacket, iSize);
    if(m_pucPacketQueue[m_iQTail]) delete m_pucPacketQueue[m_iQTail];
    m_pucPacketQueue[m_iQTail] = pP;
    m_iQTail = (m_iQTail +1) % NODESCENTER_MAXQUEUE;
    return 0;
}

int CNodesCenter::Pop(unsigned char *&pPacket)
{
    if(pPacket == NULL)
    {
        ACTDBG_WARNING("Pop: null packet.")
        return -1; 
    }

    if(m_iQHead == m_iQTail)
    {
        ACTDBG_WARNING("Pop: empty queue.")
        pPacket = NULL;
        return -1;
    }

    pPacket = m_pucPacketQueue[m_iQHead];
    m_iQHead = (m_iQHead+1) % NODESCENTER_MAXQUEUE;
    return 0;
}

int CNodesCenter::MakeBuf(int iConn, int iNeed)
{
    int iSize = m_iBufSize[iConn];
    unsigned char *pTmp;

    iNeed += m_iBytesInBuf[iConn];

    if(iSize < NODESCENTER_MINBUFSIZE) iSize = NODESCENTER_MINBUFSIZE;
    while(iSize < NODESCENTER_MAXBUFSIZE)
    {
        if(iSize < iNeed)
            iSize <<= 1;
        else break;
    }
    pTmp = m_pucPacketBuf[iConn];
    if(m_iBufSize[iConn] < iSize)
    {
        m_pucPacketBuf[iConn] = NULL;
    }
    else return 0;
    if(m_pucPacketBuf[iConn] == NULL)
    {
        m_pucPacketBuf[iConn] = new unsigned char[iSize];
        if(pTmp)
        {
            memcpy(m_pucPacketBuf[iConn], pTmp, m_iBufSize[iConn]);
            delete pTmp;
        }
        m_iBufSize[iConn] = iSize;
    }
    return 0;
}

int CNodesCenter::ProcessData(int iConn, unsigned char *pBuf, int iLen)
{
    int i;

    if((pBuf == NULL) || (iLen <=0) || (iConn<0) || (iConn>=ACTTCPSVR_MAXCONN))
    {
        ACTDBG_ERROR("ProcessData: Invalid Parameters.")
        return -1;
    }

    int iCur = 0;
    int iCopy = 0;
    PDATA_PACKET_HEADER pHeader;
    while(iCur<iLen)
    {
        if(m_iBytesInBuf[iConn] == 0)
        {
            // find header sync word
            for(i=0; i<iLen; i++) // possible bug here: ASSERT that sync word never be splitted.
            {
                if(*(int *)(pBuf + i) == DATAPACKETSYNC) 
                    break;
            }
            if(i<iLen)
            {
                iCopy = iLen-i;
                if(iCopy>sizeof(DATA_PACKET_HEADER))
                    iCopy = sizeof(DATA_PACKET_HEADER);
                MakeBuf(iConn, iCopy);
                memcpy(m_pucPacketBuf[iConn], pBuf, iCopy);
                m_iBytesInBuf[iConn] = iCopy;
                iCur += iCopy;
            }
            else
            {
                iCur = iLen;
            }
            continue;
        }
        if(m_iBytesInBuf[iConn] < sizeof(DATA_PACKET_HEADER))
        {
            iCopy = sizeof(DATA_PACKET_HEADER) - m_iBytesInBuf[iConn];
            if(iCopy <= iLen - iCur) iCopy = iLen - iCur;
            MakeBuf(iConn, iCopy);
            memcpy(m_pucPacketBuf[iConn]+m_iBytesInBuf[iConn], pBuf+iCur, iCopy);
            m_iBytesInBuf[iConn] += iCopy;
            iCur += iCopy;
            continue;
        }
        pHeader = (PDATA_PACKET_HEADER)m_pucPacketBuf[iConn];
        iCopy = pHeader->iPayloadSize;
        if(iCopy)
        {
            if(iCopy <= iLen-iCur)
            {
                MakeBuf(iConn, iCopy);
                memcpy(m_pucPacketBuf[iConn]+m_iBytesInBuf[iConn], pBuf+iCur, iCopy);
                m_iBytesInBuf[iConn] += iCopy;
                iCur += iCopy;
                Push(iConn);
                m_iBytesInBuf[iConn] = 0;
            }
            else 
            {
                iCopy = iLen-iCur;
                MakeBuf(iConn, iCopy);
                memcpy(m_pucPacketBuf[iConn]+m_iBytesInBuf[iConn], pBuf+iCur, iCopy);
                m_iBytesInBuf[iConn] += iCopy;
                iCur += iCopy;
            }
        }
        else 
        {
            Push(iConn);
            m_iBytesInBuf[iConn] = 0;
        }
    }
    return 0;
}


int CNodesCenter::OnConnected(int iConn)
{
    return 0;
}