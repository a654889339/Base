#ifndef _JUDP_CONNECTION_H_
#define _JUDP_CONNECTION_H_

#include <map>
#include "JBaseDef.h"
#include "JG_Memory.h"
#include "JUDPBaseDef.h"

class JUDPConnection
{
public:
    JUDPConnection();
    virtual ~JUDPConnection();

    BOOL Init(int nConnIndex, sockaddr_in* pAddr, int nSocketFD);
    void UnInit();

    BOOL Send(IJG_Buffer* piBuffer);
    BOOL ProcessPackage(int nConnIndex, BYTE* pbyData, size_t uSize);

private:
    BOOL OnAckPacket(DWORD dwPacket);
    void OnUDPReliable(int nConnIndex, BYTE* pbyData, size_t uSize);
    void OnUDPUnreliable(int nConnIndex, BYTE* pbyData, size_t uSize);

private:
    typedef void (JUDPConnection::*PROCESS_UDP_PROTOCOL_FUNC)(int nConnIndex, BYTE* pbyData, size_t uSize);
    PROCESS_UDP_PROTOCOL_FUNC m_ProcessUDPProtocolFunc[eUDPProtocolEnd];
    size_t                    m_nUDPProtocolSize[eUDPProtocolEnd];

    sockaddr_in m_ConnectionAddr;
    int         m_nConnectionAddrSize;
    int         m_nConnIndex;
    int         m_nSocketFD;

    fd_set      m_ReadFDSet;
    char        m_iRecvBuffer[JUDP_MAX_DATA_SIZE];

private:    // maintain for reliable udp
    typedef std::map<DWORD, IJG_Buffer*> JNOT_ACK_MAP;
    JNOT_ACK_MAP                         m_NotACKMap;
    JNOT_ACK_MAP::iterator               m_TempACKFind;

    DWORD       m_dwPacketID;
    IJG_Buffer* m_pTempBuffer;
};

#endif //_JUDP_CONNECTION_H_