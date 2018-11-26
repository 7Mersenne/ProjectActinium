#ifndef ACTINIUM_TCPClient_H_b16609c5-14fe-40b2-a1fe-9f2b396ba212
#define ACTINIUM_TCPClient_H_b16609c5-14fe-40b2-a1fe-9f2b396ba212 



#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "../include/debug.h"

//extern "C"{

#define TCPCLIENT_MODNAME "TCPClient"
#define ACTTCPCLI_TIMEOUT_US 100000L
#define ACTTCPCLI_MAXDATALEN 1024
#define ACTTCPCLI_MAXCONN 8
typedef struct tag_tcpconnect
{
    void *pTh;
    int iConn; 
}TCPCONNECT, *PTCPCONNECT;

class CTCPClient 
{
public:
	CTCPClient();
	~CTCPClient();

	int Start(char* m_ip, int m_port);
	int Stop(int iConn);

	static void *ConnectFunc(void *arg);
	void *ClientConnect(int iConn);

	virtual int processData(int iConn, unsigned char *pbuf, int ilen);
	int Sendmess(unsigned char *pbuf, int ilen, int iConn);

	int OnConnect(int iConn);

private:
	int m_socket_fd[ACTTCPCLI_MAXCONN];
	int m_Port[ACTTCPCLI_MAXCONN];
	char* m_Ip[ACTTCPCLI_MAXCONN];
	char message[ACTTCPCLI_MAXDATALEN];
	struct sockaddr_in server_addr;
	int m_State[ACTTCPCLI_MAXCONN];

	pthread_t m_SendThread[ACTTCPCLI_MAXCONN];

protected:
	int m_iModID;
	
};

//}
#endif 

