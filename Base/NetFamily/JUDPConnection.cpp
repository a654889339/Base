#include "JUDPConnection.h"

#define REGISTER_UDP_PARSE_FUNC(ProtocolID, FuncName, ProtocolSize) \
{m_ProcessUDPProtocolFunc[ProtocolID] = FuncName;                   \
    m_uUDPProtocolSize[ProtocolID] = ProtocolSize;}


JUDPConnection::JUDPConnection()
{
    m_nConnectionAddrSize = 0;
    m_dwSendPacketID      = 0;
    m_dwRecvPacketID      = 0;
    m_eUDPStatus          = eustInvalid;
    m_uSendWindowSize     = JUDP_WINDOW_DEFAULT_SIZE;

#ifdef WIN32
    memset(&m_ConnectionAddr, 0, sizeof(m_ConnectionAddr));
#else
    bzero(&m_ConnectionAddr, sizeof(m_ConnectionAddr));
#endif

    memset(m_ProcessUDPProtocolFunc, 0, sizeof(m_ProcessUDPProtocolFunc));
    memset(m_uUDPProtocolSize, 0, sizeof(m_uUDPProtocolSize));

    REGISTER_UDP_PARSE_FUNC(euptUDPReliable, &JUDPConnection::OnUDPReliable, sizeof(EXTERNAL_RELIABLE_PROTOCOL_HEADER));
    REGISTER_UDP_PARSE_FUNC(euptUDPUnreliable, &JUDPConnection::OnUDPUnreliable, sizeof(EXTERNAL_UNRELIABLE_PROTOCOL_HEADER));
    REGISTER_UDP_PARSE_FUNC(euptUDPACK, &JUDPConnection::OnAckPacket, sizeof(EXTERNAL_ACK_PROTOCOL));
}

JUDPConnection::~JUDPConnection()
{
    UnInit();
}

BOOL JUDPConnection::Init(int nConnIndex, sockaddr_in* pAddr, int nSocketFD)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;

    JGLOG_PROCESS_ERROR(pAddr);

    m_nConnIndex          = nConnIndex;
    m_ConnectionAddr      = *pAddr;
    m_nConnectionAddrSize = sizeof(m_ConnectionAddr);
    m_nSocketFD           = nSocketFD;

    bResult = true;
Exit0:
    return bResult;
}

void JUDPConnection::UnInit()
{
    JNOT_ACK_PACKET*      pSend = NULL;
    JNON_SEQUENCE_PACKET  RecvPacket;

    while (!m_SendWindow.empty())
    {
        pSend = &m_SendWindow.front();

        m_SendWindow.pop_front();

        JG_COM_RELEASE(pSend->piBuffer);
    }

    for (m_RecvWindowFind = m_RecvWindow.begin(); m_RecvWindowFind != m_RecvWindow.end();)
    {
        RecvPacket = *m_RecvWindowFind;

        m_RecvWindow.erase(m_RecvWindowFind++);

        JG_COM_RELEASE(RecvPacket.piBuffer);
    }
}

void JUDPConnection::Activate()
{
    RetransmitPacket();
}

BOOL JUDPConnection::SendReliablePacket(IJG_Buffer* piBuffer)
{
    BOOL                 bResult  = false;
    BOOL                 bRetCode = false;
    int                  nRetCode = 0;
    UDP_PROTOCOL_HEADER* pHeader  = NULL;

    JG_PROCESS_ERROR(m_eUDPStatus == eustEstablished);

    JGLOG_PROCESS_ERROR(piBuffer);

    pHeader = (UDP_PROTOCOL_HEADER*)piBuffer->GetData();
    JGLOG_PROCESS_ERROR(pHeader);

    JGLOG_PROCESS_ERROR(pHeader->byUDPProtocol == euptUDPReliable);

    bRetCode = AddNotAckPacket(piBuffer);
    JGLOG_PROCESS_ERROR(bRetCode);

    Send(piBuffer);

    bResult = true;
Exit0:
    return bResult;
}

void JUDPConnection::Send(IJG_Buffer* piBuffer)
{
    int nRetCode = 0;

    JGLOG_PROCESS_ERROR(piBuffer);

    nRetCode = sendto(m_nSocketFD, (char *)piBuffer->GetData(), piBuffer->GetSize(), 0, (sockaddr *)&m_ConnectionAddr, m_nConnectionAddrSize);
    JGLOG_PROCESS_ERROR(nRetCode == piBuffer->GetSize());

Exit0:
    return;
}

BOOL JUDPConnection::ProcessPackage(int nConnIndex, BYTE* pbyData, size_t uSize)
{
    BOOL                        bResult         = false;
    BOOL                        bRetCode        = false;
    UDP_PROTOCOL_HEADER*        pHeader         = NULL;
    PROCESS_UDP_PROTOCOL_FUNC   ProcessFunc     = NULL;

    JG_PROCESS_ERROR(m_eUDPStatus == eustEstablished);

    JGLOG_PROCESS_ERROR(nConnIndex == m_nConnIndex);
    JGLOG_PROCESS_ERROR(pbyData);

    pHeader = (UDP_PROTOCOL_HEADER*)pbyData;

    JGLOG_PROCESS_ERROR(pHeader->byUDPProtocol > euptUDPProtocolBegin);
    JGLOG_PROCESS_ERROR(pHeader->byUDPProtocol < euptUDPProtocolEnd);

    JGLOG_PROCESS_ERROR(uSize >= m_uUDPProtocolSize[pHeader->byUDPProtocol]);

    ProcessFunc = m_ProcessUDPProtocolFunc[pHeader->byUDPProtocol];
    JGLOG_PROCESS_ERROR(ProcessFunc);

    (this->*ProcessFunc)(nConnIndex, (BYTE*)pHeader, uSize);

    bResult = true;
Exit0:
    return bResult;
}

void JUDPConnection::RetransmitPacket()
{
    LONG lTimeNow = clock(); 
    JG_PROCESS_ERROR(m_eUDPStatus == eustEstablished);

    for (m_SendWindowFind = m_SendWindow.begin(); m_SendWindowFind != m_SendWindow.end(); ++m_SendWindowFind)
    {
        if (m_SendWindowFind->lRetransTime >= lTimeNow)
        {
            m_SendWindowFind->lRetransTime = lTimeNow + JUDP_RETRANS_INTERVAL;

            m_SendWindowFind->dwRetransCount++;
            if (m_SendWindowFind->dwRetransCount > JUDP_RETRANS_INTERVAL)
            {
                m_eUDPStatus = eustTimeout;
                goto Exit0;
            }

            Send(m_SendWindowFind->piBuffer);
        }
    }

Exit0:
    return ;
}

BOOL JUDPConnection::AddNotAckPacket(IJG_Buffer* piBuffer)
{
    BOOL            bResult = false;
    JNOT_ACK_PACKET Packet;

    JGLOG_PROCESS_ERROR(piBuffer);

    Packet.dwPacketID     = ++m_dwSendPacketID;
    Packet.dwRetransCount = 0;
    Packet.piBuffer       = piBuffer;
    Packet.lRetransTime   = clock() + JUDP_RETRANS_INTERVAL;

    if (m_SendWindow.size() >= m_uSendWindowSize)
    {
        m_eUDPStatus = eustTimeout;
        goto Exit0;
    }

    m_SendWindow.push_back(Packet);

    piBuffer->AddRef();

    bResult = true;
Exit0:
    return bResult;
}

BOOL JUDPConnection::AddRecvPacket(DWORD dwPacketID, BYTE* pbyData, size_t uSize)
{
    BOOL        bResult  = false;
    IJG_Buffer* piBuffer = NULL;

    if (dwPacketID == m_dwRecvPacketID + 1)
    {

    }
    else
    {
        JNON_SEQUENCE_PACKET Packet;

        piBuffer = JG_MemoryCreateBuffer(uSize);
        JGLOG_PROCESS_ERROR(piBuffer);

        memcpy(piBuffer->GetData(), pbyData, uSize);

        piBuffer->AddRef();

        Packet.dwPacketID = dwPacketID;
        Packet.piBuffer   = piBuffer;

        m_RecvWindow.insert(Packet);
    }

    bResult = true;
Exit0:
    JG_COM_RELEASE(piBuffer);
    return bResult;
}

BOOL JUDPConnection::DoAckPacket(DWORD dwPacketID)
{
    BOOL                   bResult  = false;
    IJG_Buffer*            piBuffer = NULL;
    EXTERNAL_ACK_PROTOCOL* pACK     = NULL;

    piBuffer = JG_MemoryCreateBuffer(sizeof(EXTERNAL_ACK_PROTOCOL));
    JGLOG_PROCESS_ERROR(piBuffer);

    pACK = (EXTERNAL_ACK_PROTOCOL*)piBuffer->GetData();
    JGLOG_PROCESS_ERROR(pACK);

    pACK->byUDPProtocol = euptUDPACK;
    pACK->dwPacketID    = dwPacketID;

    Send(piBuffer);

    bResult = true;
Exit0:
    JG_COM_RELEASE(piBuffer);
    return bResult;
}

void JUDPConnection::OnAckPacket(int nConnIndex, BYTE* pbyData, size_t uSize)
{
    JNOT_ACK_PACKET*       pPacket  = NULL;
    EXTERNAL_ACK_PROTOCOL* pAck     = (EXTERNAL_ACK_PROTOCOL *)pbyData;

    JGLOG_PROCESS_ERROR(uSize == sizeof(EXTERNAL_ACK_PROTOCOL));

    if (pAck->dwPacketID > m_dwSendPacketID)
    {
        m_eUDPStatus = eustError;
        goto Exit0;
    }

    while (!m_SendWindow.empty())
    {
        pPacket = &m_SendWindow.front();
        JGLOG_PROCESS_ERROR(pPacket);

        JG_PROCESS_SUCCESS(pPacket->dwPacketID > pAck->dwPacketID);

        m_SendWindow.pop_front();

        JG_COM_RELEASE(pPacket->piBuffer);
    }

Exit1:
Exit0:
    return;
}

void JUDPConnection::OnUDPReliable(int nConnIndex, BYTE* pbyData, size_t uSize)
{
    BOOL                               bRetCode  = false;
    EXTERNAL_RELIABLE_PROTOCOL_HEADER* pReliable = (EXTERNAL_RELIABLE_PROTOCOL_HEADER*)pbyData;

    DoAckPacket(pReliable->dwPacketID);

    bRetCode = AddRecvPacket(pReliable->dwPacketID, pbyData, uSize);
    JGLOG_PROCESS_ERROR(bRetCode);

Exit0:
    return;
}

void JUDPConnection::OnUDPUnreliable(int nConnIndex, BYTE* pbyData, size_t uSize)
{

}
