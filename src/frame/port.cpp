#include "../include/port.h"
#include "../include/debug.h"

CPort::CPort()
{
    m_iModID = g_cDebug.AddModule(PORT_MODNAME);
    m_iCnt = 0;
}

CPort::~CPort()
{

}

int CPort::GetCnt()
{
    return m_iCnt;
}

int CPort::Connect(CPort *pIn)
{
    
}