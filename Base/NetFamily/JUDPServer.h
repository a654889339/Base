#ifndef _JUDP_SERVER_H_
#define _JUDP_SERVER_H_

#include "JBaseDef.h"

#ifdef _WIN32
#include <WINSOCK2.H>
#pragma comment(lib, "WS2_32.lib")
#endif

class JUDPServer
{
public:
    JUDPServer();
    virtual ~JUDPServer();

    BOOL Init();
    void UnInit();

    BOOL Listen(char* pszIP, int nPort);
    void Close();

    int  Recv(char& szRecvBuf, size_t uRecvBufSize);

private:
    WSADATA     m_WSAData;
    BYTE        m_byLowByteVersion;
    BYTE        m_byHightByteVersion;
    SOCKET      m_Socket;
    sockaddr_in m_Addr;
    int         m_nPort;

    sockaddr_in m_RemoteAddr;
    int         m_nAddrSize;

    char        m_szSendBuffer[1024];
    size_t      m_uBufferSize;
};

#endif // _JUDP_SERVER_H_