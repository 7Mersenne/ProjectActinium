#ifndef ACTINIUM_READF_H_600c4497-aacc-43fb-a4ec-d3131c6e0ec1
#define ACTINIUM_READF_H_600c4497-aacc-43fb-a4ec-d3131c6e0ec1

#include "interface.h"
#include "nodescenter.h"
#include "define.h"
#include "PacketMachine.h"
#include "../include/node.h"


#define FOUNCTIONAL_MODNAME "Founctional_Node"

class CActFounc:public CActNode ,public CPackMach 
{
public:
    CActFounc();
    ~CActFounc();

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