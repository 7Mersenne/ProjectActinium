#include <stdio.h>  
#include <stdlib.h>  
#include <unistd.h>  
#include <fcntl.h>  
#include <string.h>  
#include <sys/types.h>  
#include <sys/stat.h> 
#include <errno.h>

#include "../include/writef.h"
using namespace std;

CWritef::CWritef()
{
    m_iModID = g_cDebug.AddModule(WRITEF_MODNAME);
    m_iServerPort = 0;
    fpath = "../file/file.txt";
}

CWritef::~CWritef()
{
    
}


int CWritef::InitWritef(unsigned char *pPacket)
{
    if(*(int *)(pPacket) != DATA_PACKETSYNC)
    {
        ACTDBG_ERROR("Frame: AppConfig Invalid Parameters.")
        return -1;
    }

    int *pData = reinterpret_cast<int*>(pPacket+44);
  
    m_iServerPort = *(int *)(pData);
 
    if(Start(m_iServerPort) == 0)
    {
        ACTDBG_INFO("InitWritef: Start ServerPort <%d> successfully.", m_iServerPort)
        return 0;
    }
    else
    {
        ACTDBG_ERROR("InitWritef: Start ServerPort <%d> fail.", m_iServerPort);
        return -1;
    }
}

int CWritef::ProcessData(int iConn, unsigned char *pBuf, int iLen)
{
    if((iConn<0) || (iConn>=ACTTCPSVR_MAXCONN) || (pBuf == NULL) || (iLen<0) || (iLen > ACTTCPSVR_MAXDATALEN))
    {
        ACTDBG_ERROR("Writef ProcessData: Invalid Params.")
        return -1;
    }
    ACTDBG_INFO("Writef ProcessData: Conn<%d>, Len<%d>", iConn, iLen)
    WriteFile(pBuf);
    return 0;
}

int CWritef::WriteFile(unsigned char *pBuf)
{
    PDATA_PACKET_HEADER pHeader;
    pHeader = (tag_DataPacketHeader *)pBuf;
    void* pDate = (void*)(pBuf + 40);
    int fp;
    if((fp=open(fpath,O_RDWR|O_APPEND)) == -1)
    {
        ACTDBG_ERROR("Writef File open: %s fail.",fpath)
        return -1;
    }
    if(write(fp,pDate,pHeader->iPayloadSize) == -1)
    {
        ACTDBG_ERROR("WriteFile error %s.",strerror(errno))
        return -1;
    }
	close(fp);
    return 0;
}