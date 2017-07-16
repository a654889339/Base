#ifndef _JTEST_UDP_CLIENT_H_
#define _JTEST_UDP_CLIENT_H_

#include <time.h>
#include "JTestUDPDef.h"
#include "JUDPClient.h"

class JTestUDPClient
{
public:
    JTestUDPClient();
    virtual ~JTestUDPClient();

    BOOL Init();
    void UnInit();

    BOOL ProcessPackage();
    BOOL Run();

private:
    JUDPClient m_Client;
};

#endif // _JTEST_UDP_CLIENT_H_
