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
#define JUDP_SEND_CACHE_SIZE     5000
#define UNDEFINED_PROTOCOL_SIZE  -1

enum JUDP_STATUS_TYPE
{
    eustInvalid,

    eustEstablished,
    eustTimeout,
    eustClose,
    eustError,

    eustDisable
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
};

struct EXTERNAL_RELIABLE_PROTOCOL_HEADER : UDP_PROTOCOL_HEADER
{
    DWORD      dwPacketID;
    uint16_t   byProtocolID;
};

struct EXTERNAL_UNRELIABLE_PROTOCOL_HEADER : UDP_PROTOCOL_HEADER
{
    uint16_t    byProtocolID;       //–≠“È∫≈
};

struct EXTERNAL_ACK_PROTOCOL : UDP_PROTOCOL_HEADER
{
    DWORD      dwPacketID;
};
#pragma pack()

#endif //_JUDP_BASE_DEF_H_