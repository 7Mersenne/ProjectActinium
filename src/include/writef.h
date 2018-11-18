#ifndef ACTINIUM_WRITEF_H_b5a8ae8a-ab17-490b-b9e6-8b7897c0fe23
#define ACTINIUM_WRITEF_H_b5a8ae8a-ab17-490b-b9e6-8b7897c0fe23

#include "TCPServer.h"
#include "define.h"

#define WRITEF_MODNAME "Writef"

class CWritef :public CTCPServer
{
public:
    CWritef();
    ~CWritef();

    int InitWritef(unsigned char *pPacket);
    int WriteFile(unsigned char *pBuf);
    int ProcessData(int iConn, unsigned char *pBuf, int iLen);

protected:
    int m_iServerPort;
    const char *fpath;

private:
    int m_iModID;

};

#endif