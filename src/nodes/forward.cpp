#include <stdio.h>

#include "../include/forward.h"
#include "../include/debug.h"

class CActNode *ActNewNode()
{
    class CActForward *pNode = new class CActForward();
    return (class CActNode *)pNode;
}

int ActDeleteNode(class CActNode *pNode)
{
    delete (class CActForward *)pNode;
    return 0;
}

PROCITEM g_sProcList_for[] = 
{
    {DATA_CMDTYPE_DATA,&CActForward::ToHandleData, 0},
    {0}
};

CActForward::CActForward():CActNode(),CPackMach()
{
    m_iModID = g_cDebug.AddModule(FORWARD_MODNAME);
    memset(m_iClientIp, 0, sizeof(m_iClientIp));
    memset(m_iClientPort, 0, sizeof(m_iClientPort));
    memset(m_iClientType, 0, sizeof(m_iClientType));
    m_iServerPort = -1;
    m_iNodetype = -1;
    m_iClientCon = -1;
}

CActForward::~CActForward()
{

}

int CActForward::Init(unsigned char *&pPacket)
{
    if(*(int *)(pPacket) != DATA_PACKETSYNC)
    {
        ACTDBG_ERROR("forward Node: AppConfig Invalid Parameters.")
        return -1;
    }

    int con = 48;
    m_iNodetype = *(int *)(pPacket + con);
    con = con + 4;
    m_iClientCon = *(int *)(pPacket + con);
    con = con + 4;

    //pClientPort = new CInterface [m_iClientCon-1];//bug
    
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
    if(m_cServer.Start(m_iServerPort) == 0)
    {
        int j=0;
        while(g_sProcList_for[j].iCmdType)
        {
            g_sProcList_for[j].pContext = this;
            m_cServer.Addprocs(&g_sProcList_for[j]);
            j++;
        }
        m_cServer.InitProcs();
        ACTDBG_INFO("forward Node Server:%d started.",m_iServerPort)
    }
    else 
    {
        ACTDBG_ERROR("forward Node Server:%d start fail.",m_iServerPort)
        return -1;
    }

    struct in_addr inAddr;  
    for(int j=0; j<m_iClientCon; j++)
    {   
        inAddr.s_addr = m_iClientIp[j];
        IP = inet_ntoa(inAddr);     
        if(ClientPort.Start(IP,m_iClientPort[j]) == 0)
        {
            ACTDBG_INFO("forward Node Client:%d started.",m_iClientPort[j])
        }
        else 
        {
            ACTDBG_ERROR("forward Node Client:%d start fail.",m_iClientPort[j])
            return -1;
        }
    }
    return 0;
}

int CActForward::ToHandleData(unsigned char *&pPacket, unsigned char *&pQuery, void *pContext, int iConn)
{
    if(!pContext) return -1;
    CActForward *pThis = (CActForward *)pContext;
    return pThis->HandleData(pPacket);
}


int CActForward::HandleData(unsigned char *&pPacket)
{
    for(int i=0; i<DATA_OUTPUT_MAXCON; i++)
    {   
        int len;
        if(m_iClientType[i] != CONNECTDEST_RIGHT) continue;
        len = 40 + *(int *)(pPacket+ 36);
        ClientPort.Sendmess(pPacket,len,i);
        i = DATA_OUTPUT_MAXCON;
    }
    return 0;
}


int CActForward::OneStep()
{
    ACTDBG_INFO("I'm CActForward onestep.\n")
    return 0;
}

int CActForward::Reset()
{
    ACTDBG_INFO("I'm CActForward reset.\n")
    return 0;
}