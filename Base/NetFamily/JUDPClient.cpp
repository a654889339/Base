#include "JUDPClient.h"

JUDPClient::JUDPClient()
{

}

JUDPClient::~JUDPClient()
{

}

BOOL JUDPClient::Init()
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    
    bResult = true;
//Exit0:
    return bResult;
}

void JUDPClient::UnInit()
{
}

BOOL JUDPClient::Connect(char* pszIP, int nPort, char* pszLocalIP,int nLocalPort)
{
    BOOL bResult  = false;
    int  nRetCode = 0;

    m_nSocketFD = socket(AF_INET, SOCK_DGRAM, 0);
    JGLOG_PROCESS_ERROR(m_nSocketFD == INVALID_SOCKET);

#ifdef WIN32
    memset(&m_ServerAddr, 0, sizeof(m_ServerAddr));
#else
    bzero(&m_ServerAddr, sizeof(m_ServerAddr));
#endif

    m_ServerAddr.sin_family           = AF_INET;
    m_ServerAddr.sin_addr.s_addr      = inet_addr(pszIP);
    m_ServerAddr.sin_port             = htons(nPort);

    m_LocalAddr.sin_family            = AF_INET;
    m_LocalAddr.sin_addr.s_addr       = inet_addr(pszLocalIP);
    m_LocalAddr.sin_port              = htons(nLocalPort);

    nRetCode = bind(m_nSocketFD, (sockaddr*)&m_LocalAddr, sizeof(m_LocalAddr));
    JGLOG_PROCESS_ERROR(nRetCode != -1);

    nRetCode = connect(m_nSocketFD, (sockaddr*)&m_ServerAddr, sizeof(m_ServerAddr));
    JGLOG_PROCESS_ERROR(nRetCode != -1);

    bResult = true;
Exit0:
    return bResult;
}

void JUDPClient::Close()
{
    closesocket(m_nSocketFD);
}

BOOL JUDPClient::Recv(IJG_Buffer* pszRecvBuf)
{
    BOOL bResult  = false;
    int  nRetCode = 0;

    nRetCode = recv(m_nSocketFD, (char*)m_iRecvBuffer, JUDP_MAX_DATA_SIZE, 0);
    JGLOG_PROCESS_ERROR(nRetCode != -1);

    memcpy(pszRecvBuf, m_iRecvBuffer, nRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL JUDPClient::Send(IJG_Buffer* pszSendBuf, size_t uSendSize)
{
    BOOL bResult  = false;
    int  nRetCode = 0;

    JGLOG_PROCESS_ERROR(pszSendBuf);

    nRetCode = send(m_nSocketFD, (char*)pszSendBuf, uSendSize, 0);

    bResult = true;
Exit0:
    return bResult;
}