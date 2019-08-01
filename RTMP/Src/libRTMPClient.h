#pragma once
#include "librtmp/rtmp-client.h"
#include "Base/Base.h"
using namespace Public::Base;

namespace Public{
namespace RTMP {

class LibRTMPClient
{
public:
	typedef Function0<void> DisconnectCallback;
	typedef Function4<void, RTMPType, const void*, size_t, uint32_t> ReadCallback;
	typedef Function2<void, bool, const std::string&> ConnectCallback;
private:
	std::string				url;
	rtmp_client_t*			rtmp;
	shared_ptr<Socket>		sock;
	DisconnectCallback		disconnectcallback;
	ReadCallback			readcallback;
	ConnectCallback			connectCallback;
	uint64_t				startConnecttime;
	uint32_t				connectTimeout;
	shared_ptr<Timer>		poolTimer;

	void poolTimerProc(unsigned long)
	{
		uint64_t nowtime = Time::getCurrentMilliSecond();

		if (nowtime > startConnecttime + connectTimeout)
		{
			connectCallback(false, "connect timeout");

			poolTimer->stop();
		}
	}
	void onSocketConnectCallback(const weak_ptr<Socket>& sock,bool status,const std::string& errmsg)
	{
		poolTimer = NULL;
		connectCallback(status, errmsg);
	}
	void onSocketRecvCallback(const weak_ptr<Socket>& sock,const char* buffer, int len)
	{
		shared_ptr<Socket> socktmp = sock.lock();
		if (socktmp == NULL) return;

		rtmp_client_input(rtmp, buffer, len);

		socktmp->async_recv(Socket::ReceivedCallback(&LibRTMPClient::onSocketRecvCallback, this));
	}
	void onSocketDisconnectCallback(const weak_ptr<Socket>& sock, const std::string& errorinfo)
	{
		disconnectcallback();
	}
	int sendbuffer(const char* buffer, int len)
	{
		int lentmp = len;
		int sendtimes = 0;

		shared_ptr<Socket> socktmp = sock;
		while (len > 0 && sendtimes++ < 1000 && socktmp != NULL)
		{
			int sendlen = socktmp->send(buffer, len);
			if (sendlen > 0)
			{
				buffer += sendlen;
				len -= sendlen;
			}
			else
			{
				Thread::sleep(10);
			}
		}
		if (len > 0)
		{
			assert(0);
		}


		return lentmp - len;
	}
	static int rtmp_client_send(void* param, const void* header, size_t len, const void* data, size_t bytes)
	{
		LibRTMPClient* internal = (LibRTMPClient*)param;
		if (internal == NULL)
		{
			return -1;
		}

		int sendlen = 0;
		if (header != NULL && len > 0)
		{
			int ret = internal->sendbuffer((const char*)header, len);
			if (ret <= 0) return -1;

			sendlen += ret;
		}
		if (data != NULL && bytes > 0)
		{
			int ret = internal->sendbuffer((const char*)data, bytes);
			if (ret <= 0) return -1;

			sendlen += ret;
		}

		return sendlen;
	}

	static int rtmp_client_onaudio(void* param, const void* data, size_t bytes, uint32_t timestamp)
	{
		LibRTMPClient* internal = (LibRTMPClient*)param;
		if (internal == NULL)
		{
			return -1;
		}

		internal->readcallback(FlvTagAudio, data, bytes, timestamp);

		return 0;
	}

	static int rtmp_client_onvideo(void* param, const void* data, size_t bytes, uint32_t timestamp)
	{
		LibRTMPClient* internal = (LibRTMPClient*)param;
		if (internal == NULL)
		{
			return -1;
		}

		internal->readcallback(FlvTagVideo, data, bytes, timestamp);

		return 0;
	}

	static int rtmp_client_onscript(void* param, const void* data, size_t bytes, uint32_t timestamp)
	{
		LibRTMPClient* internal = (LibRTMPClient*)param;
		if (internal == NULL)
		{
			return -1;
		}

		internal->readcallback(FlvTagScript, data, bytes, timestamp);

		return 0;
	}

public:
	LibRTMPClient(const std::string& _url, const shared_ptr<IOWorker>& worker, const std::string& useragent)
	{
		url = _url;


		struct rtmp_client_handler_t handler;
		memset(&handler, 0, sizeof(handler));
		handler.send = LibRTMPClient::rtmp_client_send;
		handler.onaudio = LibRTMPClient::rtmp_client_onaudio;
		handler.onvideo = LibRTMPClient::rtmp_client_onvideo;
		handler.onscript = LibRTMPClient::rtmp_client_onscript;


		const char* urltmp = url.c_str();
		const char* endtmp = strstr(urltmp, "://");

		if (endtmp == NULL) endtmp = urltmp;
		else endtmp += 3;

		const char* tmp1 = strchr(endtmp, '/');
		const char* streamurl = NULL;
		if (tmp1 == NULL) streamurl = tmp1 = endtmp;
		else streamurl = tmp1 += 1;

		rtmp = rtmp_client_create(useragent.c_str(), streamurl, std::string(urltmp, streamurl - urltmp).c_str(), this, &handler);
		sock = TCPClient::create(worker);
	}
	~LibRTMPClient()
	{
		sock = NULL;
		rtmp_client_destroy(rtmp);
	}

	bool connect(const DisconnectCallback& discallback, uint32_t timeout_ms)
	{
		disconnectcallback = discallback;
		startConnecttime = Time::getCurrentMilliSecond();
		connectTimeout = timeout_ms;
		sock->setSocketTimeout(timeout_ms, timeout_ms);

		URL url(url);
		NetAddr addr(url.hostname, url.port == 0 ? DEFAULTRTMPPORT : url.port);

		return sock->connect(addr);
	}
	bool connect(const ConnectCallback& callback, const DisconnectCallback& discallback, uint32_t timeout_ms)
	{
		disconnectcallback = discallback;
		startConnecttime = Time::getCurrentMilliSecond();
		connectCallback = callback;
		connectTimeout = timeout_ms;
		sock->setSocketTimeout(timeout_ms, timeout_ms);
		poolTimer = make_shared<Timer>("LibRTMPClient");
		poolTimer->start(Timer::Proc(&LibRTMPClient::poolTimerProc, this), 0, 100);

		URL uri(url);
		NetAddr addr(uri.hostname, uri.port == 0 ? DEFAULTRTMPPORT : uri.port);

		sock->async_connect(addr, Socket::ConnectedCallback(&LibRTMPClient::onSocketConnectCallback, this));

		return true;
	}
	bool start(bool pull,const ReadCallback& callback)
	{
		if (rtmp_client_start(rtmp, pull ? 1 : 0) != 0)
		{
			return false;
		}
		readcallback = callback;
		sock->setDisconnectCallback(Socket::DisconnectedCallback(&LibRTMPClient::onSocketDisconnectCallback, this));
		sock->async_recv(Socket::ReceivedCallback(&LibRTMPClient::onSocketRecvCallback, this));

		return true;
	}

	bool pause(bool pasue)
	{
		return rtmp_client_pause(rtmp, pasue ? 1 : 0) == 0;
	}

	bool seek(double timestamp_ms)
	{
		return rtmp_client_seek(rtmp, timestamp_ms) == 0;
	}
	bool writePacket(RTMPType type, uint32_t timestmap /*ms*/, const char* buffer, int maxlen)
	{
		if (type == FlvTagAudio)
		{
			return rtmp_client_push_audio(rtmp, buffer, maxlen, timestmap) == 0;
		}
		else if (type == FlvTagVideo)
		{
			return rtmp_client_push_video(rtmp, buffer, maxlen, timestmap) == 0;
		}
		else
		{
			return rtmp_client_push_script(rtmp, buffer, maxlen, timestmap) == 0;
		}
	}
};


}
}
