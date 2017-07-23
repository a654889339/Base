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

#ifdef WIN32
    memset(&m_ConnectionAddr, 0, sizeof(m_ConnectionAddr));
#else
    bzero(&m_ConnectionAddr, sizeof(m_ConnectionAddr));
#endif

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
    JNOT_ACK_PACKET*      pSend = NULL;
    JNON_SEQUENCE_PACKET  RecvPacket;

    m_eUDPStatus = eustInvalid;

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

    m_RecvWindow.clear();
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

BOOL JUDPConnection::IsInvalid()
{
    return m_eUDPStatus == eustInvalid;
}

BOOL JUDPConnection::Send(BYTE* pbyData, size_t uSize)
{
    BOOL                 bResult  = false;
    BOOL                 bRetCode = false;
    int                  nRetCode = 0;
    UDP_PROTOCOL_HEADER* pHeader  = NULL;

    JG_PROCESS_ERROR(m_eUDPStatus == eustEstablished);

    JGLOG_PROCESS_ERROR(pbyData);

    pHeader = (UDP_PROTOCOL_HEADER*)pbyData;
    JGLOG_PROCESS_ERROR(pHeader);

    if (pHeader->byUDPProtocol == euptUDPReliable)
    {
        bRetCode = AddNotAckPacket(pbyData, uSize);
        JGLOG_PROCESS_ERROR(bRetCode);
    }

    bRetCode = SendUnreliablePacket(pbyData, uSize);
    JGLOG_PROCESS_ERROR(bRetCode);

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

        m_SendWindow.pop_front();

        JG_COM_RELEASE(pPacket->piBuffer);
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

BOOL JUDPConnection::SendUnreliablePacket(BYTE* pbyData, size_t uSize)
{
    BOOL bResult  = false;
    int  nRetCode = 0;

    JGLOG_PROCESS_ERROR(pbyData);

    nRetCode = sendto(m_nSocketFD, (char *)pbyData, uSize, 0, (sockaddr *)&m_ConnectionAddr, m_nConnectionAddrSize);
    JGLOG_PROCESS_ERROR(nRetCode == uSize);

    bResult = true;
Exit0:
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

            SendUnreliablePacket((BYTE *)piBuffer->GetData(), piBuffer->GetSize());
        }
    }

Exit0:
    return ;
}

BOOL JUDPConnection::AddNotAckPacket(BYTE* pbyData, size_t uSize)
{
    BOOL            bResult  = false;
    IJG_Buffer*     piBuffer = NULL;
    JNOT_ACK_PACKET Packet;

    JGLOG_PROCESS_ERROR(pbyData);

    piBuffer = JG_MemoryCreateBuffer(uSize);
    JGLOG_PROCESS_ERROR(piBuffer);

    memcpy(piBuffer->GetData(), pbyData, uSize);

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
    JG_COM_RELEASE(piBuffer);
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
    EXTERNAL_ACK_PROTOCOL  ACK;

    ACK.dwPacketID = dwPacketID;

    SendUnreliablePacket((BYTE *)&ACK, sizeof(ACK));

    bResult = true;
//Exit0:
    return bResult;
}
