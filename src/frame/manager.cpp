#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <arpa/inet.h>
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

PROCITEM g_sProcList_m[] = 
{
    {DATA_CMDTYPE_NODESTATE, &CActMan::NodeConfig, 0},
    {DATA_CMDTYPE_NODEREST, &CActMan::ResetNodeState, 0},
    {0}
};

CActMan::CActMan()
{
    m_iUsersPort = ACTMAN_DEFUSERPORT;
    m_iNodesPort = ACTMAN_DEFNODEPORT;
    m_iState = ACTMAN_STATE_IDLE;
    m_iNodesLoc_row = 0;
    m_iNodesLoc_col = 0;
    m_iListSize = PACKMACH_PROCLIST_INITSIZE;
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
    if(g_cConfig.GetConfigItem(ACTMAN_NODESROWNUM, MANAGER_MODNAME, strTemp) == 0)
    {
        m_iNodesLoc_row = atoi(strTemp);
    }
    if(g_cConfig.GetConfigItem(ACTMAN_NODESCOLNUM, MANAGER_MODNAME, strTemp) == 0)
    {
        m_iNodesLoc_col = atoi(strTemp);
    }

    InitCmds();

    // init Nodes Center
    m_cNodesCenter.Start(m_iNodesPort);
    m_cNodesCenter.InitTopo(m_iNodesLoc_row, m_iNodesLoc_col);
    m_cNodesCenter.InitProcs();
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
    int j=0;
    while(g_sProcList_m[j].iCmdType)
    {
        g_sProcList_m[j].pContext = this;
        m_cNodesCenter.Addprocs(&g_sProcList_m[j]);
        j++;
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

int CActMan::NodeConfig(unsigned char *&pPacket, unsigned char *&pQuery, void *pContext, int iConn)
{
    if(!pContext) return -1;
    CActMan *pThis = (CActMan *)pContext;
    return pThis->onNodeConfig(pPacket, pQuery, iConn);
}

int CActMan::onNodeConfig(unsigned char *&pPacket, unsigned char *&pQuery, int iConn)
{   
    int temp_row,temp_col;
    char strTemp[CONFIGITEM_DATALEN];
    PDATA_PACKET_HEADER pHeader;
    pHeader = (tag_DataPacketHeader *)pPacket;
    PSEAT pSeat;
    if(pHeader->iSync != DATA_PACKETSYNC)
    {
        ACTDBG_ERROR("Manager: NodeConfig Invalid request.")
        pHeader = NULL;
        delete pHeader;
        pSeat = NULL;
        delete pSeat;
        return -1;
    }
    int *pData = reinterpret_cast<int*>(pPacket + 40);
    if(*pData > 0)
    {
        for(int m=1; m<=NODETYPE_NUM; m++)
        {
            if(*pData == m)
            {
                int i,j;
                i = *(int *)(pPacket + 44);
                j = *(int *)(pPacket + 48);
                pSeat = &m_cNodesCenter.m_pSeats[i*m_iNodesLoc_row+j];
                pSeat->iState = NODESCENTER_SEATSTATE_OCCUPIED;
                pSeat->sInfo.iType = *pData;
                pSeat->sInfo.iID = iConn;
                ACTDBG_WARNING("Manager: Node %d%d existing configuration",i,j)
                pHeader = NULL;
                delete pHeader;
                pSeat = NULL;
                delete pSeat;
                return 0;
            }
        }
        ACTDBG_ERROR("Manager: No such NodeType!")
        pSeat = NULL;
        delete pSeat;
        pHeader = NULL;
        delete pHeader;
        return -1;
    }
    else if(*pData == 0)
    {
        ACTDBG_WARNING("Manager: Node %d is not configured",pHeader->iConn)
        char *iNodeConfig;
        int l,k;
        for(int l=0; l<m_iNodesLoc_row; l++)
        {
            for(int k=0; k<m_iNodesLoc_col; k++)
            {
                pSeat = &m_cNodesCenter.m_pSeats[l*m_iNodesLoc_row+k];
                if(pSeat->iState == NODESCENTER_SEATSTATE_AVAILABLE)
                {
                    iNodeConfig= new char[sizeof(ACTMAN_NODECONFIG) + sizeof(l) + sizeof(k)];
                    sprintf(iNodeConfig, "%d%d%s", l, k, ACTMAN_NODECONFIG);
                    temp_row = l;
                    temp_col = k;
                    l = m_iNodesLoc_row;
                    k = m_iNodesLoc_col;
                }
            }
        }
        int Portnum = 0;
        int *pPortnum = &Portnum;
        int Nodeport = 0;
        int *Node = &Nodeport;
        int NodeType = 0;
        int *pNodeType = &NodeType;
        unsigned char mess[ACTMAN_BUFSIZE];
        int j=0;
        ACTDBG_INFO("manager read config:%s",iNodeConfig)
        if(g_cConfig.GetConfigItem(ACTMAN_NODETYPE, iNodeConfig, strTemp) == 0)
        {
            NodeType = atoi(strTemp);
            ACTDBG_INFO("manager read config: %d",NodeType)
            memcpy(&mess[j],pNodeType,sizeof(NodeType));
            j = j + sizeof(NodeType);
        }
        if(g_cConfig.GetConfigItem(ACTMAN_NODEPORTNUM, iNodeConfig, strTemp) == 0)
        {
            Portnum = atoi(strTemp);
            ACTDBG_INFO("Portnum:%d",Portnum)
            memcpy(&mess[j],pPortnum,sizeof(Portnum));
            j = j + sizeof(Portnum);
        }
        
        for(int i=1; i<=Portnum; i++)
        {
            char *PortDir= new char[sizeof(ACTMAN_NODEDIRECTION) + sizeof(i)];
            sprintf(PortDir, "%d%s", i, ACTMAN_NODEDIRECTION);
            
            if(g_cConfig.GetConfigItem(PortDir, iNodeConfig, strTemp) == 0)
            {
                Nodeport = atoi(strTemp);
                ACTDBG_INFO("manager read config: %d",*Node)
            }
    
            delete PortDir;
            memcpy(&mess[j],Node,sizeof(Nodeport));
            j = j + sizeof(Nodeport);


            char *Portname= new char[sizeof(ACTMAN_NODEPORT) + sizeof(i)];
            sprintf(Portname, "%d%s", i, ACTMAN_NODEPORT);
            
            if(g_cConfig.GetConfigItem(Portname, iNodeConfig, strTemp) == 0)
            {
                Nodeport = atoi(strTemp);
                ACTDBG_INFO("manager read config: %d",*Node)
            }
            delete Portname;
            memcpy(&mess[j],Node,sizeof(Nodeport));
            j = j + sizeof(Nodeport);
            
            unsigned long dwAddr;
            unsigned long *pdwAddr = &dwAddr;
            char *Ipname= new char[sizeof(ACTMAN_NODEIP) + sizeof(i)];
            sprintf(Ipname, "%d%s", i, ACTMAN_NODEIP);
            if(g_cConfig.GetConfigItem(Ipname, iNodeConfig, strTemp) == 0)
            {
                dwAddr = inet_addr(strTemp);
                ACTDBG_INFO("manager read config:%s,%ld",strTemp,dwAddr)
                memcpy(&mess[j],pdwAddr,sizeof(dwAddr));
                j = j + sizeof(dwAddr);
            }
            delete Ipname;
        }
        if(g_cConfig.GetConfigItem(ACTMAN_NODESERVERPORT, iNodeConfig, strTemp) == 0)
        {
            Nodeport = atoi(strTemp);
            ACTDBG_INFO("manager read config: %d",Nodeport)
            memcpy(&mess[j],Node,sizeof(Nodeport));
            j = j + sizeof(Nodeport);
        }
        delete iNodeConfig;

        unsigned char message[ACTMAN_BUFSIZE];

        pHeader->iSync = DATA_PACKETSYNC;
        pHeader->iFrom = m_iNodesPort;
        pHeader->iTo = iConn;
        pHeader->iConn = iConn;
        pHeader->iType = DATA_CMDTYPE_CONFIG;
        memcpy(&message[0], pHeader, 40);
        Portnum = l;
        memcpy(&message[40], pPortnum, 4);
        Portnum = k;
        ACTDBG_INFO("manager Node_loc: %d %d",temp_row ,temp_col)
        memcpy(&message[44], pPortnum, 4);
        memcpy(&message[48], mess, j);

        if(m_cNodesCenter.Send(iConn,message, sizeof(message)) == 0)
        {
            ACTDBG_INFO("Manager send config to node %d<%s> successfull.",pHeader->iConn,message)
            pSeat = &m_cNodesCenter.m_pSeats[temp_row*m_iNodesLoc_row+temp_col];
            pSeat->iState = NODESCENTER_SEATSTATE_OCCUPIED;
            pSeat->sInfo.iID = iConn;
            pSeat->sInfo.iType = NodeType;
            ACTDBG_INFO("Manager config node Nodes_iState=%d,Nodes_iID=%d",pSeat->iState,pSeat->sInfo.iID)

        }
        else 
        {
            ACTDBG_ERROR("Manager send config to node %d fail.",pHeader->iConn)
            pHeader = NULL;
            delete pHeader;
            pSeat = NULL;
            delete pSeat;
            return -1;
        }
    }

    else ACTDBG_ERROR("Manager: Node %d error status",pHeader->iConn)
    pHeader = NULL;
    delete pHeader;
    pSeat = NULL;
    delete pSeat;
    return 0;
}

int CActMan::ResetNodeState(unsigned char *&pPacket, unsigned char *&pQuery, void *pContext, int iConn)
{
    if(!pContext) return -1;
    CActMan *pThis = (CActMan *)pContext;
    return pThis->onResetNodeState(pPacket, pQuery, iConn);
}

int CActMan::onResetNodeState(unsigned char *&pPacket, unsigned char *&pQuery, int iConn)
{
    PSEAT pSeat;
    for(int l=0; l<m_iNodesLoc_row; l++)
    {
        for(int k=0; k<m_iNodesLoc_col; k++)
        {
            pSeat = &m_cNodesCenter.m_pSeats[l*m_iNodesLoc_row+k];
            if(pSeat->sInfo.iID == iConn)
            {
                pSeat->iState = NODESCENTER_SEATSTATE_AVAILABLE;
                pSeat->sInfo.iID = -1;
                pSeat->sInfo.iType = -1;
                ACTDBG_WARNING("Manager: ResetNodeState %d%d",l, k)
                ACTDBG_INFO("Manager: iState=%d,sInfo.iID=%d,sInfo.iType=%d",pSeat->iState, pSeat->sInfo.iID, pSeat->sInfo.iType)
            }
        }
    }
    pSeat = NULL;
    delete pSeat;
    return 0;
}
