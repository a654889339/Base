#ifndef _JUDP_SERVER_H_
#define _JUDP_SERVER_H_

#include "JBaseDef.h"

class JUDPServer
{
public:
    JUDPServer();
    virtual ~JUDPServer();

    BOOL Init();
    void UnInit();
};

#endif // _JUDP_SERVER_H_