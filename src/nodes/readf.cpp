#include <stdio.h>  
#include <stdlib.h>  
#include <unistd.h>  
#include <fcntl.h>  
#include <string.h>  
#include <sys/types.h>  
#include <sys/stat.h> 

#include "../include/readf.h"
using namespace std;

class CActNode *ActNewNode()
{
    class CReadf *pNode = new class CReadf();
    return (class CActNode *)pNode;
}

int ActDeleteNode(class CActNode *pNode)
{
    delete (class CReadf *)pNode;
    return 0;
}
   
CReadf::CReadf():CActNode(),CTCPClient()
{
    m_iModID = g_cDebug.AddModule(READF_MODNAME);
    m_iSerialCon = 0;
    fpath = "file/file.txt";
}

CReadf::~CReadf()
{

}

int CReadf::Init(unsigned char *&pPacket)
{
    if(*(int *)(pPacket) != DATA_PACKETSYNC)
    {
        ACTDBG_ERROR("Frame: AppConfig Invalid Parameters.")
        return -1;
    }
//    int *pData = (int *)(pPacket+52);
    ACTDBG_INFO("%d",*(int *)(pPacket+48))
  
    m_iSendPort = *(int *)(pPacket+60);


    unsigned long m_iClientIp = *(unsigned long *)(pPacket+64);

    struct in_addr inAddr;
    inAddr.s_addr = m_iClientIp;
    m_iSendIp = inet_ntoa(inAddr); 
    ACTDBG_INFO("Readf Clientport: %d,ClientIP:%s,,%ld",m_iSendPort,m_iSendIp,m_iClientIp)
    if(Start(m_iSendIp,m_iSendPort) == 0)
    {
        ACTDBG_INFO("InitReadf: Start SendPort <%d> successfully.", m_iSendPort)
    }
    else
    {
        ACTDBG_ERROR("InitReadf: Start SendPort <%d> fail.", m_iSendPort);
    }

    ReadFile();

    return 0;
}

int CReadf::ReadFile()
{
    if(OnConnect(0) == 0)
    {
        ACTDBG_INFO("ReadFile: Open file..")

        FILE *fp;
        char rBuf[READF_READMAXLEN];

        if((fp = fopen(fpath,"r")) == NULL)
        {
            ACTDBG_ERROR("ReadFile: %s does not existent.",fpath)
            return -1;
        }

        memset(rBuf, 0, sizeof(rBuf));
        ACTDBG_INFO("ReadFile: Read and send file..")
        while(!feof(fp))
        {
            fread(rBuf, 1, sizeof(rBuf), fp);
            Packet(rBuf, sizeof(rBuf));
            if(Sendmess(m_message,sizeof(m_message),0) == -1)
            {
                ACTDBG_ERROR("ReadFile: Send message Faild.")
                break;
            }
            sleep(1);
        }
        ACTDBG_INFO("ReadFile: Read and send file completed")
        return 0;
    }
    else 
    {
//        sleep(1);
        ReadFile();
        return 0;
    }
}

int CReadf::Packet(char *rBuf, int rLen)
{
    memset(m_message, 0 ,sizeof(m_message));
    PDATA_PACKET_HEADER pHeader;
    pHeader=(struct tag_DataPacketHeader *)malloc(sizeof(struct tag_DataPacketHeader));
 
    if((rBuf == NULL) || (rLen <=0))
    {
        ACTDBG_ERROR("Readf: Packet Invalid Parameters.")
        return -1;
    }
    pHeader->iSync = DATA_PACKETSYNC;
    pHeader->iPayloadSize = rLen;
    pHeader->iState = DATA_PACKETSTATE_NOREPLY;
    pHeader->iSerial = m_iSerialCon++;
    pHeader->iType = DATA_CMDTYPE_DATA;
    memcpy(&m_message[0], pHeader, 40);
    memcpy(&m_message[40], rBuf, rLen);

    return 0;
}

int CReadf::OneStep()
{
    ACTDBG_INFO("I'm CReadf onestep.\n")
    return 0;
}

int CReadf::Reset()
{
    ACTDBG_INFO("I'm CReadf reset.\n")
    return 0;
}

int CReadf::HandleData(unsigned char *&pPacket)
{
    ACTDBG_INFO("I'm CReadf HandleData.\n")
    return 0;
}

int CReadf::processData()
{
    ACTDBG_INFO("I'm CReadf processData.\n")
    return 0;
}