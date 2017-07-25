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
    BOOL              bFirst       = true;
    while (true)
    {
        m_Client.Activate();
        JGThread_Sleep(10);

        if (bFirst)
        {
            bFirst = false;
            m_Client.DoReliableTestRespond(123); //随便发个包，不然服务器检测不到。
        }
    }

    bResult = true;
//Exit0:
    return bResult;
}