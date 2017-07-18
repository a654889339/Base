#include "JTestUDPClient.h"
#include "JG_Memory.h"

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

void JTestUDPClient::UnInit()
{

}

BOOL JTestUDPClient::ProcessPackage()
{
    BOOL        bResult   = false;
    BOOL        bRetCode  = false;
    int         nRecvCode = 0;
    IJG_Buffer* piBuffer  = NULL;
    size_t      uDataSize = 0;

    while (true)
    {
        bRetCode = m_Client.Recv(&piBuffer);
        JGLOG_PROCESS_ERROR(bRetCode);

        JG_PROCESS_SUCCESS(piBuffer == NULL);

        uDataSize = piBuffer->GetSize();
        JG_PROCESS_SUCCESS(uDataSize == 0);
        JGLOG_PROCESS_ERROR(uDataSize == sizeof(int));

        nRecvCode = *((int*)piBuffer->GetData());

        JGLogPrintf(JGLOG_INFO, "[Package] %d\n", nRecvCode);

        JG_COM_RELEASE(piBuffer);
    }

Exit1:
    bResult = true;
Exit0:
    JG_COM_RELEASE(piBuffer);
    return bResult;
}

BOOL JTestUDPClient::Run()
{
    BOOL     bResult      = false;
    BOOL     bRetCode     = false;
    int      nSendCount   = 0;
    time_t   nTimeNow     = time(NULL);

    while (true)
    {
        nTimeNow = time(NULL);

        bRetCode = ProcessPackage();
        JGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = m_Client.Send((char*)&nSendCount, sizeof(int));
        JGLOG_PROCESS_ERROR(bRetCode);

        nSendCount++;

        JGThread_Sleep(10);
    }

    bResult = true;
Exit0:
    return bResult;
}
/*
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
*/