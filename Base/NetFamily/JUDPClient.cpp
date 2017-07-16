#include "JUDPClient.h"

JUDPClient::JUDPClient()
{
    m_byLowByteVersion   = 1;
    m_byHightByteVersion = 1;
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
    LONG lRetCode = 1;

    WORD  wVersion = MAKEWORD(m_byHightByteVersion, m_byLowByteVersion);

    nRetCode = WSAStartup(wVersion, &m_WSAData);
    JGLOG_PROCESS_ERROR(nRetCode == 0);

    nRetCode = LOBYTE(m_WSAData.wVersion);
    JGLOG_PROCESS_ERROR(nRetCode == m_byLowByteVersion);

    nRetCode = HIBYTE(m_WSAData.wVersion);
    JGLOG_PROCESS_ERROR(nRetCode == m_byHightByteVersion);

    m_nSocketFD = socket(AF_INET, SOCK_DGRAM, 0);
    JGLOG_PROCESS_ERROR(m_nSocketFD != INVALID_SOCKET);

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

    nRetCode = ioctlsocket(m_nSocketFD, FIONBIO, (unsigned long *)&lRetCode);//设置成非阻塞模式。
    JGLOG_PROCESS_ERROR(nRetCode != SOCKET_ERROR)

    bResult = true;
Exit0:
    return bResult;
}

void JUDPClient::Close()
{
    closesocket(m_nSocketFD);
}

BOOL JUDPClient::Recv(IJG_Buffer* pszRecvBuf, size_t* puDataSize)
{
    BOOL bResult  = false;
    int  nRetCode = 0;

    JGLOG_PROCESS_ERROR(puDataSize);

    *puDataSize = 0;

    nRetCode = recv(m_nSocketFD, (char*)m_iRecvBuffer, JUDP_MAX_DATA_SIZE, 0);
    if (nRetCode == SOCKET_ERROR)
    {
        nRetCode = WSAGetLastError();
        JG_PROCESS_SUCCESS(nRetCode == WSAEWOULDBLOCK);

        JGLOG_PROCESS_ERROR(nRetCode == WSAETIMEDOUT);
        JGLOG_PROCESS_ERROR(nRetCode == WSAENETDOWN);

        JGLogPrintf(JGLOG_ERR, "[JUDPClient] Recv Error Code: %d\n", nRetCode);
        goto Exit0;
    }

    memcpy(pszRecvBuf, m_iRecvBuffer, nRetCode);
    *puDataSize = (size_t)nRetCode;

Exit1:
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