#include "JUDPClient.h"

#define JUDP_TEST_SERVER_IP   "10.20.81.60"
#define JUDP_TEST_SERVER_PORT 8000

#define JUDP_TEST_CLIENT_IP     "10.20.81.60"
#define JUDP_TEST_CLIENT_PORT   9000

JUDPClient m_Client;


BOOL Run()
{
    BOOL bResult         = false;
    BOOL bRetCode        = false;



    bResult = true;
Exit0:
    return bResult;
}

BOOL Init()
{
    BOOL bResult         = false;
    BOOL bRetCode        = false;
    BOOL bClientInitFlag = false;
    BOOL bSocketInitFlag = false;

    bRetCode = m_Client.Init();
    JGLOG_PROCESS_ERROR(bRetCode);
    bClientInitFlag = true;

    bRetCode = m_Client.Connect(JUDP_TEST_SERVER_IP, JUDP_TEST_SERVER_PORT, JUDP_TEST_CLIENT_IP, JUDP_TEST_CLIENT_PORT);
    JGLOG_PROCESS_ERROR(bRetCode);
    bSocketInitFlag = true;

    bResult = true;
Exit0:
    if (!bResult)
    {
        if (bSocketInitFlag)
        {
            m_Client.Close();
            bSocketInitFlag = false;
        }
        if (bClientInitFlag)
        {
            m_Client.UnInit();
            bClientInitFlag = false;
        }
    }
    return bResult;
}

int main()
{
    BOOL bResult         = false;
    BOOL bRetCode        = false;
    BOOL bClientInitFlag = false;

    bRetCode = Init();
    JGLOG_PROCESS_ERROR(bRetCode);
    bClientInitFlag = true;

    bRetCode = Run();
    JGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    if (bClientInitFlag)
    {
        m_Client.Close();
        bClientInitFlag = false;
    }
    if (!bResult)
    {
        getchar();
    }
    return bResult;
}