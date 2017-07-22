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
    S2C_TEST_REQUEST    Request;

    while (true)
    {
        m_Server.Activate();

        ++nSendCount;

        Request.nTestCount = nSendCount;

        bRetCode = m_Server.Broadcast((BYTE *)&Request, sizeof(S2C_TEST_REQUEST));
        JGLOG_PROCESS_ERROR(bRetCode);

        JGThread_Sleep(10);
    }


    bResult = true;
Exit0:
    return bResult;
}
