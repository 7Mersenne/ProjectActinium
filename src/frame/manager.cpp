#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include "../include/manager.h"
#include "../include/debug.h"
#include "../include/node.h"
#include "../include/config.h"
using namespace std;

CMDITEM g_sCmdList[] = 
{
    {"exit", "exit this console connection.", 0, &CActMan::CmdExit, 0},
    {"show_users", "list all users.", 0, &CActMan::CmdShowUsers, 0},
    {"show_nodes", "list all nodes.", 0, &CActMan::CmdShowNodes, 0},
    {"shutdown", "shutdown manager.", 0, &CActMan::CmdShutDown, 0},
    {0}
};

CActMan::CActMan()
{
    m_iUsersPort = ACTMAN_DEFUSERPORT;
    m_iNodesPort = ACTMAN_DEFNODEPORT;
    m_iState = ACTMAN_STATE_IDLE;
}

CActMan::~CActMan()
{
}

int CActMan::InitMan()
{
    char strTemp[CONFIGITEM_DATALEN];
    m_iModID = g_cDebug.AddModule(MANAGER_MODNAME);

    ACTDBG_INFO("InitMan: Test messages:");
    ACTDBG_DEBUG("InitMan: debug.");
    ACTDBG_INFO("InitMan: info.");
    ACTDBG_WARNING("InitMan: warning.");
    ACTDBG_ERROR("InitMan: error.");
    ACTDBG_FATAL("InitMan: fatal.");

    memset(strTemp, 0 , sizeof(strTemp));
    if(g_cConfig.GetConfigItem(ACTMAN_NODEPORTITEM, MANAGER_MODNAME, strTemp) == 0)
    {
        m_iNodesPort = atoi(strTemp);
    } 
    if(g_cConfig.GetConfigItem(ACTMAN_USERPORTITEM, MANAGER_MODNAME, strTemp) == 0)
    {
        m_iUsersPort = atoi(strTemp);
    } 

    InitCmds();

    // init Nodes Center
    m_cNodesCenter.Start(m_iNodesPort);
    m_cUsersCenter.Start(m_iUsersPort);

    return 0;
}

int CActMan::InitCmds()
{
    int i=0;
    
    while(g_sCmdList[i].strName[0])
    {
        g_sCmdList[i].pContext = this;
        m_cUsersCenter.AddCmd(&g_sCmdList[i]);
        i++;
    }
    return 0;
}

int CActMan::Run()
{
    m_iState = ACTMAN_STATE_RUN;
    ACTDBG_INFO("Run: Manager Running.")
    while(m_iState == ACTMAN_STATE_RUN)
    {
        sleep(1);
    }
    pthread_join(m_cNodesCenter.m_ListenThread, NULL);
    pthread_join(m_cUsersCenter.m_ListenThread, NULL);
    m_iState = ACTMAN_STATE_STOP;
    ACTDBG_INFO("Run: Manager Stoped.")
    return 0;
}

int CActMan::Stop()
{
    char strMsg[] = "Manager shutdown...";
    m_cNodesCenter.Stop();
    m_cUsersCenter.SendToAll((unsigned char *)strMsg, sizeof(strMsg));
    m_cUsersCenter.Stop();
    m_iState = ACTMAN_STATE_STOP;
    return 0;
}

int CActMan::CmdExit(PCOMMAND pCmd, char *strRet, void *pContext)
{
    CActMan *pThis = (CActMan *)pContext;
    pThis->onCmdExit(pCmd, strRet);
    return 0;
}

int CActMan::onCmdExit(PCOMMAND pCmd, char *strRet)
{
    strcpy(strRet, "Bye!");
    m_cUsersCenter.StopConnection(pCmd->iConn);
    return 0;
}

int CActMan::CmdShowUsers(PCOMMAND pCmd, char *strRet, void *pContext)
{
    CActMan *pThis = (CActMan *)pContext;
    pThis->onCmdShowUsers(pCmd, strRet);
    return 0;
}

int CActMan::onCmdShowUsers(PCOMMAND pCmd, char *strRet)
{
    int iRv, i, iCur;
    int iUsers=0;

    iCur = 0;
    for(i=0; i<ACTTCPSVR_MAXCONN; i++)
    {
        if(m_cUsersCenter.m_piConnFd[i]>0)
        {
            iRv = snprintf(strRet+iCur, ACTCON_CMDRETMESGLEN-iCur, "#%d\t%d\t%d\n\r", i, m_cUsersCenter.m_piConnFd[i], m_cUsersCenter.m_iConnState[i]);
            iCur += iRv;
            iUsers ++;
        }
    }

    iRv = snprintf(strRet+iCur, ACTCON_CMDRETMESGLEN-iCur, "%d user(s) active.\n\r", iUsers);
    return 0;
}

int CActMan::CmdShowNodes(PCOMMAND pCmd, char *strRet, void *pContext)
{
    CActMan *pThis = (CActMan *)pContext;
    pThis->onCmdShowNodes(pCmd, strRet);
    return 0;
}

int CActMan::onCmdShowNodes(PCOMMAND pCmd, char *strRet)
{
    int iRv, i, iCur;
    int iNodes=0;

    iCur = 0;
    for(i=0; i<ACTTCPSVR_MAXCONN; i++)
    {
        if(m_cNodesCenter.m_piConnFd[i]>0)
        {
            iRv = snprintf(strRet+iCur, ACTCON_CMDRETMESGLEN-iCur, "#%d\t%d\t%d\n\r", i, m_cNodesCenter.m_piConnFd[i], m_cNodesCenter.m_iConnState[i]);
            iCur += iRv;
            iNodes ++;
        }
    }

    iRv = snprintf(strRet+iCur, ACTCON_CMDRETMESGLEN-iCur, "%d Node(s) active.\n\r", iNodes);
    return 0;
}

int CActMan::CmdShutDown(PCOMMAND pCmd, char *strRet, void *pContext)
{
    CActMan *pThis = (CActMan *)pContext;
    pThis->onCmdShutDown(pCmd, strRet);
    return 0;
}

int CActMan::onCmdShutDown(PCOMMAND pCmd, char *strRet)
{
    Stop();
    return 0;
}

int CActMan::NodeConfig(unsigned char *&pPacket, unsigned char *&pQuery, void *pContext)
{
    if(!pContext) return -1;
    CActMan *pThis = (CActMan *)pContext;
    return pThis->onNodeConfig(pPacket, pQuery);
}

int CActMan::onNodeConfig(unsigned char *&pPacket, unsigned char *&pQuery)
{
    char strTemp[CONFIGITEM_DATALEN];
    PDATA_PACKET_HEADER pHeader;
    pHeader = (tag_DataPacketHeader *)pPacket;
    if(pHeader->iSync != DATA_PACKETSYNC)
    {
        ACTDBG_ERROR("Manager: NodeConfig Invalid request.")
        return -1;
    }
    int *pData = reinterpret_cast<int*>(pPacket + 40);
    if(*pData == 1)
    {
        ACTDBG_WARNING("Manager: Node %d existing configuration",pHeader->iConn)
        return 0;
    }
    else if(*pData == 0)
    {
        ACTDBG_WARNING("Manager: Node %d is not configured",pHeader->iConn)
        int Portnum = 0;
        if(g_cConfig.GetConfigItem(ACTMAN_NODEPORTNUM, ACTMAN_NODECONFIG, strTemp) == 0)
        {
            Portnum = atoi(strTemp);
        }
        int Nodeport = 0;
        int *Node = &Nodeport;
        unsigned char *mess[ACTMAN_BUFSIZE];
        for(int i=1; i<=Portnum; i++)
        {
            char *temp;
            temp = ACTMAN_NODEPORT;
            if(g_cConfig.GetConfigItem(temp += char(i), ACTMAN_NODECONFIG, strTemp) == 0)
            {
                Nodeport = atoi(strTemp);
            }
            memcpy(mess,Node,sizeof(Nodeport));
            *mess = *mess + sizeof(Nodeport);
            temp = ACTMAN_NODEIP;
            if(g_cConfig.GetConfigItem(temp += char(i), ACTMAN_NODECONFIG, strTemp) == 0)
            {
                memcpy(mess,strTemp,sizeof(strTemp));
                *mess = *mess + sizeof(strTemp);
            }
            temp = ACTMAN_NODESERVERPORT;
            if(g_cConfig.GetConfigItem(temp, ACTMAN_NODECONFIG, strTemp) == 0)
            {
                Nodeport = atoi(strTemp);
            }
            memcpy(mess,Node,sizeof(Nodeport));
        }
        unsigned char message[ACTMAN_BUFSIZE];
        memcpy(&message[0], &pHeader, 40);
        memcpy(&message[40], mess, sizeof(mess));
        if(m_cNodesCenter.Send(pHeader->iConn,message, sizeof(message)) == 0)
        {
            ACTDBG_INFO("Manager send config to node %d successfull.",pHeader->iConn)
        }
        else 
        {
            ACTDBG_ERROR("Manager send config to node %d fail.",pHeader->iConn)
            return -1;
        }
    }
    return 0;
}

