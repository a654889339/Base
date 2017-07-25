#include "JTestUDPClient.h"
#include "JG_Memory.h"
#include "JGS_Client_Protocol.h"

JTestUDPClient::JTestUDPClient()
{
}

JTestUDPClient::~JTestUDPClient()
{

}

BOOL JTestUDPClient::Init()
{
    BOOL bResult         = false;
    BOOL bRetCode        = false;
    BOOL bClientInitFlag = false;

    bRetCode = m_Client.Init();
    JGLOG_PROCESS_ERROR(bRetCode);
    bClientInitFlag = true;

    bRetCode = m_Client.Connect(JUDP_TEST_SERVER_IP, JUDP_TEST_SERVER_PORT, JUDP_TEST_CLIENT_IP, JUDP_TEST_CLIENT_PORT);
    JGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    if (!bResult)
    {
        if (bClientInitFlag)
        {
            m_Client.UnInit();
            bClientInitFlag = false;
        }
    }
    return bResult;
}

void JTestUDPClient::UnInit()
{

}

BOOL JTestUDPClient::Run()
{
    BOOL              bResult      = false;
    BOOL              bRetCode     = false;
    int               nSendCount   = 0;
    time_t            nTimeNow     = time(NULL);
    IJG_Buffer*       piBuffer     = NULL;
    C2S_RELIABLE_TEST_RESPOND* pRespond     = NULL;

    while (true)
    {
        nTimeNow = time(NULL);

        m_Client.Activate();

        JG_COM_RELEASE(piBuffer);

        piBuffer = JG_MemoryCreateBuffer(sizeof(C2S_RELIABLE_TEST_RESPOND));
        JGLOG_PROCESS_ERROR(piBuffer);

        pRespond = (C2S_RELIABLE_TEST_RESPOND *)piBuffer->GetData();
        JGLOG_PROCESS_ERROR(pRespond);

        pRespond->byUDPProtocol = euptUDPReliable;
        pRespond->byProtocolID  = c2s_reliable_test_respond;
        pRespond->nTestCount    = ++nSendCount;

        bRetCode = m_Client.Send(piBuffer);
        JGLOG_PROCESS_ERROR(bRetCode);

        JGThread_Sleep(10);
    }

    bResult = true;
Exit0:
    JG_COM_RELEASE(piBuffer);
    return bResult;
}