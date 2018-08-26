#pragma once
#include "librtmp/rtmp-server.h"
#include "RTMP/RTMPServer.h"

namespace Public{
namespace RTMP {

class LibRTMPServer
{
public:
	struct RTMPDataInfo
	{
		RTMPDataInfo(const char* _buffer, int _maxlen)
		{
			if (_maxlen == 1)
			{
				int a = 0;
			}
			maxlen = _maxlen;
			buffer = new char[maxlen];
			memcpy(buffer, _buffer, maxlen);
		}
		~RTMPDataInfo()
		{
			delete[]buffer;
		}
		char* buffer;
		int maxlen;
	};

	struct ReqInfo
	{
		std::string app;
		std::string stream;
		std::string type;
		double start;
		double duration;
		bool reset;
	};

	typedef Function0<void> DisconnectCallback;
	typedef Function4<void, RTMPType, const void*, size_t, uint32_t> ReadCallback;
	typedef Function1<void, bool> PauseCallback;
	typedef Function1<void, uint32_t> SeekCallback;
	typedef Function3<void, LibRTMPServer*, bool,const ReqInfo&> PlayCallback;
private:
	Mutex					mutex;
	rtmp_server_t*			rtmp;
	shared_ptr<Socket>		sock;
	DisconnectCallback		disconnectcallback;
	ReadCallback			readcallback;
	PauseCallback			pausecallback;
	SeekCallback			seekcalblack;
	PlayCallback			playcallback;

	bool								 asyncSendingFlag;
	std::list<shared_ptr<RTMPDataInfo> > sendcache;

	void onSocketRecvCallback(const shared_ptr<Socket>& sock,const char* buffer, int len)
	{
		Guard locker(mutex);
		if(rtmp != NULL)
		{
			rtmp_server_input(rtmp, (const uint8_t*)buffer, len);
		}

		sock->async_recv(Socket::ReceivedCallback(&LibRTMPServer::onSocketRecvCallback, this));
	}
	void onSocketDisconnectCallback(const shared_ptr<Socket>& sock, const std::string& errorinfo)
	{
		disconnectcallback();
	}
	void asyncSendCallback(const shared_ptr<Socket>& /*sock*/, const char* tmp, int len)
	{
		{
			Guard locker(mutex);
			if (!asyncSendingFlag || sendcache.size() <= 0) return;

			shared_ptr<RTMPDataInfo> buf = sendcache.front();
			if (buf->buffer == tmp)
			{
				sendcache.pop_front();
			}

			asyncSendingFlag = false;
		}
		
		postSend();
	}
	void postSend()
	{
		shared_ptr<Socket> tmp = sock;
		Guard locker(mutex);
		if (asyncSendingFlag || sendcache.size() <= 0 || tmp == NULL) return;

		asyncSendingFlag = true;
		shared_ptr<RTMPDataInfo> buf = sendcache.front();

		tmp->async_send(buf->buffer, buf->maxlen, Socket::SendedCallback(&LibRTMPServer::asyncSendCallback,this));
	}
	int sendbuffer(const char* buffer, int len)
	{
		{
			Guard locker(mutex);
			sendcache.push_back(make_shared<RTMPDataInfo>(buffer,len));
		}

		postSend();

		return len;
	}
	static int rtmp_server_send(void* param, const void* header, size_t len, const void* data, size_t bytes)
	{
		LibRTMPServer* internal = (LibRTMPServer*)param;
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
public:
	static int rtmp_server_onplay(void* param, const char* app, const char* stream, double start, double duration, uint8_t reset)
	{
		LibRTMPServer* internal = (LibRTMPServer*)param;
		if (internal == NULL)
		{
			return -1;
		}

		ReqInfo info;
		info.app = app;
		info.stream = stream;
		info.start = start;
		info.duration = duration;
		info.reset = reset != 0;

		internal->playcallback(internal, true, info);
		return 0;
	}
	static int rtmp_server_onpublish(void* param, const char* app, const char* stream, const char* type)
	{
		LibRTMPServer* internal = (LibRTMPServer*)param;
		if (internal == NULL)
		{
			return -1;
		}

		ReqInfo info;
		info.app = app;
		info.stream = stream;
		info.type = type;

		internal->playcallback(internal, false, info);
		return 0;
	}
	static int rtmp_server_onpause(void* param, int pause, uint32_t ms)
	{
		LibRTMPServer* internal = (LibRTMPServer*)param;
		if (internal == NULL)
		{
			return -1;
		}

		internal->pausecallback(pause == 1);
		return 0;
	}

	static int rtmp_server_onseek(void* param, uint32_t ms)
	{
		LibRTMPServer* internal = (LibRTMPServer*)param;
		if (internal == NULL)
		{
			return -1;
		}

		internal->seekcalblack(ms);
		return 0;
	}
	static int rtmp_server_onaudio(void* param, const void* data, size_t bytes, uint32_t timestamp)
	{
		LibRTMPServer* internal = (LibRTMPServer*)param;
		if (internal == NULL)
		{
			return -1;
		}

		internal->readcallback(FlvTagAudio, data, bytes, timestamp);

		return 0;
	}

	static int rtmp_server_onvideo(void* param, const void* data, size_t bytes, uint32_t timestamp)
	{
		LibRTMPServer* internal = (LibRTMPServer*)param;
		if (internal == NULL)
		{
			return -1;
		}

		internal->readcallback(FlvTagVideo, data, bytes, timestamp);

		return 0;
	}

	static int rtmp_server_onscript(void* param, const void* data, size_t bytes, uint32_t timestamp)
	{
		LibRTMPServer* internal = (LibRTMPServer*)param;
		if (internal == NULL)
		{
			return -1;
		}

		internal->readcallback(FlvTagScript, data, bytes, timestamp);

		return 0;
	}

public:
	LibRTMPServer(const shared_ptr<Socket>& _sock,const PlayCallback& _playcallback)
	{
		sock = _sock;
		playcallback = _playcallback;
		asyncSendingFlag = false;

		struct rtmp_server_handler_t handler;
		memset(&handler, 0, sizeof(handler));
		handler.send = rtmp_server_send;
		handler.onplay = rtmp_server_onplay;
		handler.onpause = rtmp_server_onpause;
		handler.onseek = rtmp_server_onseek;
		handler.onpublish = rtmp_server_onpublish;
		handler.onscript = rtmp_server_onscript;
		handler.onvideo = rtmp_server_onvideo;
		handler.onaudio = rtmp_server_onaudio;

		rtmp = rtmp_server_create(this, &handler);

		sock->setDisconnectCallback(Socket::DisconnectedCallback(&LibRTMPServer::onSocketDisconnectCallback, this));
		sock->async_recv(Socket::ReceivedCallback(&LibRTMPServer::onSocketRecvCallback, this));
	}
	~LibRTMPServer()
	{
		{
			Guard locker(mutex);
			rtmp_server_destroy(rtmp);
			rtmp = NULL;
		}
		weak_ptr<Socket> tmp = sock;
		sock = NULL;
		while (tmp.lock() != NULL)
		{
			Thread::sleep(10);
		}
	}

	bool setDisconnect(const DisconnectCallback& discallback)
	{
		disconnectcallback = discallback;
		return true;
	}
	bool setPublishCallback(const ReadCallback& callback)
	{
		readcallback = callback;
		return true;
	}
	bool setServerCalblack(const PauseCallback& pcallback,const SeekCallback& scallback)
	{
		pausecallback = pcallback;
		seekcalblack = scallback;

		return true;
	}

	bool writePacket(RTMPType type, uint32_t timestmap /*ms*/, const char* buffer, int maxlen)
	{
		if (type == FlvTagAudio)
		{
			return rtmp_server_send_audio(rtmp, buffer, maxlen, timestmap) == 0;
		}
		else if (type == FlvTagVideo)
		{
			return rtmp_server_send_video(rtmp, buffer, maxlen, timestmap) == 0;
		}
		else
		{
			return rtmp_server_send_script(rtmp, buffer, maxlen, timestmap) == 0;
		}
	}
};


}
}
