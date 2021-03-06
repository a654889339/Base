#include "JUDPClient.h"

#define REGISTER_UDP_PARSE_FUNC(ProtocolID, FuncName, ProtocolSize) \
{m_ProcessUDPProtocolFunc[ProtocolID] = FuncName;                   \
    m_uUDPProtocolSize[ProtocolID] = ProtocolSize;}

#define REGISTER_RELIABLE_PARSE_FUNC(ProtocolID, FuncName, ProtocolSize) \
{m_ProcessReliableProtocolFunc[ProtocolID] = FuncName;                   \
    m_uReliableProtocolSize[ProtocolID] = ProtocolSize;}

#define REGISTER_UNRELIABLE_PARSE_FUNC(ProtocolID, FuncName, ProtocolSize) \
{m_ProcessUnreliableProtocolFunc[ProtocolID] = FuncName;                   \
    m_uUnreliableProtocolSize[ProtocolID] = ProtocolSize;}


JUDPClient::JUDPClient()
{
    m_byLowByteVersion   = 1;
    m_byHightByteVersion = 1;
    m_bWorkFlag          = false;

    memset(m_ProcessUDPProtocolFunc, 0, sizeof(m_ProcessUDPProtocolFunc));
    memset(m_uUDPProtocolSize, 0, sizeof(m_uUDPProtocolSize));

    REGISTER_UDP_PARSE_FUNC(euptUDPReliable, &JUDPClient::OnUDPReliable, sizeof(EXTERNAL_RELIABLE_PROTOCOL_HEADER));
    REGISTER_UDP_PARSE_FUNC(euptUDPUnreliable, &JUDPClient::OnUDPUnreliable, sizeof(EXTERNAL_UNRELIABLE_PROTOCOL_HEADER));

    // --------------------------------------------------------------------
    memset(m_ProcessReliableProtocolFunc, 0, sizeof(m_ProcessReliableProtocolFunc));
    memset(m_uReliableProtocolSize, 0, sizeof(m_uReliableProtocolSize));

    REGISTER_RELIABLE_PARSE_FUNC(s2c_reliable_test_request, &JUDPClient::OnReliableTestRequest, sizeof(S2C_RELIABLE_TEST_REQUEST));

    // --------------------------------------------------------------------
    memset(m_ProcessUnreliableProtocolFunc, 0, sizeof(m_ProcessUnreliableProtocolFunc));
    memset(m_uUnreliableProtocolSize, 0, sizeof(m_uUnreliableProtocolSize));

}

JUDPClient::~JUDPClient()
{

}

BOOL JUDPClient::Init()
{
    BOOL bResult  = false;
    BOOL bRetCode = false;

    bResult = true;
//Exit0:
    return bResult;
}

void JUDPClient::UnInit()
{
    Close();
}

void JUDPClient::Activate()
{
    ProcessPackage();
    ProcessSendPacket();
    ProcessRecvPacket();
    ProcessSendPacket();
}

BOOL JUDPClient::Connect(char* pszIP, int nPort, char* pszLocalIP,int nLocalPort)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;
    int  nRetCode = 0;
    LONG lRetCode = 1;

    WORD  wVersion = MAKEWORD(m_byHightByteVersion, m_byLowByteVersion);

    nRetCode = WSAStartup(wVersion, &m_WSAData);
    JGLOG_PROCESS_ERROR(nRetCode == 0);

    nRetCode = LOBYTE(m_WSAData.wVersion);
    JGLOG_PROCESS_ERROR(nRetCode == m_byLowByteVersion);

    nRetCode = HIBYTE(m_WSAData.wVersion);
    JGLOG_PROCESS_ERROR(nRetCode == m_byHightByteVersion);

    m_nSocketFD = socket(AF_INET, SOCK_DGRAM, 0);
    JGLOG_PROCESS_ERROR(m_nSocketFD != INVALID_SOCKET);

#ifdef WIN32
    memset(&m_ServerAddr, 0, sizeof(m_ServerAddr));
#else
    bzero(&m_ServerAddr, sizeof(m_ServerAddr));
#endif

    m_ServerAddr.sin_family           = AF_INET;
    m_ServerAddr.sin_addr.s_addr      = inet_addr(pszIP);
    m_ServerAddr.sin_port             = htons(nPort);

    m_LocalAddr.sin_family            = AF_INET;
    m_LocalAddr.sin_addr.s_addr       = inet_addr(pszLocalIP);
    m_LocalAddr.sin_port              = htons(nLocalPort);

    nRetCode = bind(m_nSocketFD, (sockaddr*)&m_LocalAddr, sizeof(m_LocalAddr));
    JGLOG_PROCESS_ERROR(nRetCode != -1);

    nRetCode = connect(m_nSocketFD, (sockaddr*)&m_ServerAddr, sizeof(m_ServerAddr));
    JGLOG_PROCESS_ERROR(nRetCode != -1);

    nRetCode = ioctlsocket(m_nSocketFD, FIONBIO, (unsigned long *)&lRetCode);//设置成非阻塞模式。
    JGLOG_PROCESS_ERROR(nRetCode != SOCKET_ERROR)

    bRetCode = m_Connection.Init(&m_ServerAddr, m_nSocketFD);
    JGLOG_PROCESS_ERROR(bRetCode);

    m_bWorkFlag = true;

    bResult = true;
Exit0:
    return bResult;
}

void JUDPClient::Close()
{
    m_bWorkFlag = false;
    m_Connection.UnInit();
    closesocket(m_nSocketFD);
}

int JUDPClient::Recv(IJG_Buffer **ppiRetBuffer)
{
    BOOL        bResult  = false;
    int         nRetCode = 0;
    IJG_Buffer *piBuffer = NULL;
    char*       pRecv    = NULL;

    JGLOG_PROCESS_ERROR(ppiRetBuffer);

    *ppiRetBuffer = NULL;

    nRetCode = recv(m_nSocketFD, m_iRecvBuffer, JUDP_MAX_DATA_SIZE, 0);
    if (nRetCode == SOCKET_ERROR)
    {
        nRetCode = WSAGetLastError();
        JG_PROCESS_SUCCESS(nRetCode == WSAEWOULDBLOCK);

        JGLOG_PROCESS_ERROR(nRetCode == WSAETIMEDOUT);
        JGLOG_PROCESS_ERROR(nRetCode == WSAENETDOWN);

        JGLogPrintf(JGLOG_ERR, "[JUDPClient] Recv Error Code: %d\n", nRetCode);
        goto Exit0;
    }

    piBuffer = JG_MemoryCreateBuffer(nRetCode);
    JGLOG_PROCESS_ERROR(piBuffer);

    pRecv = (char*)piBuffer->GetData();
    JGLOG_PROCESS_ERROR(pRecv);

    memcpy(pRecv, m_iRecvBuffer, nRetCode);

    *ppiRetBuffer = piBuffer;

    (*ppiRetBuffer)->AddRef();

Exit1:
    bResult = true;
Exit0:
    JG_COM_RELEASE(piBuffer);
    return bResult;
}

BOOL JUDPClient::Send(IJG_Buffer* piBuffer)
{
    BOOL bResult  = false;
    BOOL bRetCode = 0;

    JGLOG_PROCESS_ERROR(piBuffer);

    bRetCode = m_Connection.Send(piBuffer);
    JGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    if (!bResult)
    {
        m_bWorkFlag = false;
    }
    return bResult;
}

void JUDPClient::ProcessPackage()
{
    BOOL                      bResult    = false;
    BOOL                      bRetCode   = false;
    int                       nRecvCode  = 0;
    IJG_Buffer*               piBuffer   = NULL;
    size_t                    uDataSize  = 0;
    UDP_PROTOCOL_HEADER*      pUDPHeader = NULL;
    PROCESS_UDP_PROTOCOL_FUNC Func       = NULL;

    JG_PROCESS_ERROR(m_bWorkFlag);

    while (true)
    {
        JG_COM_RELEASE(piBuffer);

        bRetCode = Recv(&piBuffer);
        JGLOG_PROCESS_ERROR(bRetCode);
        JG_PROCESS_SUCCESS(piBuffer == NULL);

        uDataSize = piBuffer->GetSize();
        JGLOG_PROCESS_ERROR(uDataSize >= sizeof(UDP_PROTOCOL_HEADER));

        pUDPHeader = (UDP_PROTOCOL_HEADER*)piBuffer->GetData();
        JGLOG_PROCESS_ERROR(pUDPHeader);

        switch (pUDPHeader->byUDPProtocol)
        {
        case euptUDPACK:
            m_Connection.OnAckPacket((BYTE*)pUDPHeader, uDataSize);
            continue;
            break;

        case euptUDPReliable:
            m_Connection.OnUDPReliable((BYTE*)pUDPHeader, uDataSize);
            continue;
            break;
        }

        Func = m_ProcessUDPProtocolFunc[pUDPHeader->byUDPProtocol];
        JGLOG_PROCESS_ERROR(Func);

        (this->*Func)((BYTE*)pUDPHeader, uDataSize);
    }

Exit1:
    bResult = true;
Exit0:
    JG_COM_RELEASE(piBuffer);
    if (!bResult)
    {
        m_bWorkFlag = false;
    }
    return;
}

void JUDPClient::ProcessSendPacket()
{
    BOOL                      bResult       = false;
    int                       nRetCode      = 0;
    IJG_Buffer*               piBuffer      = NULL;

    JG_PROCESS_ERROR(m_bWorkFlag);

    while (true)
    {
        JG_COM_RELEASE(piBuffer);

        piBuffer = m_Connection.GetSendPacket();
        JG_PROCESS_SUCCESS(piBuffer == NULL);

        nRetCode = send(m_nSocketFD, (char *)piBuffer->GetData(), piBuffer->GetSize(), 0);
        JGLOG_PROCESS_ERROR(nRetCode == piBuffer->GetSize());
    }

Exit1:
    bResult = true;
Exit0:
    JG_COM_RELEASE(piBuffer);
    if (!bResult)
    {
        m_bWorkFlag = false;
    }
    return;
}

void JUDPClient::ProcessRecvPacket()
{
    BOOL                      bResult       = false;
    IJG_Buffer*               piBuffer      = NULL;
    UDP_PROTOCOL_HEADER*      pUDPHeader    = NULL;
    size_t                    uDataSize     = 0;
    PROCESS_UDP_PROTOCOL_FUNC Func          = NULL;

    JG_PROCESS_ERROR(m_bWorkFlag);

    m_Connection.Activate();

    JGLOG_PROCESS_ERROR(m_Connection.IsEnable());

    while (true)
    {
        JG_COM_RELEASE(piBuffer);

        piBuffer = m_Connection.GetRecvPacket();
        JG_PROCESS_SUCCESS(piBuffer == NULL);

        pUDPHeader = (UDP_PROTOCOL_HEADER*)piBuffer->GetData();
        JGLOG_PROCESS_ERROR(pUDPHeader);
        JGLOG_PROCESS_ERROR(pUDPHeader->byUDPProtocol == euptUDPReliable);

        uDataSize = piBuffer->GetSize();
        JGLOG_PROCESS_ERROR(uDataSize >= m_uUDPProtocolSize[pUDPHeader->byUDPProtocol]);

        Func = m_ProcessUDPProtocolFunc[pUDPHeader->byUDPProtocol];
        JGLOG_PROCESS_ERROR(Func);

        (this->*Func)((BYTE*)pUDPHeader, uDataSize);
    }

Exit1:
    bResult = true;
Exit0:
    JG_COM_RELEASE(piBuffer);
    if (!bResult)
    {
        m_bWorkFlag = false;
    }
    return;
}

void JUDPClient::OnUDPReliable(BYTE* pbyData, size_t uSize)
{
    EXTERNAL_RELIABLE_PROTOCOL_HEADER* pReliable = (EXTERNAL_RELIABLE_PROTOCOL_HEADER *)pbyData;
    PROCESS_RELIABLE_PROTOCOL_FUNC     Func      = NULL;

    JGLOG_PROCESS_ERROR(pReliable);

    JGLOG_PROCESS_ERROR(pReliable->byProtocolID > s2c_reliable_protocol_begin);
    JGLOG_PROCESS_ERROR(pReliable->byProtocolID < s2c_reliable_protocol_end);

    if (m_uReliableProtocolSize[pReliable->byProtocolID] != UNDEFINED_PROTOCOL_SIZE)
    {
        JGLOG_PROCESS_ERROR(uSize == m_uReliableProtocolSize[pReliable->byProtocolID]);
    }

    Func = m_ProcessReliableProtocolFunc[pReliable->byProtocolID];
    JGLOG_PROCESS_ERROR(Func);

    (this->*Func)(pbyData, uSize);

Exit0:
    return;
}

void JUDPClient::OnReliableTestRequest(BYTE* pbyData, size_t uSize)
{
    S2C_RELIABLE_TEST_REQUEST* pRequest = (S2C_RELIABLE_TEST_REQUEST *)pbyData;

    JGLogPrintf(JGLOG_INFO, "[OnReliableTestRequest] %d\n", pRequest->nTestCount);

    DoReliableTestRespond(pRequest->nTestCount);
}

BOOL JUDPClient::DoReliableTestRespond(int nTestCount)
{
    BOOL                       bResult      = false;
    BOOL                       bRetCode     = false;
    IJG_Buffer*                piBuffer     = NULL;
    C2S_RELIABLE_TEST_RESPOND* pRespond     = NULL;

    piBuffer = JG_MemoryCreateBuffer(sizeof(C2S_RELIABLE_TEST_RESPOND));
    JGLOG_PROCESS_ERROR(piBuffer);

    pRespond = (C2S_RELIABLE_TEST_RESPOND *)piBuffer->GetData();
    JGLOG_PROCESS_ERROR(pRespond);

    pRespond->byUDPProtocol = euptUDPReliable;
    pRespond->byProtocolID  = c2s_reliable_test_respond;
    pRespond->nTestCount    = nTestCount;

    bRetCode = Send(piBuffer);
    JGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    JG_COM_RELEASE(piBuffer);
    return bResult;
}

// --------------------------------------------------------------------------
void JUDPClient::OnUDPUnreliable(BYTE* pbyData, size_t uSize)
{

}