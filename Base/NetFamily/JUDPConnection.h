#ifndef _JUDP_CONNECTION_H_
#define _JUDP_CONNECTION_H_

#include <list>
#include <set>
#include <deque>
#include <time.h>
#include "JUDPBaseDef.h"
#include "JG_Memory.h"


class JUDPConnection
{
public:
    JUDPConnection();
    virtual ~JUDPConnection();

    BOOL Init(sockaddr_in* pAddr, int nSocketFD);
    void UnInit();

    void Activate();

    void SetTimeOut();
    void SetClose();
    BOOL IsEnable();
    BOOL Send(IJG_Buffer* piBuffer);

    IJG_Buffer* GetRecvPacket();
    IJG_Buffer* GetSendPacket();

    void OnAckPacket(BYTE* pbyData, size_t uSize);
    void OnUDPReliable(BYTE* pbyData, size_t uSize);
    void OnUDPUnreliable(BYTE* pbyData, size_t uSize);

private:
    void Close();
    BOOL SendUnreliablePacket(IJG_Buffer* piBuffer);

    void RetransmitPacket();
    BOOL AddNotAckPacket(IJG_Buffer* piBuffer);
    BOOL AddRecvPacket(DWORD dwPacketID, BYTE* pbyData, size_t uSize);

    BOOL DoAckPacket(DWORD dwPacketID);

public:
    sockaddr_in       m_ConnectionAddr;
    int               m_nConnectionAddrSize;

private:
    int               m_nSocketFD;
    JUDP_STATUS_TYPE  m_eUDPStatus;
    size_t            m_uSendWindowSize;

    fd_set            m_ReadFDSet;
    char              m_iRecvBuffer[JUDP_MAX_DATA_SIZE];

private:    // maintain for reliable udp
    DWORD             m_dwSendPacketID;
    DWORD             m_dwRecvPacketID;

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

    typedef std::list<JNOT_ACK_PACKET>     JSEND_WINDOW_LIST;
    JSEND_WINDOW_LIST                      m_SendWindow;
    JSEND_WINDOW_LIST::iterator            m_SendWindowFind;

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

    typedef std::deque<IJG_Buffer*>        JSEND_PACKET_DEQUE;
    JSEND_PACKET_DEQUE                     m_SendPacketDeque;
};

#endif //_JUDP_CONNECTION_H_