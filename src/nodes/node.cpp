#include <string.h>
#include <stdio.h>
#include "../include/node.h"

CActNode::CActNode()
{

}

int CActNode::GetInfo(ACTNODEINFO &sInfo)
{
    memcpy(&sInfo, &m_sInfo, sizeof(ACTNODEINFO));
    return 0;
}
