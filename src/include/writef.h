#ifndef ACTINIUM_WRITEF_H_b5a8ae8a-ab17-490b-b9e6-8b7897c0fe23
#define ACTINIUM_WRITEF_H_b5a8ae8a-ab17-490b-b9e6-8b7897c0fe23

#include "TCPServer.h"
#include "define.h"
#include "PacketMachine.h"
#include "../include/node.h"
extern "C"{
#define WRITEF_MODNAME "Writef"

class CWritef :public CActNode ,public CTCPServer ,public CPackMach 
{
public:
    CWritef();
    ~CWritef();

    int Init(unsigned char *&pPacket);
    int OneStep();
    int Reset();
    int HandleData(unsigned char *&pPacket);
    int WriteFile(unsigned char *pBuf);
    int ProcessData(int iConn, unsigned char *pBuf, int iLen);
    int OnConnected(int iConn);
    int OnDisconnected(int iConn);

protected:
    int m_iServerPort;
    const char *fpath;

private:
    int m_iModID;

};

class CActNode *ActNewNode();

int ActDeleteNode(class CActNode *pNode);
}
#endif