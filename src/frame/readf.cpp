#include <stdio.h>  
#include <stdlib.h>  
#include <unistd.h>  
#include <fcntl.h>  
#include <string.h>  
#include <sys/types.h>  
#include <sys/stat.h> 

#include "../include/readf.h"
using namespace std;

CReadf::CReadf()
{
    m_iModID = g_cDebug.AddModule(READF_MODNAME);
    m_iSerialCon = 0;
    fpath = "../file/file.txt";
}

CReadf::~CReadf()
{

}

int CReadf::InitReadf(unsigned char *pPacket)
{
    if(*(int *)(pPacket) != DATA_PACKETSYNC)
    {
        ACTDBG_ERROR("Frame: AppConfig Invalid Parameters.")
        return -1;
    }

    int *pData = reinterpret_cast<int*>(pPacket+48);
  
    m_iSendPort = *(int *)(pData);
    pData = pData + 4;

    int m_iClientIp = *(int *)(pData);
    pData = pData + 4;

    struct in_addr inAddr;
    inAddr.s_addr = m_iClientIp;
    m_iSendIp = inet_ntoa(inAddr); 
    if(Start(m_iSendIp,m_iSendPort) == 0)
    {
        ACTDBG_INFO("InitReadf: Start SendPort <%d> successfully.", m_iSendPort)
    }
    else
    {
        ACTDBG_ERROR("InitReadf: Start SendPort <%d> fail.", m_iSendPort);
    }
}

int CReadf::ReadFile()
{
    FILE *fp;
    char rBuf[READF_READMAXLEN];

    if((fp = fopen(fpath,"r")) == NULL)
    {
        ACTDBG_ERROR("ReadFile: %s does not existent.",fpath)
        return -1;
    }

    memset(rBuf, 0, sizeof(rBuf));
    while(!feof(fp))
    {
        fread(rBuf, 1, sizeof(rBuf), fp);
        if(Packet(rBuf, sizeof(rBuf)) == -1)
        {
            ACTDBG_ERROR("ReadFile: Packet Faild.")
            break;
        }
        if(Sendmess(m_message,sizeof(m_message)) == -1)
        {
            ACTDBG_ERROR("ReadFile: Send message Faild.")
            break;
        }
    }
    ACTDBG_INFO("ReadFile: Read and send file completed")
    return 0;
}

int CReadf::Packet(char *rBuf, int rLen)
{
    PDATA_PACKET_HEADER pHeader;
    pHeader=(struct tag_DataPacketHeader *)malloc(sizeof(struct tag_DataPacketHeader));

    if((rBuf == NULL) || (rLen <=0))
    {
        ACTDBG_ERROR("Readf: Packet Invalid Parameters.")
        return -1;
    }
    pHeader->iPayloadSize = sizeof(*rBuf);
    pHeader->iSerial = m_iSerialCon++;
    pHeader->iType = DATA_CMDTYPE_DATA;
    memcpy(&m_message[0], pHeader, 40);
    memcpy(&m_message[40], rBuf, sizeof(*rBuf));
    free(pHeader);

    return 0;
}