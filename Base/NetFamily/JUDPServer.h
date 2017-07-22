#ifndef _JUDP_SERVER_H_
#define _JUDP_SERVER_H_

#include <set>
#include <map>
#include "JUDPBaseDef.h"
#include "JG_Memory.h"
#include "JUDPConnection.h"

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

    // return -1: error, 0: timeout, 1: success, -2: non-block && no data && success
    int  Recv(IJG_Buffer** ppiRetBuffer, sockaddr_in* pClientAddr, int* pnClientAddrSize);
    BOOL Send(int nConnIndex, IJG_Buffer* piBuffer);

    BOOL AddConnection(int *pnConnIndex, sockaddr_in *pAddr, size_t uAddrSize);
    void RemoveConnection(int nConnIndex);
    JUDPConnection* GetConnection(int nConnIndex);
    void ClearConnections();

private:
    BOOL GetConnIndex(int *pnConnIndex, sockaddr_in *pAddr);

    WSADATA     m_WSAData;
    BYTE        m_byLowByteVersion;
    BYTE        m_byHightByteVersion;
    int         m_nSocketFD;
    sockaddr_in m_ServerAddr;
    int         m_nPort;

    fd_set      m_ReadFDSet;
    char        m_iRecvBuffer[JUDP_MAX_DATA_SIZE];

private:
    typedef std::map<int, JUDPConnection*> JUDP_CONNECTIONS_MAP;
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