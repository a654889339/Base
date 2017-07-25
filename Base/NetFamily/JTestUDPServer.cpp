#include "JTestUDPServer.h"
#include "JGS_Client_Protocol.h"

JTestUDPServer::JTestUDPServer()
{
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

BOOL JTestUDPServer::Run()
{
    BOOL                bResult         = false;
    BOOL                bRetCode        = false;
    int                 nSendCount      = 0;
    IJG_Buffer*         piBuffer        = NULL;
    S2C_RELIABLE_TEST_REQUEST*   pRequest        = NULL;

    while (true)
    {
        m_Server.Activate();

        JG_COM_RELEASE(piBuffer);

        piBuffer = JG_MemoryCreateBuffer(sizeof(S2C_RELIABLE_TEST_REQUEST));
        JGLOG_PROCESS_ERROR(piBuffer);

        pRequest = (S2C_RELIABLE_TEST_REQUEST *)piBuffer->GetData();
        JGLOG_PROCESS_ERROR(pRequest);

        pRequest->byUDPProtocol = euptUDPReliable;
        pRequest->byProtocolID  = s2c_reliable_test_request;
        pRequest->nTestCount    = ++nSendCount;

        bRetCode = m_Server.Broadcast(piBuffer);
        JGLOG_PROCESS_ERROR(bRetCode);

        JGThread_Sleep(10);
    }

    bResult = true;
Exit0:
    JG_COM_RELEASE(piBuffer);
    return bResult;
}
