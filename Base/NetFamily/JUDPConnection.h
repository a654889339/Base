#ifndef _JUDP_CONNECTION_H_
#define _JUDP_CONNECTION_H_

#include <list>
#include <set>
#include <time.h>
#include "JUDPBaseDef.h"
#include "JG_Memory.h"

typedef void (* JUDPParseCallBack)(int nConnIndex, BYTE* pbyData, size_t uSize);

class JUDPConnection
{
public:
    JUDPConnection();
    virtual ~JUDPConnection();

    BOOL Init(int nConnIndex, sockaddr_in* pAddr, int nSocketFD, JUDPParseCallBack Func);
    void UnInit();

    void Activate();

    BOOL SendReliablePacket(IJG_Buffer* piBuffer);
    void Send(IJG_Buffer* piBuffer);
    BOOL ProcessPackage(int nConnIndex, BYTE* pbyData, size_t uSize);

private:
    void RetransmitPacket();
    BOOL AddNotAckPacket(IJG_Buffer* piBuffer);
    BOOL AddRecvPacket(DWORD dwPacketID, BYTE* pbyData, size_t uSize);

    BOOL DoAckPacket(DWORD dwPacketID);
    void OnAckPacket(int nConnIndex, BYTE* pbyData, size_t uSize);
    void OnUDPReliable(int nConnIndex, BYTE* pbyData, size_t uSize);
    void OnUDPUnreliable(int nConnIndex, BYTE* pbyData, size_t uSize);

private:
    typedef void (JUDPConnection::*PROCESS_UDP_PROTOCOL_FUNC)(int nConnIndex, BYTE* pbyData, size_t uSize);
    PROCESS_UDP_PROTOCOL_FUNC m_ProcessUDPProtocolFunc[euptUDPProtocolEnd];
    size_t                    m_uUDPProtocolSize[euptUDPProtocolEnd];

    sockaddr_in       m_ConnectionAddr;
    int               m_nConnectionAddrSize;
    int               m_nConnIndex;
    int               m_nSocketFD;
    JUDP_STATUS_TYPE  m_eUDPStatus;
    size_t            m_uSendWindowSize;
    JUDPParseCallBack m_ParseCallBack;

    fd_set      m_ReadFDSet;
    char        m_iRecvBuffer[JUDP_MAX_DATA_SIZE];

private:    // maintain for reliable udp
    DWORD       m_dwSendPacketID;
    DWORD       m_dwRecvPacketID;

    struct JNOT_ACK_PACKET
    {
        DWORD       dwPacketID;
        IJG_Buffer* piBuffer;
        DWORD       dwRetransCount;
        clock_t     lRetransTime;

        JNOT_ACK_PACKET()
        {
            dwPacketID     = 0;
            piBuffer       = NULL;
            dwRetransCount = 0;
            lRetransTime   = 0;
        }
    };

    typedef std::list<JNOT_ACK_PACKET> JSEND_WINDOW_LIST;
    JSEND_WINDOW_LIST                  m_SendWindow;
    JSEND_WINDOW_LIST::iterator        m_SendWindowFind;

    struct JNON_SEQUENCE_PACKET
    {
        DWORD       dwPacketID;
        IJG_Buffer* piBuffer;

        JNON_SEQUENCE_PACKET()
        {
            dwPacketID     = 0;
            piBuffer       = NULL;
        }

        bool operator < (const JNON_SEQUENCE_PACKET& Packet) const
        {
            return dwPacketID < Packet.dwPacketID;
        }
    };

    typedef std::set<JNON_SEQUENCE_PACKET> JRECV_WINDOW_SET;
    JRECV_WINDOW_SET                       m_RecvWindow;
    JRECV_WINDOW_SET::iterator             m_RecvWindowFind;
};

#endif //_JUDP_CONNECTION_H_