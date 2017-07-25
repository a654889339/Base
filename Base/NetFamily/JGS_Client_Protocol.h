#ifndef _JGS_CLIENT_PROTOCOL_H_
#define _JGS_CLIENT_PROTOCOL_H_

#include "JUDPBaseDef.h"

enum JUDP_GS_CLIENT_RELIABLE_PROTOCOL_TYPE
{
    s2c_reliable_protocol_begin,

    s2c_reliable_test_request,

    s2c_reliable_protocol_end
};

enum JUDP_CLIENT_GS_RELIABLE_PROTOCOL_TYPE
{
    c2s_reliable_protocol_begin,

    c2s_reliable_test_respond,

    c2s_reliable_protocol_end
};

#pragma pack(1)

struct S2C_RELIABLE_TEST_REQUEST : EXTERNAL_RELIABLE_PROTOCOL_HEADER
{
    int nTestCount;
};

#pragma pack()

#pragma pack(1)

struct C2S_RELIABLE_TEST_RESPOND : EXTERNAL_RELIABLE_PROTOCOL_HEADER
{
    int nTestCount;
};

#pragma pack()

// ---------------------------------------------------------------------

enum JUDP_GS_CLIENT_UNRELIABLE_PROTOCOL_TYPE
{
    s2c_unreliable_protocol_begin,

    s2c_unreliable_protocol_end
};

enum JUDP_CLIENT_GS_UNRELIABLE_PROTOCOL_TYPE
{
    c2s_unreliable_protocol_begin,

    c2s_unreliable_protocol_end
};

#endif //_JGS_CLIENT_PROTOCOL_H_