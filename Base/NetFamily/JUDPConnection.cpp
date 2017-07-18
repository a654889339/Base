#include "JUDPConnection.h"

#define REGISTER_UDP_PARSE_FUNC(ProtocolID, FuncName, ProtocolSize) \
{m_ProcessUdpProtocolFunc[ProtocolID] = FuncName;                   \
    m_nUdpProtocolSize[ProtocolID] = ProtocolSize;}


JUDPConnection::JUDPConnection()
{
    m_nConnectionAddrSize = 0;
    m_dwPacketID          = 0;
    m_pTempBuffer         = NULL;

#ifdef WIN32
    memset(&m_ConnectionAddr, 0, sizeof(m_ConnectionAddr));
#else
    bzero(&m_ConnectionAddr, sizeof(m_ConnectionAddr));
#endif

    memset(m_ProcessUDPProtocolFunc, 0, sizeof(m_ProcessUDPProtocolFunc));
    memset(m_nUDPProtocolSize, 0, sizeof(m_nUDPProtocolSize));

    REGISTER_UDP_PARSE_FUNC(eReliable, OnUDPReliable, sizeof(EXTERNAL_RELIABLE_PROTOCOL_HEADER));
    REGISTER_UDP_PARSE_FUNC(eUnreliable, OnUDPUnreliable, sizeof(EXTERNAL_UNRELIABLE_PROTOCOL_HEADER));
}

JUDPConnection::~JUDPConnection()
{

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
    for (m_TempACKFind = m_NotACKMap.begin(); m_TempACKFind != m_NotACKMap.end(); )
    {
        m_pTempBuffer = m_TempACKFind->second;
        m_NotACKMap.erase(m_TempACKFind++);
        JG_COM_RELEASE(m_pTempBuffer);
    }
}

BOOL JUDPConnection::Send(IJG_Buffer* piBuffer)
{
    BOOL bResult = false;
    int  nRetCode = 0;

    JGLOG_PROCESS_ERROR(piBuffer);

    nRetCode = sendto(m_nSocketFD, (char *)piBuffer->GetData(), piBuffer->GetSize(), 0, (sockaddr *)&m_ConnectionAddr, m_nConnectionAddrSize);
    JGLOG_PROCESS_ERROR(nRetCode == piBuffer->GetSize());

    piBuffer->AddRef();
    m_NotACKMap[++m_dwPacketID] = piBuffer;

    bResult = true;
Exit0:
    return bResult;
}

BOOL JUDPConnection::ProcessPackage(int nConnIndex, BYTE* pbyData, size_t uSize)
{
    BOOL                        bResult         = false;
    UDP_PROTOCOL_HEADER*        pHeader         = NULL;
    PROCESS_UDP_PROTOCOL_FUNC   ProcessFunc     = NULL;

    JGLOG_PROCESS_ERROR(nConnIndex == m_nConnIndex);
    JGLOG_PROCESS_ERROR(pbyData);

    pHeader = (UDP_PROTOCOL_HEADER*)pbyData;

    JGLOG_PROCESS_ERROR(pHeader->byUDPProtocol > eUDPProtocolBegin);
    JGLOG_PROCESS_ERROR(pHeader->byUDPProtocol < eUDPProtocolEnd);

    JGLOG_PROCESS_ERROR(uSize >= m_nUDPProtocolSize[pHeader->byUDPProtocol]);

    ProcessFunc = m_ProcessUDPProtocolFunc[pHeader->byUDPProtocol];
    JGLOG_PROCESS_ERROR(ProcessFunc);

    (this->*ProcessFunc)(nConnIndex, (BYTE*)pHeader, uSize);

    bResult = true;
Exit0:
    return bResult;
}

BOOL JUDPConnection::OnAckPacket(DWORD dwPacket)
{
    BOOL        bResult  = false;
    IJG_Buffer* piBuffer = NULL;

    m_TempACKFind = m_NotACKMap.find(dwPacket);
    JGLOG_PROCESS_ERROR(m_TempACKFind != m_NotACKMap.end());

    piBuffer = m_TempACKFind->second;
    JGLOG_PROCESS_ERROR(piBuffer);

    m_NotACKMap.erase(m_TempACKFind);

    JG_COM_RELEASE(piBuffer);

    bResult = true;
Exit0:
    return bResult;
}

void JUDPConnection::OnUDPReliable(int nConnIndex, BYTE* pbyData, size_t uSize)
{
    BOOL                               bRetCode  = false;
    EXTERNAL_RELIABLE_PROTOCOL_HEADER* pReliable = (EXTERNAL_RELIABLE_PROTOCOL_HEADER*)pbyData;

    bRetCode = OnAckPacket(pReliable->dwPacketID);
    JGLOG_PROCESS_ERROR(bRetCode);

Exit0:
    return;
}

void JUDPConnection::OnUDPUnreliable(int nConnIndex, BYTE* pbyData, size_t uSize)
{

}
