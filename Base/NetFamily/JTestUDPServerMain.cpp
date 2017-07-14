#include "JUDPServer.h"
#include <time.h>
#include <set>

#define JUDP_TEST_SERVER_IP   "10.20.81.60"
#define JUDP_TEST_SERVER_PORT 8000

struct JClientSocket
{
    sockaddr_in ClientAddr;
    int         nClientAddrSize;
    int         nConnIndex;

    JClientSocket()
    {
    #ifdef WIN32
        memset(&ClientAddr, 0, sizeof(ClientAddr));
    #else
        bzero(&ClientAddr, sizeof(ClientAddr));
    #endif
        nClientAddrSize = sizeof(ClientAddr);
        nConnIndex      = 0;
    }

    bool operator<(const JClientSocket& Sock) const
    {
        return ClientAddr.sin_addr.S_un.S_addr < Sock.ClientAddr.sin_addr.S_un.S_addr;
    }
};

typedef std::set<JClientSocket> JCLIENT_SOCKET_SET;
JCLIENT_SOCKET_SET              m_ClientSocketSet;

int        m_nClientCount;
JUDPServer m_Server;

BOOL ProcessPackage()
{
    BOOL          bResult         = false;
    BOOL          bRetCode        = false;
    IJG_Buffer*   piBuffer        = NULL;
    size_t        uDataSize       = 0;
    JClientSocket ClientSocket;

    while (true)
    {
        bRetCode = m_Server.Recv(piBuffer, &ClientSocket.ClientAddr, &ClientSocket.nClientAddrSize);
        JGLOG_PROCESS_ERROR(bRetCode);

        bRetCode = m_ClientSocketSet.count(ClientSocket);
        if (bRetCode)
        {

        }
        else
        {
            ClientSocket.nConnIndex = ++m_nClientCount;
            m_ClientSocketSet.insert(ClientSocket);
        }

        uDataSize = strlen((char*)piBuffer);

        JG_PROCESS_SUCCESS(uDataSize == 0);
        JGLOG_PROCESS_ERROR(uDataSize == sizeof(int));

        JGLogPrintf(JGLOG_INFO, "[Package] %d from %u\n", (int)&piBuffer, 
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

BOOL Run()
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

BOOL Init()
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

int main()
{
    BOOL bResult         = false;
    BOOL bRetCode        = false;
    BOOL bServerInitFlag = false;

    bRetCode = Init();
    JGLOG_PROCESS_ERROR(bRetCode);
    bServerInitFlag = true;

    bRetCode = Run();
    JGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    if (bServerInitFlag)
    {
        m_Server.Close();
        bServerInitFlag = false;
    }
    if (!bResult)
    {
        getchar();
    }
    return bResult;
}