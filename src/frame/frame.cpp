#include <stdio.h>
#include <dlfcn.h>
#include "../include/frame.h"
#include "../include/debug.h"
#include "../include/node.h"

CActFrame::CActFrame()
{
    m_pConfig = NULL;
}

CActFrame::~CActFrame()
{
    if(m_pConfig) delete m_pConfig;
}

int CActFrame::InitFrame()
{
    m_pConfig = new CActConfig;

    m_iModID = g_cDebug.AddModule(FRAME_MODNAME);

    ACTDBG_INFO("InitFrame: Test messages.");
    ACTDBG_DEBUG("InitFrame: debug.");
    ACTDBG_INFO("InitFrame: info.");
    ACTDBG_WARNING("InitFrame: warning.");
    ACTDBG_ERROR("InitFrame: error.");
    ACTDBG_FATAL("InitFrame: fatal.");


    char *error;
//    class CActNode *(*GetNode)();
    int (*GetNode)();

    void *pdlHandle = dlopen("libs/libnode_in.so", RTLD_LAZY);
    if((error = dlerror())!=NULL)
    {
        ACTDBG_ERROR("dlopen fail: %s", error)
    }
    GetNode = dlsym(pdlHandle, "ActNewNode");
    if((error = dlerror())!=NULL)
    {
        ACTDBG_ERROR("ActNewNode fail: %s", error)
    }
    GetNode();
//    m_pNode = (*GetNode)();
//    m_pNode->PrintMe();

    return 0;
}