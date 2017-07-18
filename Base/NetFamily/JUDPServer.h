#ifndef _JUDP_SERVER_H_
#define _JUDP_SERVER_H_

#include "JBaseDef.h"
#include "JUDPBaseDef.h"
#include "JG_Memory.h"

class JUDPServer
{
public:
    JUDPServer();
    virtual ~JUDPServer();

    BOOL Init();
    void UnInit();

    BOOL Listen(char* pszIP, int nPort);
    void Close();

    // return -1: error, 0: timeout, 1: success, -2: non-block && no data && success
    int  Recv(IJG_Buffer** ppiRetBuffer, sockaddr_in* pClientAddr, int* pnClientAddrSize);
    BOOL Send(char* pszSendBuf, size_t uSendSize, sockaddr_in* pClientAddr, int nClientAddrSize);

private:
    WSADATA     m_WSAData;
    BYTE        m_byLowByteVersion;
    BYTE        m_byHightByteVersion;
    int         m_nSocketFD;
    sockaddr_in m_ServerAddr;
    int         m_nPort;

    fd_set      m_ReadFDSet;
    char        m_iRecvBuffer[JUDP_MAX_DATA_SIZE];
};

#endif // _JUDP_SERVER_H_