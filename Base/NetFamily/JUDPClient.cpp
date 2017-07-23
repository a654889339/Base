#include "JUDPClient.h"

#define REGISTER_UDP_PARSE_FUNC(ProtocolID, FuncName, ProtocolSize) \
{m_ProcessUDPProtocolFunc[ProtocolID] = FuncName;                   \
    m_uUDPProtocolSize[ProtocolID] = ProtocolSize;}


JUDPClient::JUDPClient()
{
    m_byLowByteVersion   = 1;
    m_byHightByteVersion = 1;
    m_bWorkFlag          = false;

    memset(m_ProcessUDPProtocolFunc, 0, sizeof(m_ProcessUDPProtocolFunc));
    memset(m_uUDPProtocolSize, 0, sizeof(m_uUDPProtocolSize));

    REGISTER_UDP_PARSE_FUNC(euptUDPReliable, &JUDPClient::OnUDPReliable, sizeof(EXTERNAL_RELIABLE_PROTOCOL_HEADER));
    REGISTER_UDP_PARSE_FUNC(euptUDPUnreliable, &JUDPClient::OnUDPUnreliable, sizeof(EXTERNAL_UNRELIABLE_PROTOCOL_HEADER));
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
    ProcessConnection();
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

BOOL JUDPClient::Send(BYTE* piBuffer, size_t uSize)
{
    BOOL bResult  = false;
    BOOL bRetCode = 0;

    JGLOG_PROCESS_ERROR(piBuffer);

    bRetCode = m_Connection.Send(piBuffer, uSize);
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
        JGLOG_PROCESS_ERROR(piBuffer);

        uDataSize = piBuffer->GetSize();
        JG_PROCESS_SUCCESS(uDataSize == 0);

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

void JUDPClient::ProcessConnection()
{
    BOOL                      bResult       = false;
    IJG_Buffer*               piBuffer      = NULL;
    UDP_PROTOCOL_HEADER*      pUDPHeader    = NULL;
    size_t                    uDataSize     = 0;
    PROCESS_UDP_PROTOCOL_FUNC Func          = NULL;

    JG_PROCESS_ERROR(m_bWorkFlag);

    m_Connection.Activate();

    JGLOG_PROCESS_ERROR(m_Connection.IsInvalid());

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

}

void JUDPClient::OnUDPUnreliable(BYTE* pbyData, size_t uSize)
{

}