#ifndef _JUDP_BASE_DEF_H_
#define _JUDP_BASE_DEF_H_

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

#endif //_JUDP_BASE_DEF_H_