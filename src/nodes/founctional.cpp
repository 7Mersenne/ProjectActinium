#include <stdio.h>
#include <dlfcn.h>
#include <unistd.h>
#include "../include/founctional.h"
#include "../include/debug.h"


class CActNode *ActNewNode()
{
    class CActFounc *pNode = new class CActFounc();
    return (class CActNode *)pNode;
}

int ActDeleteNode(class CActNode *pNode)
{
    delete (class CActFounc *)pNode;
    return 0;
}

PROCITEM g_sProcList_foun[] = 
{
    {DATA_CMDTYPE_DATA,&CActFounc::ToHandleData, 0},
    {0}
};


CActFounc::CActFounc():CActNode(),CPackMach()
{
    m_iModID = g_cDebug.AddModule(FOUNCTIONAL_MODNAME);
    memset(m_iClientIp, 0, sizeof(m_iClientIp));
    memset(m_iClientPort, 0, sizeof(m_iClientPort));
    memset(m_iClientType, 0, sizeof(m_iClientType));
    m_iServerPort = -1;
    m_iNodetype = -1;
    m_iClientCon = -1;
}

CActFounc::~CActFounc()
{

}

int CActFounc::Init(unsigned char *&pPacket)
{
    if(*(int *)(pPacket) != DATA_PACKETSYNC)
    {
        ACTDBG_ERROR("Founctioanl Node: AppConfig Invalid Parameters.")
        return -1;
    }
    
    int con = 48;
    m_iNodetype = *(int *)(pPacket + con);
    con = con + 4;
    m_iClientCon = *(int *)(pPacket + con);
    con = con + 4;
 

 //   pClientPort = new CInterface [m_iClientCon-1];//bug
    
    //terminate called after throwing an instance of 'std::bad_alloc'
    //what():  std::bad_alloc
    //Aborted (core dumped)
    
     
    
    char *IP = 0;
    for(int i=0; i<m_iClientCon; i++)
    {
        m_iClientType[i] = *(int *)(pPacket + con);
        con = con + 4;

        m_iClientPort[i] = *(int *)(pPacket + con);
        con = con + 4;

        m_iClientIp[i] = *(unsigned long *)(pPacket + con);
        con = con + 8;
        struct in_addr inAddr1; 
        inAddr1.s_addr = m_iClientIp[i];
        IP = inet_ntoa(inAddr1);
        ACTDBG_INFO("m_iClientType:%d;m_iClientPort:%d;m_iClientIp:%s",m_iClientType[i],m_iClientPort[i],IP)
    }

    m_iServerPort = *(int *)(pPacket + con);
    ACTDBG_INFO("m_iServerPort:%d",m_iServerPort)
    if(m_cServer.Start(m_iServerPort) == 0)
    {
        int j=0;
        while(g_sProcList_foun[j].iCmdType)
        {
            g_sProcList_foun[j].pContext = this;
            m_cServer.Addprocs(&g_sProcList_foun[j]);
            j++;
        }
        m_cServer.InitProcs();
        ACTDBG_INFO("Founctioanl Node Server:%d started.",m_iServerPort)
    }
    else 
    {
        ACTDBG_ERROR("Founctioanl Node Server:%d start fail.",m_iServerPort)
        return -1;
    }

    struct in_addr inAddr;  
    for(int j=0; j<m_iClientCon; j++)
    {   
        inAddr.s_addr = m_iClientIp[j];
        IP = inet_ntoa(inAddr);     
        if(ClientPort.Start(IP,m_iClientPort[j]) == 0)
        {
            ACTDBG_INFO("Founctioanl Node Client:%d started.",m_iClientPort[j])
        }
        else 
        {
            ACTDBG_ERROR("Founctioanl Node Client:%d start fail.",m_iClientPort[j])
            return -1;
        }
    }

    return 0;
}

int CActFounc::ToHandleData(unsigned char *&pPacket, unsigned char *&pQuery, void *pContext, int iConn)
{
    if(!pContext) return -1;
    CActFounc *pThis = (CActFounc *)pContext;
    return pThis->HandleData(pPacket);
}

int CActFounc::HandleData(unsigned char *&pPacket)
{
    for(int i=0; i<DATA_OUTPUT_MAXCON; i++)
    {
        if(m_iClientType[i] != CONNECTDEST_RIGHT) continue;
        int len;
        len = 40 + *(int *)(pPacket+ 36);
        ClientPort.Sendmess(pPacket,len,i);
        i = DATA_OUTPUT_MAXCON;
    }
    return 0;
}

int CActFounc::OneStep()
{
    ACTDBG_INFO("I'm CActFounc onestep.\n")
    return 0;
}

int CActFounc::Reset()
{
    ACTDBG_INFO("I'm CActFounc reset.\n")
    return 0;
}
