#include <stdio.h>
#include <dlfcn.h>
#include <unistd.h>
#include "../include/frame.h"
#include "../include/debug.h"
#include "../include/node.h"

CActFrame::CActFrame()
{
    m_iState = ACTFRM_STATE_IDLE;
}

CActFrame::~CActFrame()
{
}

int CActFrame::InitFrame()
{
    m_iModID = g_cDebug.AddModule(FRAME_MODNAME);

    ACTDBG_INFO("InitFrame: Test messages.");
    ACTDBG_DEBUG("InitFrame: debug.");
    ACTDBG_INFO("InitFrame: info.");
    ACTDBG_WARNING("InitFrame: warning.");
    ACTDBG_ERROR("InitFrame: error.");
    ACTDBG_FATAL("InitFrame: fatal.");


    char *error;
//    int (*GetNode)();
    ActNodeCreater *GetNode;
    ActNodeRemover *DelNode;

    void *pdlHandle = dlopen("libs/libnode_in.so", RTLD_LAZY);
    if((error = dlerror())!=NULL)
    {
        ACTDBG_ERROR("dlopen fail: %s", error)
        return 0;
    }
    GetNode = (ActNodeCreater *)dlsym(pdlHandle, "ActNewNode");
    if((error = dlerror())!=NULL)
    {
        ACTDBG_ERROR("ActNewNode fail: %s", error)
        return 0;
    }
    DelNode = (ActNodeRemover *)dlsym(pdlHandle, "ActDeleteNode");
    if((error = dlerror())!=NULL)
    {
        ACTDBG_ERROR("ActDeleteNode fail: %s", error)
        return 0;
    }
    m_pNode = (*GetNode)();
    m_pNode->PrintMe();
    (*DelNode)(m_pNode);

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
    if(pthread_create(&m_MainThread, NULL, ThreadFunc, this))
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
