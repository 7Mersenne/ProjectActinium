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

    if(m_pConfig)
    {
        m_pConfig->LoadConfigs();
    }
    else return -1;

    void *pdlHandle = dlopen("../libs/node_in.so", RTLD_LAZY);
    void (*GetNode)() = dlsym(pdlHandle, "ActNewNode");
    m_pNode = (class CActNode *)GetNode();
    m_pNode->PrintMe();

    return 0;
}