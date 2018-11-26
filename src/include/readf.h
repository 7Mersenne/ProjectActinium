#ifndef ACTINIUM_READF_H_d9a8ee38-91c3-43d6-8167-c21b2f166098
#define ACTINIUM_READF_H_d9a8ee38-91c3-43d6-8167-c21b2f166098

#include "TCPClient.h"
#include "define.h"
#include "../include/node.h"

extern "C"{
#define READF_MODNAME "Readf"
#define READF_READMAXLEN (ACTTCPCLI_MAXDATALEN-40)


class CReadf :public CTCPClient,public CActNode
{
public:
    CReadf();
    ~CReadf();
    int Init(unsigned char *&pPacket);
    int OneStep();
    int Reset();
    int HandleData(unsigned char *&pPacket);
    int ReadFile();
    int Packet(char *rBuf, int rLen);
    int processData();

private:
    unsigned char m_message[ACTTCPCLI_MAXDATALEN];
    int m_iSerialCon;
    const char *fpath;
    int m_iSendPort;
	char* m_iSendIp;

protected:
    int m_iModID;
};

class CActNode *ActNewNode();

int ActDeleteNode(class CActNode *pNode);

}
#endif 