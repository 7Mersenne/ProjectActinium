#ifndef ACTINIUM_READF_H_cfa3d6f2-b6fc-4ae6-bfe0-2a1efa1e028c
#define ACTINIUM_READF_H_cfa3d6f2-b6fc-4ae6-bfe0-2a1efa1e028c

#include "interface.h"
#include "nodescenter.h"
#include "PacketMachine.h"
#include "define.h"
#include "../include/node.h"


#define FORWARD_MODNAME "Forward_Node"

class CActForward:public CActNode ,public CPackMach 
{
public:
    CActForward();
    ~CActForward();

    int Init(unsigned char *&pPacket);
    int OneStep();
    int Reset();
    static int ToHandleData(unsigned char *&pPacket, unsigned char *&pQuery, void *pContext, int iConn);
    int HandleData(unsigned char *&pPacket);
private:
    CInterface ClientPort;
    CNodesCenter m_cServer; 
    
    unsigned long m_iClientIp[DATA_OUTPUT_MAXCON];
    int m_iClientPort[DATA_OUTPUT_MAXCON];
    int m_iClientType[DATA_OUTPUT_MAXCON];
    int m_iServerPort;
    int m_iClientCon;
    int m_iNodetype;
protected:
    int m_iModID;

};
extern "C"{
class CActNode *ActNewNode();

int ActDeleteNode(class CActNode *pNode);

}

#endif 