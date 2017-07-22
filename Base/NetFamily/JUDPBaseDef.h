#ifndef _JUDP_BASE_DEF_H_
#define _JUDP_BASE_DEF_H_

#include "JBaseDef.h"

#ifdef WIN32
//#include <winsock.h>
#include "winsock2.h"
#pragma comment(lib, "ws2_32.lib")
#else
#include<unistd.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/select.h>
#endif

#define JUDP_MAX_DATA_SIZE 548
#define JUDP_WINDOW_DEFAULT_SIZE 128
#define JUDP_MAXRETRANS_LIMIT    5
#define JUDP_RETRANS_INTERVAL    3

enum JUDP_STATUS_TYPE
{
    eustInvalid,

    eustEstablished,
    eustTimeout,
    eustClose,
    eustError
};

enum JUDP_PROTOCOL_TYPE
{
    euptUDPProtocolBegin,

    euptUDPReliable,
    euptUDPUnreliable,
    euptUDPACK,

    euptUDPProtocolEnd
};

#pragma pack(1)

struct UDP_PROTOCOL_HEADER
{
    BYTE       byUDPProtocol;

    UDP_PROTOCOL_HEADER()
    {
        byUDPProtocol = euptUDPProtocolBegin;
    }
};

struct EXTERNAL_RELIABLE_PROTOCOL_HEADER : UDP_PROTOCOL_HEADER
{
    DWORD      dwPacketID;
    uint16_t   byProtocolID;

    EXTERNAL_RELIABLE_PROTOCOL_HEADER()
    {
        byUDPProtocol = euptUDPReliable;
        dwPacketID    = 0;
        byProtocolID  = 0;
    }
};

struct EXTERNAL_UNRELIABLE_PROTOCOL_HEADER : UDP_PROTOCOL_HEADER
{
    uint16_t    byProtocolID;       //–≠“È∫≈

    EXTERNAL_UNRELIABLE_PROTOCOL_HEADER()
    {
        byUDPProtocol = euptUDPUnreliable;
        byProtocolID  = 0;
    }
};

struct EXTERNAL_ACK_PROTOCOL : UDP_PROTOCOL_HEADER
{
    DWORD      dwPacketID;

    EXTERNAL_ACK_PROTOCOL()
    {
        byUDPProtocol = euptUDPACK;
        dwPacketID    = 0;
    }
};
#pragma pack()

#endif //_JUDP_BASE_DEF_H_