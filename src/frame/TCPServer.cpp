#include <sys/types.h>
#include <sys/select.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#include "../include/TCPServer.h"

CTCPServer::CTCPServer()
{
    m_iPort = 0;
    m_iSocketFd = -1;
    m_ListenThread = 0;
    m_iState = 0;
    m_iConn = 0;
    for(int i=0; i<ACTTCPSVR_MAXCONN; i++)
        m_piConnFd[i] = -1;
    m_iModID = g_cDebug.AddModule(TCPSERVER_MODNAME);
}

CTCPServer::~CTCPServer()
{

}

int CTCPServer::Start(int iPort)
{
    if(m_iSocketFd >= 0)
    {
        ACTDBG_WARNING("Start: Server(%d) started, stop now.", m_iPort)
        Stop();
    }
    m_iPort = iPort;

    m_iSocketFd = socket(AF_INET, SOCK_STREAM, 0);
    if(m_iSocketFd < 0)
    {
        ACTDBG_ERROR("Start: Socket create fail.")
        return -1;
    }

    if(m_ListenThread)
    {
        ACTDBG_WARNING("Start: Listen Thread is Runing, do nothing here.")
        return 0;
    }

    if(pthread_create(&m_ListenThread, NULL, ListenThreadFunc, this))
    {
        ACTDBG_ERROR("Create ListenThread fail!")
        m_ListenThread = 0;
        return -1;
    }

    ACTDBG_INFO("Start: ListenThread started successfully.")
    return 0;
}

int CTCPServer::Stop()
{
    m_iState = 0;
    pthread_join(m_ListenThread, NULL);
    if(m_iSocketFd>0)
        close(m_iSocketFd);
    for(int i=0; i<ACTTCPSVR_MAXCONN; i++)
    {
        if(m_piConnFd[i] >0)
        {
            close(m_piConnFd[i]);
            m_piConnFd[i] = -1;
        }
    }
    return 0;
}

void *CTCPServer::ListenThreadFunc(void *arg)
{
    class CTCPServer *pThis = (class CTCPServer *)arg;
    pThis->ListenThread();
    return NULL;
}

void *CTCPServer::ListenThread()
{
    int i, j, iRv;
    m_iState = 1;

    struct sockaddr_in sa;
    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = INADDR_ANY;
    sa.sin_port = htons(m_iPort);

    if(bind(m_iSocketFd, (struct sockaddr *)&sa, sizeof(sa)) == -1)
    {
        ACTDBG_ERROR("ListenThread: Binding fail.")
        return NULL;
    }

    if(listen(m_iSocketFd, ACTTCPSVR_MAXCONN+2) == -1)
    {
        ACTDBG_ERROR("ListenThread: Listen fail.")
        return NULL;
    }

    fd_set fsRead;
    int iFdMax=0;
    struct timeval tvTimeOut;
    tvTimeOut.tv_sec = ACTTCPSVR_TIMEOUT_US / 1000000L;
    tvTimeOut.tv_usec = ACTTCPSVR_TIMEOUT_US % 1000000L;

    for(i=0; i<ACTTCPSVR_MAXCONN; i++)
    {
        if(m_piConnFd[i]>=0)
        {
            close(m_piConnFd[i]);
            m_piConnFd[i] = -1;
        }
    }

    while(m_iState)
    {
        iFdMax = 0;
        FD_ZERO(&fsRead);
        FD_SET(m_iSocketFd, &fsRead);
        iFdMax = m_iSocketFd;
        for(i=0; i<ACTTCPSVR_MAXCONN; i++)
        {
            if(m_piConnFd[i] == -1) continue;
            FD_SET(m_piConnFd[i], &fsRead);
            iFdMax = iFdMax>m_piConnFd[i]?iFdMax:m_piConnFd[i];
        }
        
        iRv = select(iFdMax+1, &fsRead, NULL, NULL, &tvTimeOut);
        if(iRv == -1)
        {
            ACTDBG_ERROR("ListenThread: Select Error<%d>.", iRv)
            continue;
        }
        if(FD_ISSET(m_iSocketFd, &fsRead))
        {
            int fd = accept(m_iSocketFd, 0, 0);
            if(fd == -1)
            {
                ACTDBG_ERROR("ListenThread: Accept Error<%d>.", fd)
                continue;
            }
            for(j=0; j<ACTTCPSVR_MAXCONN; j++)
            {
                if(m_piConnFd[j] == -1)
                {
                    ACTDBG_INFO("ListenThread: New Connection<%d>.", fd);
                    m_piConnFd[j] = fd;
                }
            }
            if(j == ACTTCPSVR_MAXCONN)
            {
                ACTDBG_WARNING("ListenThread: too many connections.")
                close(fd);
                continue;
            }
        }
        for(j=0; j<ACTTCPSVR_MAXCONN; j++)
        {
            if(m_piConnFd[j] == -1) continue;
            if(!FD_ISSET(m_piConnFd[j], &fsRead)) continue;
            unsigned char pucBuf[ACTTCPSVR_MAXDATALEN] = {0};
            iRv = recv(m_piConnFd[j], pucBuf, sizeof(pucBuf), 0);
            if(iRv > 0)
            {
                ProcessData(j, pucBuf, iRv);
            }
            else if(iRv == 0)
            {
                ACTDBG_WARNING("ListenThread: Connection <%d> closed.", m_piConnFd[j])
                close(m_piConnFd[j]);
                m_piConnFd[j] = -1;
            }
            else
            {
                ACTDBG_ERROR("ListenThread: Recv error<%d>.", iRv)
            }
        }
    }
    ACTDBG_INFO("ListenThread exit.");
}

int CTCPServer::ProcessData(int iConn, unsigned char *pBuf, int iLen)
{
    if((iConn<0) || (iConn>=ACTTCPSVR_MAXCONN) || (pBuf == NULL) || (iLen<0) || (iLen > ACTTCPSVR_MAXDATALEN))
    {
        ACTDBG_ERROR("ProcessData: Invalid Params.")
        return -1;
    }
    ACTDBG_INFO("ProcessData: Conn<%d>, Len<%d>", iConn, iLen)
    return 0;
}

int CTCPServer::Send(int iConn, unsigned char *pBuf, int iLen)
{
    int iRv;

    if((iConn<0) || (iConn>=ACTTCPSVR_MAXCONN) || (pBuf == NULL) || (iLen<0) || (iLen > ACTTCPSVR_MAXSENDLEN))
    {
        ACTDBG_ERROR("Send: Invalid Params.")
        return -1;
    }

    if(m_piConnFd[iConn]<0) 
    {
        ACTDBG_ERROR("Send: Bad Connection <%d>%d.", iConn, m_piConnFd[iConn])
        return -1;
    }
    int iLeft = iLen;
    int iSend = iLeft>ACTTCPSVR_MAXDATALEN?ACTTCPSVR_MAXDATALEN:iLeft;
    while(iLeft>0)
    {
        iRv = send(m_piConnFd[iConn], pBuf+(iLen-iLeft), iSend, 0);
        if(iRv = -1)
        {
            ACTDBG_ERROR("Send: error<%d>.", iRv)
            break;
        }
        iLeft -= iSend;
    }
    return 0;
}

