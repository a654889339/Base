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

    BOOL Run();

private:
    JUDPServer m_Server;
};

#endif // _JTEST_UDP_SERVER_MAIN_H_