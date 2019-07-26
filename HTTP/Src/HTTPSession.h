#pragma once
#include "HTTP/HTTPPublic.h"
#include "HTTPProtocolBuilder.h"
#include "HTTPProtocolParser.h"
namespace Public {
namespace HTTP {

class HTTPSession
{
public:
	HTTPSession() {}
	virtual ~HTTPSession() {}

	virtual const char* inputData(const char* buftmp, int len) = 0;
	virtual bool isFinish() = 0;
	virtual bool requestFinish() { return false; }
	virtual void onPoolTimerProc() = 0;
	virtual void setResponse(const shared_ptr<HTTPResponse>& resp){}
	virtual uint64_t prevAliveTime() = 0;
public:
	shared_ptr<HTTPRequest>		request;
	shared_ptr<HTTPResponse>	response;
};

class HTTPSession_Client:public HTTPSession
{
	shared_ptr<HTTPProtocolBuilder<HTTPRequest> > builder;
	shared_ptr<HTTPProtocolParser<HTTPResponse> > parser;
public:
	HTTPSession_Client(const shared_ptr<HTTPRequest>& req, const shared_ptr<Socket>& _sock, const std::string& _useragent)
	{
		request = req;
		response = make_shared<HTTPResponse>();

		builder = make_shared<HTTPProtocolBuilder<HTTPRequest> >(request,_sock, _useragent);
		parser = make_shared<HTTPProtocolParser<HTTPResponse> >(response, HTTPProtocolParser<HTTPResponse>::HeaderParseSuccessCallback());
	}
	~HTTPSession_Client() {}

	virtual const char* inputData(const char* buftmp, int len)
	{
        shared_ptr<HTTPProtocolParser<HTTPResponse> > tmpParser = parser;
        if (tmpParser == NULL)
        {
            return NULL;
        }

        return tmpParser->inputData(buftmp, len);
	}
	virtual bool isFinish()
	{
        shared_ptr<HTTPProtocolParser<HTTPResponse> > tmpParser = parser;
        shared_ptr<HTTPProtocolBuilder<HTTPRequest> > tmpBuilder = builder;
        if (tmpParser == NULL || tmpBuilder == NULL)
        {
            return false;
        }

		return tmpBuilder->isFinish() && tmpParser->isFinish();
	}
	virtual void onPoolTimerProc()
	{
        shared_ptr<HTTPProtocolBuilder<HTTPRequest> > tmpBuilder = builder;
        if (tmpBuilder != NULL)
        {
            tmpBuilder->onPoolTimerProc();
        }
	}
	virtual uint64_t prevAliveTime()
	{
        shared_ptr<HTTPProtocolParser<HTTPResponse> > tmpParser = parser;
        shared_ptr<HTTPProtocolBuilder<HTTPRequest> > tmpBuilder = builder;
        if (tmpParser == NULL || tmpBuilder == NULL)
        {
            return 0;
        }

		return max(tmpBuilder->prevAliveTime(), tmpParser->prevAliveTime());
	}
};

//HTTP服务的使用close模式
#define HTTPSERVERUSECLOSESESSION

class HTTPSession_Service:public HTTPSession
{
	shared_ptr<HTTPProtocolBuilder<HTTPResponse> > builder;
	shared_ptr<HTTPProtocolParser<HTTPRequest> > parser;
	weak_ptr<Socket>					sock;
	std::string							useragent;
public:
	HTTPSession_Service(const shared_ptr<Socket>& _sock, const std::string& _useragent, const HTTPProtocolParser<HTTPRequest>::HeaderParseSuccessCallback& callback)
	{
		request = make_shared<HTTPRequest>();
		parser = make_shared<HTTPProtocolParser<HTTPRequest> >(request,callback);

		sock = _sock;
		useragent = _useragent;
	}
	~HTTPSession_Service() 
    {
        builder = NULL;
        parser = NULL;
    }
	bool checkSocketIsOk()
	{
		shared_ptr<Socket> socktmp = sock.lock();

		return socktmp != NULL && socktmp->getStatus() == NetStatus_connected;
	}
	void setResponse(const shared_ptr<HTTPResponse>& resp)
	{
		response = resp;
		
		Value connection = request->header(CONNECTION);
#ifdef HTTPSERVERUSECLOSESESSION
		connection = CONNECTION_Close;
#else
        //取消CONNECTION_KeepAlive模式
		if (connection.empty()) connection = CONNECTION_Close;
#endif
		response->headers()[CONNECTION] = connection;

		builder = make_shared<HTTPProtocolBuilder<HTTPResponse> >(response, sock.lock(), useragent);
	}

	virtual const char* inputData(const char* buftmp, int len)
	{
		return parser->inputData(buftmp, len);
	}
	virtual bool isFinish()
	{
		shared_ptr<HTTPProtocolBuilder<HTTPResponse> > buildertmp = builder;
		if (buildertmp == NULL) return false;

		return buildertmp->isFinish(response.use_count()) && parser->isFinish();
	}
	virtual void onPoolTimerProc()
	{
		shared_ptr<HTTPResponse> tmp = response;
		if (tmp == NULL) return;

		//response 无人使用了，或者有数据了，表示可以发送了
		if (tmp.use_count() == 1 || tmp->content()->size() > 0)
		{
			shared_ptr<HTTPProtocolBuilder<HTTPResponse> > buildertmp = builder;

			if(buildertmp != NULL)	buildertmp->onPoolTimerProc();
		}
	}	
	virtual uint64_t prevAliveTime()
	{
		shared_ptr<HTTPProtocolBuilder<HTTPResponse> > buildertmp = builder;

		uint64_t buildtime = buildertmp == NULL ? 0 : buildertmp->prevAliveTime();

		return max(buildtime, parser->prevAliveTime());
	}
	virtual bool requestFinish() 
	{ 
		return parser->isFinish(); 
	}
};

}
}

