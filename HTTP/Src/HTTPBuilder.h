#pragma once
#include "HTTP/HTTPPublic.h"
#include "HTTPDefine.h"
namespace Public {
namespace HTTP {


#define CONNECTION	"Connection"
#define CONNECTION_Close	"Close"
#define CONNECTION_KeepAlive	"keep-alive"

template<typename OBJECT>
class HTTPBuilder
{
	bool					 headerIsBuild;
	std::string				 readstring;
	int						 readpos;
	weak_ptr<Socket>		 sock;
	std::string				 useragent;
public:
	HTTPBuilder(const shared_ptr<Socket>& _sock, const std::string& _useragent):headerIsBuild(false), readpos(0),sock(_sock),useragent(_useragent){}
	virtual ~HTTPBuilder() {}
	void readBufferCallback(HTTPBuffer*, const char* buf, int len)
	{
		if (!headerIsBuild)
		{
			buildHeaderAndsend();
		}
		sendBuffer(buf, len);
	}
	void startasyncsend(const shared_ptr<Socket>&,const char* ,int len)
	{
		if (!headerIsBuild)
		{
			buildHeaderAndsend();
		}

		OBJECT* object = getObject();
		if (object == NULL) return;

		readstring = object->content()->read(readpos, MAXRECVBUFFERLEN);
		if (readstring.length() > 0)
		{
			readpos += readstring.length();
			beginSendBuffer(readstring.c_str(), readstring.length(), Socket::SendedCallback(&HTTPBuilder::startasyncsend, this));
		}
	}
private:
	virtual std::string buildFirstLine() = 0;
	virtual OBJECT* getObject() = 0;
	virtual bool sendBuffer(const char* buffer, int len)
	{
		shared_ptr<Socket> socket = sock.lock();
		if (socket != NULL)
		{
			int times = 0;
			while (len > 0 && times++ <= 1000)
			{
				int sendlen  = socket->send(buffer, len);
				if (sendlen > 0)
				{
					len -= sendlen;
					buffer += sendlen;
				}
				else
				{
					Thread::sleep(10);
				}
			}
		}

		return true;
	}
	virtual void beginSendBuffer(const char* buffer, int len, const Socket::SendedCallback& callback)
	{
		shared_ptr<Socket> socket = sock.lock();
		if (socket != NULL)
		{
			socket->async_send(buffer, len,callback);
		}
	}
	bool buildHeaderAndsend()
	{
		std::string str = buildFirstLine();

		OBJECT* object = getObject();
		if (object == NULL) return false;
		
		std::map<std::string, Value> headerstmp = object->headers();
		int contentlen = object->content()->size();
		if (contentlen != -1)
		{
			bool haveFind = false;
			for (std::map<std::string, Value>::iterator iter = headerstmp.begin(); iter != headerstmp.end(); iter++)
			{
				if (strcasecmp(iter->first.c_str(), Content_Length) == 0)
				{
					iter->second = contentlen;
					haveFind = true;
					break;
				}
			}

			if(!haveFind)	headerstmp[Content_Length] = contentlen;
		}
		//如果是流模式，那么connection自动切换为close方式
		if(object->content()->writetype() == HTTPBuffer::HTTPBufferType_Stream)
		{
			object->headers()[CONNECTION] = CONNECTION_Close;
		}
		if (useragent != "")
		{
			headerstmp["User-Agent"] = useragent;
		}

		for (std::map<std::string, Value>::iterator iter = headerstmp.begin();iter != headerstmp.end();iter ++)
		{
			str += iter->first + ": " + iter->second.readString() + HTTPHREADERLINEEND;
		}
		str += HTTPHREADERLINEEND;

		sendBuffer(str.c_str(), str.length());

		headerIsBuild = true;

		return true;
	}
};

}
}

