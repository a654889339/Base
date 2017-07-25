#ifndef _JUDP_CLIENT_H_
#define _JUDP_CLIENT_H_

#include "JUDPBaseDef.h"
#include "JG_Memory.h"
#include "JUDPConnection.h"
#include "JGS_Client_Protocol.h"

class JUDPClient
{
public:
    JUDPClient();
    virtual ~JUDPClient();

    BOOL Init();
    void UnInit();

    void Activate();

    BOOL Connect(char* pszIP, int nPort, char* pszLocalIP, int nLocalPort);

    BOOL Send(IJG_Buffer* piBuffer);

private:
    int  Recv(IJG_Buffer **ppiRetBuffer);
    void Close();

    void ProcessPackage();
    void ProcessSendPacket();
    void ProcessRecvPacket();

private:    // process udp header
    typedef void (JUDPClient::*PROCESS_UDP_PROTOCOL_FUNC)(BYTE* pbyData, size_t uSize);
    PROCESS_UDP_PROTOCOL_FUNC m_ProcessUDPProtocolFunc[euptUDPProtocolEnd];
    size_t                    m_uUDPProtocolSize[euptUDPProtocolEnd];

    void OnUDPReliable(BYTE* pbyData, size_t uSize);
    void OnUDPUnreliable(BYTE* pbyData, size_t uSize);

private:    // process reliable protocol
    typedef void (JUDPClient::*PROCESS_RELIABLE_PROTOCOL_FUNC)(BYTE* pbyData, size_t uSize);
    PROCESS_RELIABLE_PROTOCOL_FUNC m_ProcessReliableProtocolFunc[s2c_reliable_protocol_end];
    size_t                         m_uReliableProtocolSize[s2c_reliable_protocol_end];

    void OnReliableTestRequest(BYTE* pbyData, size_t uSize);

private:    // process unreliable protocol
    typedef void (JUDPClient::*PROCESS_UNRELIABLE_PROTOCOL_FUNC)(BYTE* pbyData, size_t uSize);
    PROCESS_UNRELIABLE_PROTOCOL_FUNC m_ProcessUnreliableProtocolFunc[s2c_unreliable_protocol_end];
    size_t                           m_uUnreliableProtocolSize[s2c_unreliable_protocol_end];

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

    JUDPConnection m_Connection;
};

#endif //_JUDP_CLIENT_H_