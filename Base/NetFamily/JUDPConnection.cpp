#include "JUDPConnection.h"

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
}

JUDPConnection::~JUDPConnection()
{
    UnInit();
}

BOOL JUDPConnection::Init(sockaddr_in* pAddr, int nSocketFD)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;

    JGLOG_PROCESS_ERROR(pAddr);
    JGLOG_PROCESS_ERROR(m_SendWindow.empty());
    JGLOG_PROCESS_ERROR(m_RecvWindow.empty());
    JGLOG_PROCESS_ERROR(m_eUDPStatus == eustInvalid);

    m_ConnectionAddr      = *pAddr;
    m_nConnectionAddrSize = sizeof(m_ConnectionAddr);
    m_nSocketFD           = nSocketFD;

    m_dwSendPacketID      = 0;
    m_dwRecvPacketID      = 0;
    m_eUDPStatus          = eustEstablished;
    m_uSendWindowSize     = JUDP_WINDOW_DEFAULT_SIZE;

    bResult = true;
Exit0:
    if (!bResult)
    {
        m_eUDPStatus = eustError;
    }
    return bResult;
}

void JUDPConnection::UnInit()
{
    Close();
}

void JUDPConnection::Close()
{
    IJG_Buffer*           piBuffer = NULL;
    JNOT_ACK_PACKET*      pSend    = NULL;
    JNON_SEQUENCE_PACKET  RecvPacket;

    m_eUDPStatus = eustDisable;

    while (!m_SendWindow.empty())
    {
        pSend = &m_SendWindow.front();

        JG_COM_RELEASE(pSend->piBuffer);
        m_SendWindow.pop_front();
    }

    for (m_RecvWindowFind = m_RecvWindow.begin(); m_RecvWindowFind != m_RecvWindow.end();)
    {
        RecvPacket = *m_RecvWindowFind;

        JG_COM_RELEASE(RecvPacket.piBuffer);
        m_RecvWindow.erase(m_RecvWindowFind++);
    }

    m_RecvWindow.clear();

    while (!m_SendPacketDeque.empty())
    {
        piBuffer = m_SendPacketDeque.front();

        JG_COM_RELEASE(piBuffer);
        m_SendPacketDeque.pop_front();
    }
}

void JUDPConnection::Activate()
{
    RetransmitPacket();

    if (m_eUDPStatus == eustError || m_eUDPStatus == eustClose || m_eUDPStatus == eustTimeout)
    {
        Close();
    }
}

void JUDPConnection::SetTimeOut()
{
    JG_PROCESS_ERROR(m_eUDPStatus == eustEstablished);

    m_eUDPStatus = eustTimeout;

Exit0:
    return;
}

void JUDPConnection::SetClose()
{
    JG_PROCESS_ERROR(m_eUDPStatus == eustEstablished);

    m_eUDPStatus = eustClose;

Exit0:
    return;
}

BOOL JUDPConnection::IsEnable()
{
    return m_eUDPStatus != eustDisable;
}

BOOL JUDPConnection::Send(IJG_Buffer* piBuffer)
{
    BOOL                 bResult  = false;
    BOOL                 bRetCode = false;
    int                  nRetCode = 0;
    UDP_PROTOCOL_HEADER* pHeader  = NULL;

    JG_PROCESS_ERROR(m_eUDPStatus == eustEstablished);

    JGLOG_PROCESS_ERROR(piBuffer);

    pHeader = (UDP_PROTOCOL_HEADER*)piBuffer->GetData();
    JGLOG_PROCESS_ERROR(pHeader);

    if (pHeader->byUDPProtocol == euptUDPReliable)
    {
        bRetCode = AddNotAckPacket(piBuffer);
        JGLOG_PROCESS_ERROR(bRetCode);
    }

    SendUnreliablePacket(piBuffer);

    bResult = true;
Exit0:
    if (!bResult)
    {
        m_eUDPStatus = eustError;
    }
    return bResult;
}

IJG_Buffer* JUDPConnection::GetRecvPacket()
{
    IJG_Buffer* pRecvPacket = NULL;

    JG_PROCESS_ERROR(m_eUDPStatus == eustEstablished);
    JG_PROCESS_ERROR(!m_RecvWindow.empty());

    m_RecvWindowFind = m_RecvWindow.begin();

    JG_PROCESS_ERROR(m_RecvWindowFind->dwPacketID == m_dwRecvPacketID + 1);

    m_dwRecvPacketID++;

    pRecvPacket = m_RecvWindowFind->piBuffer;

    m_RecvWindow.erase(m_RecvWindowFind);

Exit0:
    return pRecvPacket;
}

IJG_Buffer* JUDPConnection::GetSendPacket()
{
    IJG_Buffer* pSendPacket = NULL;

    JG_PROCESS_ERROR(m_eUDPStatus == eustEstablished);
    JG_PROCESS_ERROR(!m_SendPacketDeque.empty());

    pSendPacket = m_SendPacketDeque.front();

    m_SendPacketDeque.pop_front();

Exit0:
    return pSendPacket;
}

void JUDPConnection::OnAckPacket(BYTE* pbyData, size_t uSize)
{
    BOOL                   bResult  = false;
    JNOT_ACK_PACKET*       pPacket  = NULL;
    EXTERNAL_ACK_PROTOCOL* pAck     = (EXTERNAL_ACK_PROTOCOL *)pbyData;

    JGLOG_PROCESS_ERROR(uSize == sizeof(EXTERNAL_ACK_PROTOCOL));

    JGLOG_PROCESS_ERROR(pAck->dwPacketID <= m_dwSendPacketID);

    while (!m_SendWindow.empty())
    {
        pPacket = &m_SendWindow.front();
        JGLOG_PROCESS_ERROR(pPacket);

        JG_PROCESS_SUCCESS(pPacket->dwPacketID > pAck->dwPacketID);

        JG_COM_RELEASE(pPacket->piBuffer);
        m_SendWindow.pop_front();
    }

Exit1:
    bResult = true;
Exit0:
    if (!bResult)
    {
        m_eUDPStatus = eustError;
    }
    return;
}

void JUDPConnection::OnUDPReliable(BYTE* pbyData, size_t uSize)
{
    BOOL                               bResult   = false;
    BOOL                               bRetCode  = false;
    EXTERNAL_RELIABLE_PROTOCOL_HEADER* pReliable = (EXTERNAL_RELIABLE_PROTOCOL_HEADER*)pbyData;
    JNON_SEQUENCE_PACKET               RecvPack;

    JG_PROCESS_ERROR(m_eUDPStatus == eustEstablished);

    DoAckPacket(pReliable->dwPacketID);

    bRetCode = AddRecvPacket(pReliable->dwPacketID, pbyData, uSize);
    JGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    if (!bResult)
    {
        m_eUDPStatus = eustError;
    }
    return;
}

void JUDPConnection::OnUDPUnreliable(BYTE* pbyData, size_t uSize)
{
    BOOL bResult = false;

    JG_PROCESS_ERROR(m_eUDPStatus == eustEstablished);

    bResult = true;
Exit0:
    if (!bResult)
    {
        m_eUDPStatus = eustError;
    }
    return;
}

BOOL JUDPConnection::SendUnreliablePacket(IJG_Buffer* piBuffer)
{
    BOOL bResult  = false;

    JGLOG_PROCESS_ERROR(piBuffer);
    JGLOG_PROCESS_ERROR(m_SendPacketDeque.size() < JUDP_SEND_CACHE_SIZE);

    piBuffer->AddRef();
    m_SendPacketDeque.push_back(piBuffer);

    bResult = true;
Exit0:
    if (!bResult)
    {
        if (piBuffer)
        {
            m_eUDPStatus = eustTimeout;
        }
        else
        {
            m_eUDPStatus = eustError;
        }
    }
    return bResult;
}

void JUDPConnection::RetransmitPacket()
{
    IJG_Buffer* piBuffer = NULL;
    LONG        lTimeNow = clock();

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

            piBuffer = m_SendWindowFind->piBuffer;
            JGLOG_PROCESS_ERROR(piBuffer);

            SendUnreliablePacket(piBuffer);
        }
    }

Exit0:
    return ;
}

BOOL JUDPConnection::AddNotAckPacket(IJG_Buffer* piBuffer)
{
    BOOL                               bResult   = false;
    EXTERNAL_RELIABLE_PROTOCOL_HEADER* pReliable = NULL;
    JNOT_ACK_PACKET                    Packet;

    JGLOG_PROCESS_ERROR(piBuffer);

    pReliable = (EXTERNAL_RELIABLE_PROTOCOL_HEADER*)piBuffer->GetData();
    JGLOG_PROCESS_ERROR(pReliable);

    pReliable->dwPacketID = ++m_dwSendPacketID;

    Packet.dwPacketID     = pReliable->dwPacketID;
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
    BOOL                 bResult  = false;
    IJG_Buffer*          piBuffer = NULL;
    JNON_SEQUENCE_PACKET Packet;

    piBuffer = JG_MemoryCreateBuffer(uSize);
    JGLOG_PROCESS_ERROR(piBuffer);

    memcpy(piBuffer->GetData(), pbyData, uSize);

    piBuffer->AddRef();

    Packet.dwPacketID = dwPacketID;
    Packet.piBuffer   = piBuffer;

    m_RecvWindow.insert(Packet);

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

    pACK = (EXTERNAL_ACK_PROTOCOL *)piBuffer->GetData();
    JGLOG_PROCESS_ERROR(pACK);

    pACK->byUDPProtocol = euptUDPACK;
    pACK->dwPacketID    = dwPacketID;

    SendUnreliablePacket(piBuffer);

    bResult = true;
Exit0:
    JG_COM_RELEASE(piBuffer);
    return bResult;
}
