#include <stdio.h>
#include <dlfcn.h>
#include <unistd.h>
#include "../include/frame.h"
#include "../include/debug.h"
#include "../include/node.h"
#include "../include/config.h"
#include "../include/PacketMachine.h"

PROCITEM g_sProcList_f[] = 
{
    {DATA_CMDTYPE_DATA,&CActFrame::HandleData, 0},
    {DATA_CMDTYPE_CONFIG, &CActFrame::AppConfig, 0},
    {0}
};

CActFrame::CActFrame()
{
    m_iState = ACTFRM_STATE_IDLE;
    m_pdlHandle = NULL;
    for(int i=0;i<ACTFRM_MAXCPORT;i++)
    {
        m_iClientPort[i] = 0;
        m_iClientIp[i] = 0;
    }
    m_iNodesNum_row = -1;
    m_iNodesNum_col = -1;
    m_iNodetype = 0;
}

CActFrame::~CActFrame()
{
}

int CActFrame::InitFrame()
{
    char strTemp[CONFIGITEM_DATALEN];
    m_iModID = g_cDebug.AddModule(FRAME_MODNAME);

    ACTDBG_INFO("InitFrame: Test messages:");
    ACTDBG_DEBUG("InitFrame: debug.");
    ACTDBG_INFO("InitFrame: info.");
    ACTDBG_WARNING("InitFrame: warning.");
    ACTDBG_ERROR("InitFrame: error.");
    ACTDBG_FATAL("InitFrame: fatal.");

/*
    memset(strTemp, 0 , sizeof(strTemp));
    if(g_cConfig.GetConfigItem(ACTFRM_NODEPATHNAME, FRAME_MODNAME, strTemp) == 0)
    {
        memcpy(m_strNodePathName, strTemp, CONFIGITEM_DATALEN);
        if(AttachNode(m_strNodePathName))
        {
            ACTDBG_WARNING("InitFrame: AttachNode <%s> failed.", m_strNodePathName)
            DetachNode();
            return -1;
        }
    }   
    else
    {
        ACTDBG_WARNING("InitFrame: Cannot find NodePathName config.")
    }
*/ 
    InitCmds();
    m_cManager.InitProcs();
    m_cManager.Start(ACTMAN_MANAGERIP, ACTMAN_MANAGERPORT);


    return 0;
}

int CActFrame::UninitFrame()
{
    DetachNode();
    return 0;
}

int CActFrame::AttachNode(char *strNodePathName)
{
    char *error;
    ActNodeCreater *GetNode;

    if(strNodePathName == NULL)
    {
        ACTDBG_ERROR("AttachNde: Missing Node path name.")
        return -1;
    }

    if(m_pNode)
    {
        ACTDBG_WARNING("AttachNode: Existing Node Object destroyed before create a new one.")
        DetachNode();
    }

    if(m_pdlHandle)
    {
        dlclose(m_pdlHandle);
        m_pdlHandle = NULL;
        ACTDBG_WARNING("AttachNode: Existing Dll Handle closed before opening a new one.")
    }


    void *m_pdlHandle = dlopen(strNodePathName, RTLD_LAZY);
    if((error = dlerror())!=NULL)
    {
        ACTDBG_ERROR("AttachNode: dlopen fail: %s", error)
        return -1;
    }
    GetNode = (ActNodeCreater *)dlsym(m_pdlHandle, "ActNewNode");
    if((error = dlerror())!=NULL)
    {
        ACTDBG_ERROR("AttachNode: cannot find symbol \"ActNewNode\" in %s", strNodePathName)
        dlclose(m_pdlHandle);
        return -1;
    }

    m_pNode = (*GetNode)();
    if(m_pNode == NULL)
    {
        ACTDBG_ERROR("AttachNode: New node fail.")
        dlclose(m_pdlHandle);
        return -1;
    }

    ACTDBG_INFO("AttachNode: New node <%s> attached successfully.", strNodePathName)
    return 0;
}

int CActFrame::DetachNode()
{
    char *error;
    ActNodeRemover *DelNode;

    if(m_pNode == NULL)
    {
        ACTDBG_WARNING("DetachNode: No node to be detached.")
        return 0;
    }

    if(m_pdlHandle == NULL)
    {
        ACTDBG_ERROR("DetachNode: Dll handler missing.")
        return -1;
    }

    DelNode = (ActNodeRemover *)dlsym(m_pdlHandle, "ActDeleteNode");
    if((error = dlerror())!=NULL)
    {
        ACTDBG_ERROR("DetachNode: cannot find symbol \"ActDeleteNode\"")
        return -1;
    }

    (*DelNode)(m_pNode);
    m_pNode = NULL;

    dlclose(m_pdlHandle);
    m_pdlHandle = NULL;

    ACTDBG_INFO("DetachNode: detach current node successfully.")
    return 0;
}


int CActFrame::Run()
{
    int i;
    if(m_iState == ACTFRM_STATE_RUN)
    {
        ACTDBG_WARNING("Run: Frame is running, do nothing.")
//        return 0;
    }
    if(m_iState == ACTFRM_STATE_PAUSE)
    {
        ACTDBG_WARNING("Run: Frame is pause, stop and restart.")
        Stop();
    }
    m_iState = ACTFRM_STATE_RUN;
 
    while(m_iState != ACTFRM_STATE_IDLE)
    {
//    ACTDBG_DEBUG("Run: <%d>", m_iState)
        if(m_cManager.m_TellMan[0] == 1)
        {
            m_cManager.m_TellMan[0] = 0;
            int status;
            int *pstatus = &status;
            char istate[12];
            ACTDBG_INFO("Frame: Send status to Manager.")
            PDATA_PACKET_HEADER pHeader;
            pHeader=(struct tag_DataPacketHeader *)malloc(sizeof(struct tag_DataPacketHeader));
            //Setting up packet header////
            pHeader->iType = DATA_CMDTYPE_NODESTATE;
            pHeader->iConn = ACTMAN_MANAGERPORT;
            pHeader->iSync = DATA_PACKETSYNC;

        //////////////////////////////

            if(m_iNodetype >0)
            {
                ACTDBG_INFO("Frame: Node have configed.Send status to Manager.")
                status = m_iNodetype;
                memcpy(&istate[0], pstatus, 4);
                status = m_iNodesNum_row;
                memcpy(&istate[4], pstatus, 4);
                status = m_iNodesNum_col;
                memcpy(&istate[8], pstatus, 4);
                ACTDBG_INFO("Frame: NodeNum= %d%d, Type= %d.",m_iNodesNum_row,m_iNodesNum_col,m_iNodetype)
                PacketData(pHeader, istate, 12); 
            }
            else
            {
                ACTDBG_INFO("Frame: Node have not configed.Send status to Manager.")
                status = m_iNodetype;
                memcpy(&istate[0], pstatus, 4);
                PacketData(pHeader, istate, 12);
            }
            free(pHeader);
        }
        sleep(1);
    }
    return 0;
}

int CActFrame::Pause(int iGo)
{
    if(iGo)
        m_iState = ACTFRM_STATE_RUN;
    else
        m_iState = ACTFRM_STATE_PAUSE;
    return 0;
}

int CActFrame::Stop()
{
    ACTDBG_INFO("Stop: stop frame.")
    m_iState = ACTFRM_STATE_IDLE;

    return 0;
}


void *CActFrame::ThreadFunc(void *arg)
{
    class CActFrame *pThis = (class CActFrame *)arg;
    pThis->MainThread();
    return NULL;
}

void *CActFrame::MainThread()
{
    ACTDBG_DEBUG("MainTread counter start.")
    for(int i=0; i<3; i++)
    {
        ACTDBG_DEBUG("Tread counter: %d", i)
        sleep(2);
    }
    ACTDBG_DEBUG("MainTread counter stop.")
   return NULL;
}


int CActFrame::OnCmdExit(PCOMMAND pCmd, char *strRet, void *pContext)
{
    CActFrame *pThis = (CActFrame *)pContext;

    sprintf(strRet, "Main frame exit.");
    pThis->Stop();
    pThis->UninitFrame();
    return 0;
}

int CActFrame::PacketData(void *pHeader, char *Data, int dLen)
{
    unsigned char message[ACTTCPCLI_MAXDATALEN];
    if((Data == NULL) || (dLen <=0) || (dLen > (ACTTCPCLI_MAXDATALEN - 40)))
    {
        ACTDBG_ERROR("Frame: Packet Invalid Parameters.")
        return -1;
    }
    ACTDBG_INFO("PacketData:%s",Data)
    memcpy(&message[0], pHeader, 40);
    memcpy(&message[40], Data, dLen);
    m_cManager.Sendmess(message, sizeof(message), 0);
    return 0;
}

int CActFrame::AppConfig(unsigned char *&pPacket, unsigned char *&pQuery, void *pContext, int iConn)
{
    if(!pContext) return -1;
    CActFrame *pThis = (CActFrame *)pContext;
    return pThis->onAppConfig(pPacket, pQuery);
}


int CActFrame::onAppConfig(unsigned char *&pPacket, unsigned char *&pQuery)
{
    char node_path[CONFIGITEM_DATALEN];

    memset(node_path, 0 , sizeof(node_path));
    m_iNodesNum_row = *(int *)(pPacket + 40);
    m_iNodesNum_col = *(int *)(pPacket + 44);
    m_iNodetype = *(int *)(pPacket + 48);

    ACTDBG_INFO("Frame read m_iNodetype:%d",m_iNodetype)
    char strTemp[CONFIGITEM_DATALEN];
    memset(strTemp, 0 , sizeof(strTemp));
    char *pCh_NodeType = new char[sizeof(m_iNodetype)];
 
    sprintf(pCh_NodeType,"%d",m_iNodetype);
    if(g_cConfig.GetConfigItem(pCh_NodeType, FRAME_MODNAME, strTemp) == 0)
    {
        memcpy(node_path, strTemp, sizeof(strTemp)); 
        ACTDBG_INFO("Frame read .so Path:%s",node_path) 
    }
    else
    {
        ACTDBG_ERROR("Frame cnt't read .so Path.")
        return -1;
    }
    delete pCh_NodeType;

    if(AttachNode(node_path))
    {
        ACTDBG_WARNING("InitFrame: AttachNode <%s> failed.", node_path)
        m_iNodesNum_row = -1;
        m_iNodesNum_col = -1;
        DetachNode();
        return -1;
    } 

    m_pNode->Init(pPacket);
    return 0;
}

int CActFrame::HandleData(unsigned char *&pPacket, unsigned char *&pQuery, void *pContext, int iConn)
{
    if(!pContext) return -1;
    CActFrame *pThis = (CActFrame *)pContext;
    return pThis->onHandleData(pPacket, pQuery);
}

int CActFrame::onHandleData(unsigned char *&pPacket, unsigned char *&pQuery)
{
    m_pNode->HandleData(pPacket);
    return 0;
}

int CActFrame::InitCmds()
{
    int j=0;
    while(g_sProcList_f[j].iCmdType)
    {
        g_sProcList_f[j].pContext = this;
        m_cManager.Addprocs(&g_sProcList_f[j]);
        j++;
    }
    return 0;
}