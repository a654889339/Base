#include "JUDPServer.h"
#include "JGS_Client_Protocol.h"

#define REGISTER_UDP_PARSE_FUNC(ProtocolID, FuncName, ProtocolSize) \
{m_ProcessUDPProtocolFunc[ProtocolID] = FuncName;                   \
    m_uUDPProtocolSize[ProtocolID] = ProtocolSize;}


JUDPServer::JUDPServer()
{
    m_byLowByteVersion   = 1;
    m_byHightByteVersion = 1;
    m_nConnectionCount   = 0;
    m_bWorkFlag          = false;

    memset(m_ProcessUDPProtocolFunc, 0, sizeof(m_ProcessUDPProtocolFunc));
    memset(m_uUDPProtocolSize, 0, sizeof(m_uUDPProtocolSize));

    REGISTER_UDP_PARSE_FUNC(euptUDPReliable, &JUDPServer::OnUDPReliable, sizeof(EXTERNAL_RELIABLE_PROTOCOL_HEADER));
    REGISTER_UDP_PARSE_FUNC(euptUDPUnreliable, &JUDPServer::OnUDPUnreliable, sizeof(EXTERNAL_UNRELIABLE_PROTOCOL_HEADER));
}

JUDPServer::~JUDPServer()
{
    UnInit();
}

BOOL JUDPServer::Init()
{
    BOOL bResult  = false;
    BOOL bRetCode = false;

    bResult = true;
//Exit0:
    return bResult;
}

void JUDPServer::UnInit()
{
    ClearConnections();
    WSACleanup();
}

void JUDPServer::Activate()
{
    ProcessPackage();
    ProcessConnections();
}

BOOL JUDPServer::Listen(char* pszIP, int nPort)
{
    BOOL  bResult  = false;
    BOOL  bRetCode = false;
    int   nRetCode = 0;
    WORD  wVersion = MAKEWORD(m_byHightByteVersion, m_byLowByteVersion);

    nRetCode = WSAStartup(wVersion, &m_WSAData);
    JGLOG_PROCESS_ERROR(nRetCode == 0);

    nRetCode = LOBYTE(m_WSAData.wVersion);
    JGLOG_PROCESS_ERROR(nRetCode == m_byLowByteVersion);

    nRetCode = HIBYTE(m_WSAData.wVersion);
    JGLOG_PROCESS_ERROR(nRetCode == m_byHightByteVersion);

    m_nSocketFD = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    JGLOG_PROCESS_ERROR(m_nSocketFD != INVALID_SOCKET);

#ifdef WIN32
    memset(&m_ServerAddr, 0, sizeof(m_ServerAddr));
#else
    bzero(&m_ServerAddr, sizeof(m_ServerAddr));
#endif

    m_ServerAddr.sin_family           = AF_INET;
    m_ServerAddr.sin_addr.S_un.S_addr = inet_addr(pszIP);
    m_ServerAddr.sin_port             = htons(nPort);

    nRetCode = bind(m_nSocketFD, (sockaddr*)&m_ServerAddr, sizeof(m_ServerAddr));
    JGLOG_PROCESS_ERROR(nRetCode != SOCKET_ERROR);

    m_bWorkFlag = true;

    bResult = true;
Exit0:
    if (!bResult)
    {
        Close();
    }
    return bResult;
}

void JUDPServer::Close()
{
    closesocket(m_nSocketFD);
}

int JUDPServer::Recv(IJG_Buffer** ppiRetBuffer, sockaddr_in* pClientAddr, int* pnClientAddrSize)
{
    int         nResult   = -1;
    BOOL        bRetCode  = false;
    int         nRetCode  = 0;
    int         nConnIndex = 0;
    IJG_Buffer* piBuffer  = NULL;
    timeval     TimeOut   = {0, 0};

    JGLOG_PROCESS_ERROR(ppiRetBuffer);
    JGLOG_PROCESS_ERROR(pClientAddr);
    JGLOG_PROCESS_ERROR(pnClientAddrSize);

    *ppiRetBuffer = NULL;

    FD_ZERO(&m_ReadFDSet);
    FD_SET(m_nSocketFD, &m_ReadFDSet);

    nRetCode = select(m_nSocketFD + 1, &m_ReadFDSet, NULL, NULL, &TimeOut);
    if (nRetCode == 0)
    {
        nResult = 2;
        goto Exit0;
    }
    else if (nRetCode < 0)
    {
        nResult = 0;
        goto Exit0;
    }

    nRetCode = recvfrom(m_nSocketFD, m_iRecvBuffer, JUDP_MAX_DATA_SIZE, 0, (sockaddr*)pClientAddr, pnClientAddrSize);
    JGLOG_PROCESS_ERROR(nRetCode != -1);

    piBuffer = JG_MemoryCreateBuffer(nRetCode);
    JGLOG_PROCESS_ERROR(piBuffer);

    memcpy(piBuffer->GetData(), m_iRecvBuffer, nRetCode);

    *ppiRetBuffer = piBuffer;

    (*ppiRetBuffer)->AddRef();

    nResult = 1;
Exit0:
    JG_COM_RELEASE(piBuffer);
    return nResult;
}

BOOL JUDPServer::Send(int nConnIndex, BYTE* pbyData, size_t uSize)
{
    BOOL            bResult      = false;
    BOOL            bRetCode     = false;
    JUDPConnection* pConnection  = NULL;

    JGLOG_PROCESS_ERROR(pbyData);

    pConnection = GetConnection(nConnIndex);
    JG_PROCESS_ERROR(pConnection);

    bRetCode = pConnection->Send(pbyData, uSize);
    JGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL JUDPServer::Broadcast(BYTE* pbyData, size_t uSize)
{
    BOOL            bResult      = false;
    BOOL            bRetCode     = false;
    JUDPConnection* pConnection  = NULL;

    JGLOG_PROCESS_ERROR(pbyData);

    for (m_ConnectionsMapFind = m_ConnectionsMap.begin(); m_ConnectionsMapFind != m_ConnectionsMap.end(); ++m_ConnectionsMapFind)
    {
        pConnection = &m_ConnectionsMapFind->second;

        bRetCode = pConnection->Send(pbyData, uSize);
        if (!bRetCode)
        {
            JGLOG_PROCESS_ERROR(false);
        }
    }

    bResult = true;
Exit0:
    return bResult;
}

BOOL JUDPServer::AddConnection(int *pnConnIndex, sockaddr_in *pAddr, size_t uAddrSize)
{
    BOOL            bResult     = false;
    JUDPConnection *pConnection = NULL;

    JGLOG_PROCESS_ERROR(pnConnIndex);
    JGLOG_PROCESS_ERROR(pAddr);

    m_ConnectionsInfo.Addr = *pAddr;

    m_ConnectionsInfoSetFind = m_ConnectionsInfoSet.find(m_ConnectionsInfo);
    JGLOG_PROCESS_ERROR(m_ConnectionsInfoSetFind == m_ConnectionsInfoSet.end());

    m_ConnectionsInfo.nConnIndex = ++m_nConnectionCount;
    m_ConnectionsInfo.uAddrSize  = uAddrSize;

    m_ConnectionsInfoSet.insert(m_ConnectionsInfo);

    *pnConnIndex = m_ConnectionsInfo.nConnIndex;

    pConnection = &m_ConnectionsMap[m_ConnectionsInfo.nConnIndex];

    pConnection->Init(pAddr, m_nSocketFD);

    bResult = true;
Exit0:
    return bResult;
}

void JUDPServer::RemoveConnection(int nConnIndex)
{
    JUDPConnection* pConnection = NULL;

    m_ConnectionsMapFind = m_ConnectionsMap.find(nConnIndex);
    JG_PROCESS_SUCCESS(m_ConnectionsMapFind == m_ConnectionsMap.end());

    pConnection = &m_ConnectionsMapFind->second;
    JGLOG_PROCESS_ERROR(pConnection);

    m_ConnectionsInfo.Addr = pConnection->m_ConnectionAddr;

    m_ConnectionsInfoSetFind = m_ConnectionsInfoSet.find(m_ConnectionsInfo);
    JGLOG_PROCESS_ERROR(m_ConnectionsInfoSetFind != m_ConnectionsInfoSet.end());

    m_ConnectionsInfoSet.erase(m_ConnectionsInfoSetFind);

    pConnection->UnInit();

    m_ConnectionsMap.erase(m_ConnectionsMapFind);

Exit1:
Exit0:
    return;
}

JUDPConnection* JUDPServer::GetConnection(int nConnIndex)
{
    JUDPConnection* pConnection = NULL;

    m_ConnectionsMapFind = m_ConnectionsMap.find(nConnIndex);
    JG_PROCESS_ERROR(m_ConnectionsMapFind != m_ConnectionsMap.end());

    pConnection = &m_ConnectionsMapFind->second;

Exit0:
    return pConnection;
}

void JUDPServer::ClearConnections()
{
    JUDPConnection* pConnection = NULL;

    for (m_ConnectionsMapFind = m_ConnectionsMap.begin(); m_ConnectionsMapFind != m_ConnectionsMap.end();)
    {
        pConnection = &m_ConnectionsMapFind->second;
        if (pConnection)
        {
            pConnection->UnInit();
        }

        m_ConnectionsMap.erase(m_ConnectionsMapFind++);
    }

    m_ConnectionsInfoSet.clear();
}

BOOL JUDPServer::GetConnIndex(int *pnConnIndex, sockaddr_in *pAddr)
{
    BOOL bResult = false;

    JGLOG_PROCESS_ERROR(pnConnIndex);
    JGLOG_PROCESS_ERROR(pAddr);

    m_ConnectionsInfo.Addr = *pAddr;

    m_ConnectionsInfoSetFind = m_ConnectionsInfoSet.find(m_ConnectionsInfo);
    JG_PROCESS_ERROR(m_ConnectionsInfoSetFind == m_ConnectionsInfoSet.end());

    *pnConnIndex = m_ConnectionsInfoSetFind->nConnIndex;

    bResult = true;
Exit0:
    return bResult;
}

BOOL JUDPServer::ProcessPackage()
{
    BOOL                      bResult       = false;
    BOOL                      bRetCode      = false;
    int                       nRetCode      = 0;
    IJG_Buffer*               piBuffer      = NULL;
    int                       nConnIndex    = 0;
    JUDPConnection*           pConnection   = NULL;
    UDP_PROTOCOL_HEADER*      pUDPHear      = NULL;
    int                       nAddrSize     = 0;
    PROCESS_UDP_PROTOCOL_FUNC Func          = NULL;
    size_t                    uDataSize     = 0;
    sockaddr_in               ConnAddr;

    JG_PROCESS_ERROR(m_bWorkFlag);

    nRetCode = Recv(&piBuffer, &ConnAddr, &nAddrSize);
    JG_PROCESS_SUCCESS(nRetCode == 2);
    if (nRetCode != 1)
    {
        JGLogPrintf(JGLOG_ERR, "[ProcessPackage] Recv failed, Error Code = %d", nRetCode);
        m_bWorkFlag = false;
        goto Exit0;
    }

    JGLOG_PROCESS_ERROR(piBuffer);

    bRetCode = GetConnIndex(&nConnIndex, &ConnAddr);
    if (!bRetCode)
    {
        bRetCode = AddConnection(&nConnIndex, &ConnAddr, nAddrSize);
        JGLOG_PROCESS_ERROR(bRetCode);
    }

    pConnection = GetConnection(nConnIndex);
    JGLOG_PROCESS_ERROR(pConnection);

    pUDPHear = (UDP_PROTOCOL_HEADER*)piBuffer->GetData();
    JGLOG_PROCESS_ERROR(pUDPHear);
    JGLOG_PROCESS_ERROR(pUDPHear->byUDPProtocol > euptUDPProtocolBegin && pUDPHear->byUDPProtocol < euptUDPProtocolEnd);

    uDataSize = piBuffer->GetSize();
    JGLOG_PROCESS_ERROR(uDataSize >= m_uUDPProtocolSize[pUDPHear->byUDPProtocol]);

    switch (pUDPHear->byUDPProtocol)
    {
    case euptUDPACK:
        pConnection->OnAckPacket((BYTE*)pUDPHear, uDataSize);
        goto Exit1;
        break;

    case euptUDPReliable:
        pConnection->OnUDPReliable((BYTE*)pUDPHear, uDataSize);
        break;
    }

    Func = m_ProcessUDPProtocolFunc[pUDPHear->byUDPProtocol];
    JGLOG_PROCESS_ERROR(Func);

    (this->*Func)(nConnIndex, (BYTE*)pUDPHear, uDataSize);

Exit1:
    bResult = true;
Exit0:
    if (!bResult)
    {
        if (m_bWorkFlag)
        {
            if (nConnIndex)
            {
                RemoveConnection(nConnIndex);
            }
        }
    }
    return bResult;
}

void JUDPServer::ProcessConnections()
{
    JG_PROCESS_ERROR(m_bWorkFlag);

    JUDPConnection* pConnection = NULL;
    for (m_ConnectionsMapFind = m_ConnectionsMap.begin(); m_ConnectionsMapFind != m_ConnectionsMap.end(); ++m_ConnectionsMapFind)
    {
        pConnection = &(m_ConnectionsMapFind->second);
        JGLOG_PROCESS_ERROR(pConnection);

        pConnection->Activate();
    }

Exit0:
    return;
}

void JUDPServer::OnUDPReliable(int nConnIndex, BYTE* pbyData, size_t uSize)
{

}

void JUDPServer::OnUDPUnreliable(int nConnIndex, BYTE* pbyData, size_t uSize)
{

}