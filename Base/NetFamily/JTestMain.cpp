/*
#include "JTestUDPClient.h"

JTestUDPClient m_Client;

int main()
{
    BOOL bResult         = false;
    BOOL bRetCode        = false;
    BOOL bClientInitFlag = false;

    bRetCode = m_Client.Init();
    JGLOG_PROCESS_ERROR(bRetCode);
    bClientInitFlag = true;

    bRetCode = m_Client.Run();
    JGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    if (bClientInitFlag)
    {
        m_Client.UnInit();
        bClientInitFlag = false;
    }
    if (!bResult)
    {
        getchar();
    }
    return bResult;
}

/**/
#include "JTestUDPServer.h"

JTestUDPServer m_Server;

int main()
{
    BOOL bResult         = false;
    BOOL bRetCode        = false;
    BOOL bServerInitFlag = false;

    bRetCode = m_Server.Init();
    JGLOG_PROCESS_ERROR(bRetCode);
    bServerInitFlag = true;

    bRetCode = m_Server.Run();
    JGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    if (bServerInitFlag)
    {
        m_Server.UnInit();
        bServerInitFlag = false;
    }

    if (!bResult)
    {
        getchar();
    }
    return bResult;
}
/**/