#include "JUDPServer.h"

JUDPServer::JUDPServer()
{
    m_byLowByteVersion   = 1;
    m_byHightByteVersion = 1;
    m_nAddrSize          = sizeof(m_RemoteAddr);
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
Exit0:
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
    DWORD dwVersion = MAKEWORD(m_byHightByteVersion, m_byLowByteVersion);

    nRetCode = WSAStartup(dwVersion, &m_WSAData);
    JG_PROCESS_ERROR(nRetCode == 0);

    nRetCode = LOBYTE(m_WSAData.wVersion);
    JG_PROCESS_ERROR(nRetCode == m_byLowByteVersion);

    nRetCode = HIBYTE(m_WSAData.wVersion);
    JG_PROCESS_ERROR(nRetCode == m_byHightByteVersion);

    m_Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_UDP);
    JG_PROCESS_ERROR(m_Socket != INVALID_SOCKET);

    m_Addr.sin_family = AF_INET;
    m_Addr.sin_port   = nPort;
    m_Addr.sin_addr.S_un.S_addr = INADDR_ANY;

    nRetCode = bind(m_Socket, (sockaddr*)&m_Addr, sizeof(m_Addr));
    if (nRetCode == SOCKET_ERROR)
    {
        closesocket(m_Socket);
        goto Exit0;
    }

    bResult = true;
Exit0:
    return bResult;
}

void JUDPServer::Close()
{
    closesocket(m_Socket);
}

int JUDPServer::Recv(char& szRecvBuf, size_t uRecvBufSize)
{
    int nRetCode = 0;

    nRetCode = recvfrom(m_Socket, &szRecvBuf, uRecvBufSize, 0, (sockaddr*)&m_RemoteAddr, &m_nAddrSize);

    return nRetCode;
}