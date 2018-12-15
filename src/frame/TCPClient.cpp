#include <sys/types.h>
#include <sys/select.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <signal.h>


#include "../include/TCPClient.h"

extern int errno;

CTCPClient::CTCPClient()
{
	for(int i=0; i<ACTTCPCLI_MAXCONN; i++)
	{
		m_socket_fd[i] = -1;
		m_Port[i] = 0;
		m_Ip[i] = 0;
		m_State[i] = 0;
		m_SendThread[i] = 0;
		m_RunState[i] = 0;
		m_TellMan[i] = 0;
	}
	m_iModID = g_cDebug.AddModule(TCPCLIENT_MODNAME);
}

CTCPClient::~CTCPClient()
{

}

int CTCPClient::Start(char* mip, int mport)
{   
	struct sigaction sa;
    sa.sa_handler = SIG_IGN;
    sigaction( SIGPIPE, &sa, 0 );
	int i=0;
	for(i=0; i<ACTTCPCLI_MAXCONN; i++)
	{
		if(m_Port[i] == mport)
		{
			ACTDBG_ERROR("Client Start: Client(%d) started",mport)
			return -1;
		}
	}
	for(i=0; i<ACTTCPCLI_MAXCONN; i++)
	{
		if(m_socket_fd[i] == -1)
		{
			m_Port[i] = mport;
	        m_Ip[i] = mip;
			break;
		}
	}
/*	if(m_socket_fd >= 0)
	{
		ACTDBG_WARNING("Start: Client(%d) started, stop now.", m_Port)
			Stop();
	}
*/
//    m_Port = mport;
//	m_Ip = mip;

	if((m_socket_fd[i] = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		ACTDBG_ERROR("Start:Client(%d) NO.%d Socket create fail.", m_Port[i],i)
			return -1;
	}

	if(m_SendThread[i])
	{
		ACTDBG_WARNING("Start:Send Thread is Runing,do nothing here.")
			return 0;
	}
	struct tag_tcpconnect *Client;
    Client=(struct tag_tcpconnect *)malloc(sizeof(struct tag_tcpconnect));
	Client->pTh = this;
	Client->iConn = i;
	if (pthread_create(&m_SendThread[i], NULL, ConnectFunc, (void*)Client))
	{
		ACTDBG_ERROR("Create ConnectThread %d fail!",i)
		m_SendThread[i] = 0;
		return -1;
	}

	ACTDBG_INFO("Start: SendThread %d started successfully.",i)
	
	return 0;
}

int CTCPClient::Stop(int iConn)
{
	m_State[iConn] = 0;
	pthread_join(m_SendThread[iConn], NULL);
	if (m_socket_fd[iConn] > 0)
		close(m_socket_fd[iConn]);
	return 0;
}

void *CTCPClient::ConnectFunc(void *arg)
{
	struct tag_tcpconnect *temp;
    temp = (struct tag_tcpconnect *)arg;
	class CTCPClient *pThis = (class CTCPClient *)(temp->pTh);
	pThis->ClientConnect(temp->iConn);
	return NULL;
}

void *CTCPClient::ClientConnect(int iConn)
{   int iRv;
//    m_State = 1;
	if( (m_socket_fd[iConn] = socket(AF_INET,SOCK_STREAM,0)) < 0 ) 
	{
        ACTDBG_ERROR("create socket error: %s(errno:%d))",strerror(errno),errno)
        return NULL; 
    }
 
    memset(&server_addr,0,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(m_Port[iConn]);
 
    if( inet_pton(AF_INET,m_Ip[iConn],&server_addr.sin_addr) <=0 ) 
	{
        ACTDBG_ERROR("inet_pton error for %s",m_Ip[iConn])
        return NULL;       
    }
 
    while(connect(m_socket_fd[iConn],(struct sockaddr*)&server_addr,sizeof(server_addr))<0) 
	{
        ACTDBG_ERROR("Clientconnect error: %s(errno: %d)",strerror(errno),errno)
		ACTDBG_INFO("%d will reconnect in 2s.",m_Port[iConn]);
		sleep(2);       
    }
	OnConnect(iConn);

	ACTDBG_INFO("ClientThread: ClientConnected<%d>.", m_Port[iConn]);
	m_State[iConn] = 1;
	m_RunState[iConn] = 1;

	fd_set fsRead;
	int iFdMax=0;
	struct timeval tvTimeOut;
    tvTimeOut.tv_sec = ACTTCPCLI_TIMEOUT_US / 1000000L;
    tvTimeOut.tv_usec = ACTTCPCLI_TIMEOUT_US % 1000000L;

	while(m_State[iConn])
	{
		iFdMax = 0;
		FD_ZERO(&fsRead);
		FD_SET(m_socket_fd[iConn], &fsRead);
		iFdMax = m_socket_fd[iConn];
		m_State[iConn] = 1;

	    iRv = select(iFdMax+1, &fsRead,NULL, NULL, &tvTimeOut);
		if(iRv == -1)
		{
			ACTDBG_ERROR("CilentThread: Select Error<%s>.", strerror(errno));
			m_State[iConn] = 0;
			return NULL;
		}

        unsigned char rebuf[ACTTCPCLI_MAXDATALEN] = {0};
//		FD_ISSET(m_socket_fd,&fsRead);
	    iRv = recv(m_socket_fd[iConn],rebuf,sizeof(rebuf),0);
		if(iRv > 0)
		{
		    ACTDBG_DEBUG("ClientThread: Recv <%d> [%s]",m_Port[iConn],(char *)rebuf)
			processData(m_Port[iConn],rebuf, iRv);
		}
		if(iRv == 0)
		{
			if( (m_socket_fd[iConn] = socket(AF_INET,SOCK_STREAM,0)) < 0 ) 
		    {
       	       ACTDBG_ERROR("create socket error: %s(errno:%d))",strerror(errno),errno)
       	       return NULL; 
   	        }
		    m_RunState[iConn] = 0;
            memset(&server_addr,0,sizeof(server_addr));
            server_addr.sin_family = AF_INET;
            server_addr.sin_port = htons(m_Port[iConn]);
 
            if( inet_pton(AF_INET,m_Ip[iConn],&server_addr.sin_addr) <=0 ) 
    	    {
                ACTDBG_ERROR("inet_pton error for %s",m_Ip[iConn])
                return NULL;       
            }
      
            while(!m_RunState[iConn]) 
	        {
				if(connect(m_socket_fd[iConn],(struct sockaddr*)&server_addr,sizeof(server_addr))>=0)
				{
					m_RunState[iConn] = 1;
					OnConnect(iConn);
					break;
				}
                ACTDBG_ERROR("Clientconnect error: %s(errno: %d)",strerror(errno),errno)
	        	ACTDBG_INFO("%d will reconnect in 2s.",m_Port[iConn]);
	        	sleep(2);       
            }
		}
		
	}
	ACTDBG_INFO("ClientThread exit.");
	return 0;
}

int CTCPClient::processData(int iConn, unsigned char *rebuf, int ilen)
{
	if((rebuf == NULL) || ilen >ACTTCPCLI_MAXDATALEN)
	{
		ACTDBG_ERROR("client processdata: Invalid Params.")
		return -1;
	}
	ACTDBG_INFO("client processdata: Len<%d>",ilen)
	return 0;
}

int CTCPClient::Sendmess(unsigned char *Pbuf, int ilen, int iConn)
{
	int iRv;

	if((Pbuf == NULL) || (ilen<0) || (ilen > ACTTCPCLI_MAXDATALEN))
	{
		ACTDBG_ERROR("Client send: Invaild Params.")
		return -1;
	}

	if(m_socket_fd[iConn]<0)
	{
		ACTDBG_ERROR("Client send: Bad Connection %d.", m_Port[iConn])
		return -1;
	}
    if(ilen > 0)
	{
		iRv = send(m_socket_fd[iConn],Pbuf,ilen,0);
		if(iRv <= 0)
		{
			ACTDBG_ERROR("Client send: error<%s>.", strerror(errno))
			return -1;
		}
	}
	ACTDBG_DEBUG("Client send: <%d> [%s]", iRv, (char *)Pbuf)
	return 0;
}

int CTCPClient::OnConnect(int iConn)
{
	if(m_State[iConn] == 1) return 0;
	else return -1;
}