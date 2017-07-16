#ifndef _JTEST_UDP_SERVER_MAIN_H_
#define _JTEST_UDP_SERVER_MAIN_H_

#include <time.h>
#include <set>
#include "JUDPServer.h"
#include "JTestUDPDef.h"

class JTestUDPServer
{
public:
    JTestUDPServer();
    virtual ~JTestUDPServer();

    BOOL Init();
    void UnInit();

    BOOL ProcessPackage();
    BOOL Run();

private:
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
};

#endif