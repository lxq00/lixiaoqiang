#pragma once
#include "HTTP/HTTPPublic.h"
#include "HTTPDefine.h"
namespace Public {
namespace HTTP {

template<typename OBJECT>
struct buildFirstLine
{
	static std::string build(const shared_ptr<OBJECT>&obj) { return ""; }
};
template<>
struct buildFirstLine<HTTPRequest>
{
	static std::string build(const shared_ptr<HTTPRequest>&req)
	{
		std::string path = req->url().getPath();
		if (path == "") path = "/";
		return String::toupper(req->method()) + " " + path + " " + HTTPVERSION + HTTPHREADERLINEEND;
	}
};
template<>
struct buildFirstLine<HTTPResponse>
{
	static std::string build(const shared_ptr<HTTPResponse>&resp)
	{
		return std::string(HTTPVERSION) + " " + Value(resp->statusCode()).readString() + " " + resp->errorMessage() + HTTPHREADERLINEEND;
	}
};

template<typename OBJECT>
class HTTPBuilder
{
	Mutex					 mutex;
	bool					 headerIsBuild;
	int						 sendpos;
	weak_ptr<Socket>		 sock;
	std::string				 useragent;
	bool					 issending;
	int						 sendcontentlen;
	int						 sendheaderlen;
	weak_ptr<OBJECT>		 object;
	uint64_t				 socketUsedTime;

	char					 sendbuffercache[MAXRECVBUFFERLEN];
public:
	HTTPBuilder(const shared_ptr<OBJECT>& _obj,const shared_ptr<Socket>& _sock, const std::string& _useragent)
		: headerIsBuild(false), sendpos(0),sock(_sock),useragent(_useragent)
		, issending(false), sendcontentlen(-1), sendheaderlen(0),object(_obj), socketUsedTime(Time::getCurrentMilliSecond())
	{}
	virtual ~HTTPBuilder()
    {
        shared_ptr<Socket> socket = sock.lock();
        if (socket != NULL)
        {
            socket->disconnect();
        }
    }

	bool isFinish(uint32_t objectsize = 1)
	{
        Guard locker(mutex);
        uint64_t nowtime = Time::getCurrentMilliSecond();
        if (sendcontentlen == -1 && objectsize == 1)
        {
            return true;
        }
        else if (sendpos == sendcontentlen && objectsize == 1 && (nowtime < socketUsedTime ||nowtime - socketUsedTime >= 500))
        {
            return true;
        }
		return false;
	}

	void onPoolTimerProc()
	{
		checkSendCallback(sock, NULL, 0);
	}

	uint64_t  prevAliveTime() { return socketUsedTime; }
private:
	void checkSendCallback(const weak_ptr<Socket>&, const char* tmp, int len)
	{
		Guard locker(mutex);

		shared_ptr<OBJECT>  objecttmp = object.lock();
		shared_ptr<Socket> socket = sock.lock();
		if (objecttmp == NULL || socket == NULL) return;
        
		if (tmp != NULL && len >= 0) issending = false;

		if (issending) return;

		//需要将头的长度减下来
		if (len >= sendheaderlen && sendheaderlen > 0)
		{
			len -= sendheaderlen;
			sendheaderlen = 0;
		}

		sendpos += len;

		int sendlen = 0;
		if (!headerIsBuild)
		{
			sendlen = sendheaderlen = buildHeader(sendbuffercache,objecttmp,socket);
		}
		{
			int readlen = objecttmp->content()->read(sendbuffercache + sendlen, MAXRECVBUFFERLEN - sendlen);
			if (readlen > 0) sendlen += readlen;
		}
		if(sendlen > 0)
		{
			socketUsedTime = Time::getCurrentMilliSecond();
			socket->async_send(sendbuffercache, sendlen, Socket::SendedCallback(&HTTPBuilder::checkSendCallback, this));

			issending = true;
		}
	}
	int buildHeader(char* buffer,shared_ptr<OBJECT>& object, shared_ptr<Socket>& socket)
	{
		if (object->content()->writetype() == HTTPContent::HTTPContentType_Chunk)
		{
			object->headers()[Transfer_Encoding] = CHUNKED;
		}
		else
		{
            sendcontentlen = object->content()->size();
			object->headers()[Content_Length] = sendcontentlen;
		}

		if (useragent != "")
		{
			object->headers()["User-Agent"] = useragent;
		}

		std::string str = buildFirstLine<OBJECT>::build(object);
		for (std::map<std::string, Value>::iterator iter = object->headers().begin();iter != object->headers().end();iter ++)
		{
			str += iter->first + ": " + iter->second.readString() + HTTPHREADERLINEEND;
		}
		str += HTTPHREADERLINEEND;

		headerIsBuild = true;

		memcpy(buffer, str.c_str(), str.length());

		return (int)str.length();
	}
};

}
}

