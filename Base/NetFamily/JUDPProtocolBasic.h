#ifndef _JUDP_PROTOCOL_BASIC_H_
#define _JUDP_PROTOCOL_BASIC_H_

#include "JBaseDef.h"

enum JUDP_PROTOCOL_TYPE
{
    eUDPProtocolBegin,

    eReliable,
    eUnreliable,

    eUDPProtocolEnd
};

#pragma pack(1)

struct UDP_PROTOCOL_HEADER
{
    BYTE       byUDPProtocol;
};

struct EXTERNAL_RELIABLE_PROTOCOL_HEADER : UDP_PROTOCOL_HEADER
{
    DWORD      dwProtocolID;
    uint16_t   byProtocolID;
};

struct EXTERNAL_UNRELIABLE_PROTOCOL_HEADER : UDP_PROTOCOL_HEADER
{
    uint16_t    byProtocolID;       //–≠“È∫≈
};

#pragma pack()

#endif // _JUDP_PROTOCOL_BASIC_H_