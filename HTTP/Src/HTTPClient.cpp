#include "HTTP/HTTPClient.h"
#include "HTTPCommunication.h"

namespace Public {
namespace HTTP {

struct HTTPClientManager:public HTTPCommunicationHandler,public enable_shared_from_this<HTTPClientManager>
{
	shared_ptr<IOWorker>	worker;
	shared_ptr<Socket>		socket;

	shared_ptr<HTTPCommunication>	commu;
	shared_ptr<HTTPClientRequest>	request;
	shared_ptr<HTTPClientResponse>	response;

	std::string				useragent;

	bool					connecteddisconected;

	uint64_t				startconnecttime;
	std::string				responsesavefile;

	HTTPAsyncClient::HTTPCallback	asynccallback;

	HTTPClientManager(const shared_ptr<IOWorker>& _worker, const std::string& _useragent):worker(_worker),useragent(_useragent), connecteddisconected(false), startconnecttime(Time::getCurrentMilliSecond())
	{
		if (worker == NULL) worker = NULL;
	}
	~HTTPClientManager()
	{
		close();
		if (socket) socket->disconnect();
		socket = NULL;
		commu = NULL;
	}
	void close()
	{
		if (commu) commu->close();	
	}

	bool start(const shared_ptr<HTTPClientRequest>& req, const std::string & savefile)
	{
		responsesavefile = savefile;
		startconnecttime = Time::getCurrentMilliSecond();
		request = req;

		if (connecteddisconected)
		{
			socket = NULL;
			commu = NULL;
			connecteddisconected = false;
		}
		socket = TCPClient::create(worker);

		socket->async_connect(NetAddr(request->url().getHostname(), request->url().getPort(80)), Socket::ConnectedCallback(&HTTPClientManager::socketConnectCallback, this));

		return true;
	}

	void socketConnectCallback(const weak_ptr<Socket>& sock, bool status, const std::string& errmsg)
	{
		if (!status)
		{
			connecteddisconected = true;
			return;
		}
		if (!commu)
		{
			//HTTPCommunication(bool _isserver,const shared_ptr<Socket> & _sock,const shared_ptr<HTTPCommunicationHandler>& _handler,const std::string& _useragent)
			commu = make_shared<HTTPCommunication>(false,socket,shared_from_this(),useragent);
		}
		
		{
			shared_ptr<ReadContent> content = make_shared<ReadContent>(responsesavefile.length() == 0 ? HTTPCacheType_Mem : HTTPCacheType_File, responsesavefile);
			
			response = make_shared<HTTPClientResponse>(content);
		}

		commu->sendHeader = request->header();
		commu->sendContent = request->content();
		commu->recvHeader = response->header();
		commu->recvContent = response->content();
	}

	bool onRecvHeaderOk(HTTPCommunication* commu) { return true; }
	void onRecvContentOk(HTTPCommunication* commu) 
	{
	}
	void onDisconnect(HTTPCommunication* commu)
	{
		request->discallback()(request,"disconntend");
		connecteddisconected = true;
	}

	shared_ptr<HTTPClientResponse> errorResponse(uint32_t errcode, const std::string& errmsg)
	{
		shared_ptr<ReadContent> content = make_shared<ReadContent>(HTTPCacheType_Mem);
		shared_ptr<HTTPClientResponse> response = make_shared<HTTPClientResponse>(content);
		
		response->header()->statuscode = errcode;
		response->header()->statusmsg = errmsg;

		return response;
	}
};

struct HTTPClient::HTTPClientInternal
{
	shared_ptr<HTTPClientManager> manager;

	const shared_ptr<HTTPClientResponse> depend(uint32_t timeout)
	{
		uint64_t starttime = Time::getCurrentMilliSecond();

		while (1)
		{
			uint64_t nowtime = Time::getCurrentMilliSecond();

			if (nowtime - starttime >= timeout) return manager->errorResponse(408, "Request Timeout");

			if(manager->connecteddisconected) return manager->errorResponse(500, "Socket Disconnect");

			shared_ptr<HTTPCommunication> commu = manager->commu;
			if (commu)
			{
				commu->onPoolTimerProc();

				if (commu->sessionIsFinish()) break;
			}

			Thread::sleep(10);
		}

		return manager->response;
	}
};
HTTPClient::HTTPClient(const shared_ptr<IOWorker>& worker, const std::string& useragent)
{
	internal = new HTTPClientInternal;
	internal->manager = make_shared<HTTPClientManager>(worker, useragent);
}
HTTPClient::~HTTPClient()
{
	internal->manager = NULL;
	SAFE_DELETE(internal);
}

const shared_ptr<HTTPClientResponse> HTTPClient::request(const shared_ptr<HTTPClientRequest>& req, const std::string& saveasfile)
{
	internal->manager->start(req, saveasfile);

	return internal->depend(req->timeout());
}

struct HTTPAsyncClient::HTTPAsyncClientInternal
{
	shared_ptr<IOWorker>	worker;
	std::string				useragent;

	Mutex					mutex;
	std::map<HTTPClientManager*,shared_ptr< HTTPClientManager> > clientlist;

	shared_ptr<Timer>			timer;

	void onPoolTimerProc(unsigned long)
	{
		std::map<HTTPClientManager*, shared_ptr< HTTPClientManager> > clientlisttmp;

		{
			Guard locker(mutex);
			clientlisttmp = clientlist;
		}

		uint64_t nowtime = Time::getCurrentMilliSecond();
		for (std::map<HTTPClientManager*, shared_ptr< HTTPClientManager> >::iterator iter = clientlisttmp.begin(); iter != clientlisttmp.end(); iter++)
		{
			shared_ptr< HTTPClientManager> client = iter->second;

			if (client->commu)
				client->commu->onPoolTimerProc();

			if (client->commu && client->commu->sessionIsFinish())
			{
				client->asynccallback(client->request, client->response);

				Guard locker(mutex);
				clientlisttmp.erase(iter->first);

				continue;
			}

			if (nowtime > client->startconnecttime && nowtime - client->startconnecttime > client->request->timeout())
			{
				client->asynccallback(client->request, client->errorResponse(408, "Request Timeout"));

				Guard locker(mutex);
				clientlisttmp.erase(iter->first);

				continue;
			}

			if (client->connecteddisconected)
			{
				client->asynccallback(client->request, client->errorResponse(500, "Socket Disconnect"));

				Guard locker(mutex);
				clientlisttmp.erase(iter->first);

				continue;
			}
		}
	}
};

HTTPAsyncClient::HTTPAsyncClient(const shared_ptr<IOWorker>& worker, const std::string& useragent)
{
	internal = new HTTPAsyncClientInternal;
	internal->worker = worker;
	internal->useragent = useragent;

	if (internal->worker == NULL) internal->worker = IOWorker::defaultWorker();

	internal->timer = make_shared<Timer>("HTTPAsyncClient");
	internal->timer->start(Timer::Proc(&HTTPAsyncClientInternal::onPoolTimerProc, internal), 0, 1000);
}
HTTPAsyncClient::~HTTPAsyncClient()
{
	internal->timer = NULL;

	{
		Guard locker(internal->mutex);
		internal->clientlist.clear();
	}

	internal->worker = NULL;

	SAFE_DELETE(internal);
}
bool HTTPAsyncClient::request(const shared_ptr<HTTPClientRequest>& req, const HTTPCallback& callback, const std::string& saveasfile)
{
	shared_ptr<HTTPClientManager> client = make_shared<HTTPClientManager>(internal->worker,internal->useragent);
	client->asynccallback = callback;

	{
		Guard locker(internal->mutex);

		internal->clientlist[client.get()] = client;
	}

	return client->start(req, saveasfile);
}
	


}
}
