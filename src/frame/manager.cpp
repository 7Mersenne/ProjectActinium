#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <unistd.h>
#include "../include/manager.h"
#include "../include/debug.h"
#include "../include/node.h"
#include "../include/config.h"

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

    // init Nodes Center
    m_cNodesCenter.Start(m_iNodesPort);
    m_cUsersCenter.Start(m_iUsersPort);

    return 0;
}

int CActMan::Run()
{
    m_iState = ACTMAN_STATE_RUN;
    ACTDBG_INFO("Run: Manager Running.")
    pthread_join(m_cNodesCenter.m_ListenThread, NULL);
    pthread_join(m_cUsersCenter.m_ListenThread, NULL);
    m_iState = ACTMAN_STATE_STOP;
    ACTDBG_INFO("Run: Manager Stoped.")
    return 0;
}