#ifndef _JUDP_CLIENT_H_
#define _JUDP_CLIENT_H_

#include "JUDPBaseDef.h"
#include "JG_Memory.h"
#include "JUDPConnection.h"

class JUDPClient
{
public:
    JUDPClient();
    virtual ~JUDPClient();

    BOOL Init();
    void UnInit();

    void Activate();

    BOOL Connect(char* pszIP, int nPort, char* pszLocalIP, int nLocalPort);

    BOOL Send(BYTE* piBuffer, size_t uSize);

private:
    int  Recv(IJG_Buffer **ppiRetBuffer);
    void Close();

    void ProcessPackage();
    void ProcessConnection();

    void OnUDPReliable(BYTE* pbyData, size_t uSize);
    void OnUDPUnreliable(BYTE* pbyData, size_t uSize);

private:
    WSADATA        m_WSAData;
    BYTE           m_byLowByteVersion;
    BYTE           m_byHightByteVersion;
    int            m_nSocketFD;
    sockaddr_in    m_ServerAddr;
    sockaddr_in    m_LocalAddr;
    int            m_nPort;

    BOOL           m_bWorkFlag;
    char           m_iRecvBuffer[JUDP_MAX_DATA_SIZE];

private:
    typedef void (JUDPClient::*PROCESS_UDP_PROTOCOL_FUNC)(BYTE* pbyData, size_t uSize);
    PROCESS_UDP_PROTOCOL_FUNC m_ProcessUDPProtocolFunc[euptUDPProtocolEnd];
    size_t                    m_uUDPProtocolSize[euptUDPProtocolEnd];


    JUDPConnection m_Connection;
};

#endif //_JUDP_CLIENT_H_