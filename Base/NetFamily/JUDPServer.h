#ifndef _JUDP_SERVER_H_
#define _JUDP_SERVER_H_

#include <set>
#include <map>
#include "JUDPBaseDef.h"
#include "JG_Memory.h"
#include "JUDPConnection.h"
#include "JGS_Client_Protocol.h"

class JUDPServer
{
public:
    JUDPServer();
    virtual ~JUDPServer();

    BOOL Init();
    void UnInit();

    void Activate();

    BOOL Listen(char* pszIP, int nPort);
    void Close();

    BOOL Send(int nConnIndex, IJG_Buffer* piBuffer);
    BOOL Broadcast(IJG_Buffer* piBuffer);

private:
    // return -1: error, 0: timeout, 1: success, 2: non-block && no data && success
    int  Recv(IJG_Buffer** ppiRetBuffer, sockaddr_in* pClientAddr, int* pnClientAddrSize);

    BOOL AddConnection(int *pnConnIndex, sockaddr_in *pAddr, size_t uAddrSize);
    void RemoveConnection(int nConnIndex);
    JUDPConnection* GetConnection(int nConnIndex);
    void ClearConnections();

    BOOL GetConnIndex(int *pnConnIndex, sockaddr_in *pAddr);
    BOOL ProcessPackage();
    void ProcessSendPacket();
    void ProcessRecvPacket();

private:    // process udp header
    typedef void (JUDPServer::*PROCESS_UDP_PROTOCOL_FUNC)(int nConnIndex, BYTE* pbyData, size_t uSize);
    PROCESS_UDP_PROTOCOL_FUNC m_ProcessUDPProtocolFunc[euptUDPProtocolEnd];
    size_t                    m_uUDPProtocolSize[euptUDPProtocolEnd];

    void OnUDPReliable(int nConnIndex, BYTE* pbyData, size_t uSize);
    void OnUDPUnreliable(int nConnIndex, BYTE* pbyData, size_t uSize);

private:    // process reliable protocol
    typedef void (JUDPServer::*PROCESS_RELIABLE_PROTOCOL_FUNC)(int nConnIndex, BYTE* pbyData, size_t uSize);
    PROCESS_RELIABLE_PROTOCOL_FUNC m_ProcessReliableProtocolFunc[c2s_reliable_protocol_end];
    size_t                         m_uReliableProtocolSize[c2s_reliable_protocol_end];

    void OnReliableTestRespond(int nConnIndex, BYTE* pbyData, size_t uSize);

private:    // process unreliable protocol
    typedef void (JUDPServer::*PROCESS_UNRELIABLE_PROTOCOL_FUNC)(int nConnIndex, BYTE* pbyData, size_t uSize);
    PROCESS_UNRELIABLE_PROTOCOL_FUNC m_ProcessUnreliableProtocolFunc[c2s_unreliable_protocol_end];
    size_t                           m_uUnreliableProtocolSize[c2s_unreliable_protocol_end];

private:
    WSADATA     m_WSAData;
    BYTE        m_byLowByteVersion;
    BYTE        m_byHightByteVersion;
    int         m_nSocketFD;
    sockaddr_in m_ServerAddr;
    int         m_nPort;
    BOOL        m_bWorkFlag;

    fd_set      m_ReadFDSet;
    char        m_iRecvBuffer[JUDP_MAX_DATA_SIZE];

private:    // maintain for connections
    typedef std::set<int>                 JUDP_WAIT_CLOSE_CONNECTION_SET;
    JUDP_WAIT_CLOSE_CONNECTION_SET        m_WaitCloseSet;

    typedef std::map<int, JUDPConnection> JUDP_CONNECTIONS_MAP;
    JUDP_CONNECTIONS_MAP                   m_ConnectionsMap;
    JUDP_CONNECTIONS_MAP::iterator         m_ConnectionsMapFind;

    struct JConnInfo
    {
        sockaddr_in Addr;
        size_t      uAddrSize;
        int         nConnIndex;

        JConnInfo()
        {
#ifdef WIN32
            memset(&Addr, 0, sizeof(Addr));
#else
            bzero(&Addr, sizeof(Addr));
#endif
            uAddrSize  = 0;
            nConnIndex = 0;
        }

        bool operator < (const JConnInfo& ConnInfo) const
        {
            return Addr.sin_addr.S_un.S_addr < ConnInfo.Addr.sin_addr.S_un.S_addr;
        }
    };

    JConnInfo                              m_ConnectionsInfo;
    typedef std::set<JConnInfo>            JUDP_CONNECTIONS_INFO_SET;
    JUDP_CONNECTIONS_INFO_SET              m_ConnectionsInfoSet;
    JUDP_CONNECTIONS_INFO_SET::iterator    m_ConnectionsInfoSetFind;

    int                                    m_nConnectionCount;
};

#endif // _JUDP_SERVER_H_