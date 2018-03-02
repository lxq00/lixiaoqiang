#ifndef __SIMPLEIODEF_H__
#define __SIMPLEIODEF_H__


#ifdef WIN32
#include <winsock2.h>
typedef int socklen_t;


//是否支持IOCP模式
#define SIMPLEIO_SUPPORT_IOCP
//是否支持POLL模式
#define SIMPLEIO_SUPPORT_POLL
//是否支持SELECT模式
#define SIMPLEIO_SUPPORT_SELECT

#else

#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

#define closesocket close

//是否支持EPOLL模式
#define SIMPLEIO_SUPPORT_EPOLL

//是否支持POLL模式
#define SIMPLEIO_SUPPORT_POLL

//是否支持SELECT模式
#define SIMPLEIO_SUPPORT_SELECT

#endif

inline bool initNetwork()
{
	static bool initStatus = false;
	if (!initStatus)
	{
#ifdef WIN32
		WSADATA wsaData;
		WORD wVersionRequested;

		wVersionRequested = MAKEWORD(2, 2);
		int errorCode = WSAStartup(wVersionRequested, &wsaData);
		if (errorCode != 0)
		{
			return false;
		}

		if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
		{
			WSACleanup();
			return false;
		}
#else
		signal(SIGPIPE, SIG_IGN);
#endif
	}
	initStatus = true;
	return true;
}

inline void setSocketNoBlock(int fd, bool nonblock= true)
{
#ifdef WIN32
	{
		int ionbio = nonblock ? 1 : 0;
		ioctlsocket(fd, FIONBIO, (u_long*)&ionbio);
	}
#else
	{
		int flags = fcntl(fd, F_GETFL, 0);
		fcntl(fd, F_SETFL, nonblock ? (flags | O_NONBLOCK): (flags &~O_NONBLOCK));
	}
#endif
}

#include "Base/Base.h"
#include "Network/Socket.h"
using namespace Public::Base;
using namespace Public::Network;

#endif 