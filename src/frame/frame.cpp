#include <stdio.h>
#include <dlfcn.h>
#include <unistd.h>
#include "../include/frame.h"
#include "../include/debug.h"
#include "../include/node.h"
#include "../include/config.h"

CActFrame::CActFrame()
{
    m_iState = ACTFRM_STATE_IDLE;
    m_pdlHandle = NULL;
    for(int i=0;i<ACTFRM_MAXCPORT;i++)
    {
        m_iClientPort[i] = 0;
        m_iClientIp[i] = 0;
    }
    m_iClientCon = 0;
    m_iServerPort = 0;
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

    m_cManager[0].Start(ACTMAN_MANAGERIP, ACTMAN_MANAGERPORT);


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
        return 0;
    }
    if(m_iState == ACTFRM_STATE_PAUSE)
    {
        ACTDBG_WARNING("Run: Frame is pause, stop and restart.")
        Stop();
    }
    m_iState = ACTFRM_STATE_RUN;

    if(m_cManager[0].OnConnect() == 0)
    {
        char status;
        char *ps = &status;
        ACTDBG_INFO("Frame: Send status to Manager.")
        PDATA_PACKET_HEADER pHeader;
        //Setting up packet header////
        pHeader->iType = DATA_CMDTYPE_NODESTATE;
        pHeader->iConn = ACTMAN_MANAGERPORT;
        pHeader->iSync = DATA_PACKETSYNC;

        //////////////////////////////

        if(m_iClientCon >=1 )
        {
            status = 1;
            for(int i=1; i<=m_iClientCon; i++)
            {
                char *IP = 0;
                *IP = m_iClientIp[i];
                if(m_cManager[i].Start(IP,m_iClientPort[i]) == 0)
                {
                    ACTDBG_INFO("Frame Client:%d started.",m_iClientPort[i])
                }
                else 
                {
                    ACTDBG_ERROR("Frame Client:%d start fail.",m_iClientPort[i])
                    return -1;
                }
            }
            if(m_cNode.Start(m_iServerPort) == 0)
            {
                ACTDBG_INFO("Frame Server:%d started.",m_iServerPort)
            }
            else 
            {
                ACTDBG_ERROR("Frame Server:%d start fail.",m_iServerPort)
                return -1;
            }
        }
        else status = 0;
        PacketData(pHeader, ps, sizeof(status));

    }
/*    if(pthread_create(&m_MainThread, NULL, ThreadFunc, this))
    {
        ACTDBG_FATAL("Create MainThread fail, exit!")
        m_iState = ACTFRM_STATE_IDLE;
        return -1;
    }
    ACTDBG_DEBUG("Frame counter start.")
    for(i=0; i<10; i++)
    {
        ACTDBG_DEBUG("Main counter: %d", i)
        sleep(1);
    }
    ACTDBG_DEBUG("Frame counter stop.")
*/

    while(m_iState != ACTFRM_STATE_IDLE)
    {
//        ACTDBG_DEBUG("Run: <%d>", m_iState)
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
    if((Data == NULL) || (dLen <=0) || (dLen > (ACTTCPCLI_MAXDATALEN) - 40))
    {
        ACTDBG_ERROR("Frame: Packet Invalid Parameters.")
        return -1;
    }
    memcpy(&message[0], &pHeader, 40);
    memcpy(&message[40], &Data, sizeof(Data));
    m_cManager[0].Sendmess(message, sizeof(message));

    return 0;
}

int CActFrame::AppConfig(unsigned char *&pPacket, unsigned char *&pQuery, void *pContext)
{
    if(!pContext) return -1;
    CActFrame *pThis = (CActFrame *)pContext;
    return pThis->onAppConfig(pPacket, pQuery);
}

int CActFrame::onAppConfig(unsigned char *&pPacket, unsigned char *&pQuery)
{
    if(*(int *)(pPacket) != DATA_PACKETSYNC)
    {
        ACTDBG_ERROR("Frame: AppConfig Invalid Parameters.")
        return -1;
    }
    m_iClientCon = *(int *)(pPacket + 40);
    int *pData = reinterpret_cast<int*>(pPacket+44);
    
    char *IP = 0;
    for(int i=1; i<=m_iClientCon; i++)
    {
        m_iClientPort[i] = *(int *)(pData);
        pData = pData + 4;

        m_iClientIp[i] = *(char *)(pData);
        pData = pData + 1;
        *IP = m_iClientIp[i];

        if(m_cManager[i].Start(IP,m_iClientPort[i]) == 0)
        {
            ACTDBG_INFO("Frame Client:%d started.",m_iClientPort[i])
        }
        else 
        {
            ACTDBG_ERROR("Frame Client:%d start fail.",m_iClientPort[i])
            return -1;
        }
    }

    m_iServerPort = *(int *)(pData);
    if(m_cNode.Start(m_iServerPort) == 0)
    {
        ACTDBG_INFO("Frame Server:%d started.",m_iServerPort)
    }
    else 
    {
        ACTDBG_ERROR("Frame Server:%d start fail.",m_iServerPort)
        return -1;
    }

    return 0;
}