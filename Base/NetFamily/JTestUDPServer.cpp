#include "JTestUDPServer.h"

JTestUDPServer::JTestUDPServer()
{
    m_nClientCount = 0;
}

JTestUDPServer::~JTestUDPServer()
{

}

BOOL JTestUDPServer::Init()
{
    BOOL bResult         = false;
    BOOL bRetCode        = false;
    BOOL bServerInitFlag = false;
    BOOL bSocketInitFlag = false;

    bRetCode = m_Server.Init();
    JGLOG_PROCESS_ERROR(bRetCode);
    bServerInitFlag = true;

    bRetCode = m_Server.Listen(JUDP_TEST_SERVER_IP, JUDP_TEST_SERVER_PORT);
    JGLOG_PROCESS_ERROR(bRetCode);
    bSocketInitFlag = true;

    bResult = true;
Exit0:
    if (!bResult)
    {
        if (bSocketInitFlag)
        {
            m_Server.Close();
            bSocketInitFlag = false;
        }
        if (bServerInitFlag)
        {
            m_Server.UnInit();
            bServerInitFlag = false;
        }
    }
    return bResult;
}

void JTestUDPServer::UnInit()
{
    m_Server.UnInit();
}

BOOL JTestUDPServer::ProcessPackage()
{
    BOOL          bResult         = false;
    BOOL          bRetCode        = false;
    IJG_Buffer*   piBuffer        = NULL;
    size_t        uDataSize       = 0;
    int           nRecvCode       = 0;
    JClientSocket ClientSocket;

    for (int i = 0; i < 5; i++)
    {
        piBuffer = m_Server.Recv(&uDataSize, &ClientSocket.ClientAddr, &ClientSocket.nClientAddrSize);
        JGLOG_PROCESS_ERROR(piBuffer);

        bRetCode = m_ClientSocketSet.count(ClientSocket);
        if (bRetCode)
        {

        }
        else
        {
            ClientSocket.nConnIndex = ++m_nClientCount;
            m_ClientSocketSet.insert(ClientSocket);
        }

        JG_PROCESS_SUCCESS(uDataSize == 0);
        JGLOG_PROCESS_ERROR(uDataSize == sizeof(int));

        nRecvCode = (int)*piBuffer;

        JGLogPrintf(JGLOG_INFO, "[Package] %d from %u\n", nRecvCode, 
            ClientSocket.ClientAddr.sin_addr.S_un.S_addr
        );

        JG_COM_RELEASE(piBuffer);
    }

Exit1:
    bResult = true;
Exit0:
    JG_COM_RELEASE(piBuffer);
    return bResult;
}

BOOL JTestUDPServer::Run()
{
    BOOL          bResult         = false;
    BOOL          bRetCode        = false;
    int           nSendCount      = 0;
    time_t        nTimeNow        = time(NULL);
    JClientSocket ClientSocket;

    m_ClientSocketSet.clear();
    m_nClientCount = 0;

    while (true)
    {
        nTimeNow = time(NULL);

        bRetCode = ProcessPackage();
        JGLOG_PROCESS_ERROR(bRetCode);

        for (JCLIENT_SOCKET_SET::iterator itFind = m_ClientSocketSet.begin(); itFind != m_ClientSocketSet.end(); ++itFind)
        {
            ClientSocket = *itFind;

            bRetCode = m_Server.Send((IJG_Buffer*)&nSendCount, sizeof(int), &ClientSocket.ClientAddr, ClientSocket.nClientAddrSize);
            JGLOG_PROCESS_ERROR(bRetCode);

            nSendCount++;
        }

        JGThread_Sleep(1000);
    }


    bResult = true;
Exit0:
    return bResult;
}
