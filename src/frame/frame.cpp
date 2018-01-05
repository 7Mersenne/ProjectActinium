#include <stdio.h>
#include "../include/frame.h"
#include "../include/debug.h"

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
    return 0;
}