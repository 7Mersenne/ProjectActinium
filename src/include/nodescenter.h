#ifndef ACTINIUM_NODESCENTER_H_bfa8fba1_7562_4930_a32c_b3223c2bfc58
#define ACTINIUM_NODESCENTER_H_bfa8fba1_7562_4930_a32c_b3223c2bfc58

#include "TCPServer.h"

extern "C"{

#define NODESCENTER_MODNAME "NodesCenter"

#define NODESCENTER_MAXPAYLOAD 4096
#define DATAPACKETSYNC 0xAC89CF98
#define NODESCENTER_MAXQUEUE 256

#define NODESCENTER_MAXBUFSIZE 65536
#define NODESCENTER_MINBUFSIZE 1024

#define NODESCENTER_READY 0
#define NODESCENTER_SYNC  1
#define NODESCENTER_HEADER 2
#define NODESCENTER_PAYLOAD 3


typedef struct tag_DataPacketHeader
{
    int iSync;          //fixed to DATAPACKETSYNC
    int iFrom;          //Sender ID
    int iTo;            //Receiver ID
    int iType;          //Packet Type
    int iVersion;       //Data Version
    int iConn;          //Connection No.
    int iSerial;        //Serial Counter
    int iState;         //Packet State
    int iPayloadSize;   //Bytes followed this header
}DATA_PACKET_HEADER, *PDATA_PACKET_HEADER;


class CNodesCenter :public CTCPServer
{
public:
    CNodesCenter();
    ~CNodesCenter();

    int Init();
    int ProcessData(int iConn, unsigned char *pBuf, int iLen);
    int OnConnected(int iConn);

    int ClearQueue();
    int Push(int iConn);
    int Pop(unsigned char *&pPacket);

    int MakeBuf(int iConn, int iNeed);
    int InitCmds();


protected:
    unsigned char *m_pucPacketQueue[NODESCENTER_MAXQUEUE];
    int m_iQHead;
    int m_iQTail;
    
    unsigned char *m_pucPacketBuf[ACTTCPSVR_MAXCONN];
    int m_iBufSize[ACTTCPSVR_MAXCONN];
    int m_iBytesInBuf[ACTTCPSVR_MAXCONN];
    int m_iFlag[ACTTCPSVR_MAXCONN];

private:
    int m_iModID;
};














}
#endif