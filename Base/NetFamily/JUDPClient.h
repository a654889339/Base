#ifndef _JUDP_CLIENT_H_
#define _JUDP_CLIENT_H_

#include "JBaseDef.h"
#include "JUDPBaseDef.h"
#include "JG_Memory.h"

class JUDPClient
{
public:
    JUDPClient();
    virtual ~JUDPClient();

    BOOL Init();
    void UnInit();

    BOOL Connect(char* pszIP, int nPort, char* pszLocalIP, int nLocalPort);
    void Close();

    BOOL Recv(IJG_Buffer* pszRecvBuf, size_t* puDataSize);
    BOOL Send(IJG_Buffer* pszSendBuf, size_t uSendSize);

private:
    WSADATA     m_WSAData;
    BYTE        m_byLowByteVersion;
    BYTE        m_byHightByteVersion;
    int         m_nSocketFD;
    sockaddr_in m_ServerAddr;
    sockaddr_in m_LocalAddr;
    int         m_nPort;

    IJG_Buffer  m_iRecvBuffer[JUDP_MAX_DATA_SIZE];
};

#endif //_JUDP_CLIENT_H_