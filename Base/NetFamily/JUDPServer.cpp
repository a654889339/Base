#include "JUDPServer.h"

JUDPServer::JUDPServer()
{
    m_byLowByteVersion   = 1;
    m_byHightByteVersion = 1;
    m_nConnectionCount   = 0;
}

JUDPServer::~JUDPServer()
{
    WSACleanup();
}

BOOL JUDPServer::Init()
{
    BOOL bResult  = false;
    BOOL bRetCode = false;

    bResult = true;
//Exit0:
    return bResult;
}

void JUDPServer::UnInit()
{
}

BOOL JUDPServer::Listen(char* pszIP, int nPort)
{
    BOOL  bResult  = false;
    BOOL  bRetCode = false;
    int   nRetCode = 0;
    WORD  wVersion = MAKEWORD(m_byHightByteVersion, m_byLowByteVersion);

    nRetCode = WSAStartup(wVersion, &m_WSAData);
    JGLOG_PROCESS_ERROR(nRetCode == 0);

    nRetCode = LOBYTE(m_WSAData.wVersion);
    JGLOG_PROCESS_ERROR(nRetCode == m_byLowByteVersion);

    nRetCode = HIBYTE(m_WSAData.wVersion);
    JGLOG_PROCESS_ERROR(nRetCode == m_byHightByteVersion);

    m_nSocketFD = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    JGLOG_PROCESS_ERROR(m_nSocketFD != INVALID_SOCKET);

#ifdef WIN32
    memset(&m_ServerAddr, 0, sizeof(m_ServerAddr));
#else
    bzero(&m_ServerAddr, sizeof(m_ServerAddr));
#endif

    m_ServerAddr.sin_family           = AF_INET;
    m_ServerAddr.sin_addr.S_un.S_addr = inet_addr(pszIP);
    m_ServerAddr.sin_port             = htons(nPort);

    nRetCode = bind(m_nSocketFD, (sockaddr*)&m_ServerAddr, sizeof(m_ServerAddr));
    JGLOG_PROCESS_ERROR(nRetCode != SOCKET_ERROR);

    bResult = true;
Exit0:
    if (!bResult)
    {
        Close();
    }
    return bResult;
}

void JUDPServer::Close()
{
    closesocket(m_nSocketFD);
}

int JUDPServer::Recv(IJG_Buffer** ppiRetBuffer, sockaddr_in* pClientAddr, int* pnClientAddrSize)
{
    int         nResult  = -1;
    int         nRetCode = 0;
    IJG_Buffer* piBuffer = NULL;
    char*       pRecv    = NULL;
    timeval     TimeOut  = {0, 0};

    JGLOG_PROCESS_ERROR(ppiRetBuffer);
    JGLOG_PROCESS_ERROR(pClientAddr);
    JGLOG_PROCESS_ERROR(pnClientAddrSize);

    *ppiRetBuffer = NULL;

    FD_ZERO(&m_ReadFDSet);
    FD_SET(m_nSocketFD, &m_ReadFDSet);

    nRetCode = select(m_nSocketFD + 1, &m_ReadFDSet, NULL, NULL, &TimeOut);
    if (nRetCode == 0)
    {
        nResult = -2;
        goto Exit0;
    }
    else if (nRetCode < 0)
    {
        nResult = 0;
        goto Exit0;
    }

    nRetCode = recvfrom(m_nSocketFD, m_iRecvBuffer, JUDP_MAX_DATA_SIZE, 0, (sockaddr*)pClientAddr, pnClientAddrSize);
    JGLOG_PROCESS_ERROR(nRetCode != -1);

    piBuffer = JG_MemoryCreateBuffer(nRetCode);
    JGLOG_PROCESS_ERROR(piBuffer);

    pRecv = (char*)piBuffer->GetData();
    JGLOG_PROCESS_ERROR(pRecv);

    memcpy(pRecv, m_iRecvBuffer, nRetCode);

    *ppiRetBuffer = piBuffer;

    (*ppiRetBuffer)->AddRef();

    nResult = 1;
Exit0:
    JG_COM_RELEASE(piBuffer);
    return nResult;
}

BOOL JUDPServer::Send(IJG_Buffer* piBuffer, sockaddr_in* pClientAddr, int nClientAddrSize)
{
    BOOL bResult  = false;
    int  nRetCode = 0;

    JGLOG_PROCESS_ERROR(piBuffer);
    JGLOG_PROCESS_ERROR(pClientAddr);

    nRetCode = sendto(m_nSocketFD, (char *)piBuffer->GetData(), piBuffer->GetSize(), 0, (sockaddr*)pClientAddr, nClientAddrSize);
    JG_PROCESS_ERROR(nRetCode == piBuffer->GetSize());

    bResult = true;
Exit0:
    return bResult;
}