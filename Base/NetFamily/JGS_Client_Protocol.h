#ifndef _JGS_CLIENT_PROTOCOL_H_
#define _JGS_CLIENT_PROTOCOL_H_

#include "JUDPBaseDef.h"

enum JUDP_GS_CLIENT_PROTOCOL_TYPE
{
    s2c_protocol_begin,

    s2c_test_request,

    s2c_protocol_end
};

enum JUDP_CLIENT_GS_PROTOCOL_TYPE
{
    c2s_protocol_begin,

    c2s_test_respond,

    c2s_protocol_end
};

#pragma pack(1)

struct S2C_TEST_REQUEST : EXTERNAL_RELIABLE_PROTOCOL_HEADER
{
    int nTestCount;
};

#pragma pack()

#pragma pack(1)

struct C2S_TEST_RESPOND : EXTERNAL_RELIABLE_PROTOCOL_HEADER
{
    int nTestCount;
};

#pragma pack()

#endif //_JGS_CLIENT_PROTOCOL_H_