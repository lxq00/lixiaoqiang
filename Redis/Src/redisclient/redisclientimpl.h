
#ifndef REDISCLIENT_REDISCLIENTIMPL_H
#define REDISCLIENT_REDISCLIENTIMPL_H


#include "Base/Base.h"
#include "Network/Network.h"
using namespace Public::Base;
using namespace Public::Network;

#include "redisvalue.h"
#include "redisparser.h"

namespace redisclient {

typedef Function0<void> ErrorCallback;
typedef Function0<void> ConnectCallback;
typedef Function1<void, const RedisValue&> CmdCallback;

typedef CmdCallback SubcribeHandler;

class RedisClientImpl
{
public:
	RedisClientImpl(const shared_ptr<IOWorker>& worker);
	~RedisClientImpl();

	void asyncConnect(const NetAddr& addr, const ConnectCallback& callback,const ErrorCallback& errcallback);

	void* subscribe(const std::string &command, const std::string &channel, const SubcribeHandler&  subhandler, const CmdCallback& callback);

	void unsubscribe(const std::string &command, const std::string &channel, void* subid);

	void close();

	void doAsyncCommand(const std::string& cmdstr, const CmdCallback&  handler);

	bool connected() const;
private:
	void _socketSendCmd();
	void socketConnectCallback(const shared_ptr<Socket>& sock);
	void socketDisconnectCallback(const shared_ptr<Socket>& sock, const std::string&);
	void socketSendCallback(const shared_ptr<Socket>&s, const char* tmp, int len);
	void socketRecvCallback(const shared_ptr<Socket>&s, const char* tmp, int len);
	void doProcessMessage(const shared_ptr<RedisValue>& v);
private:
	struct CmdInfo
	{
		std::string		cmd;
		CmdCallback		handler;
	};

	struct SubcribeInfo
	{
		SubcribeHandler		handler;
	};

	Mutex																mutex;
	shared_ptr<IOWorker>												worker;
	shared_ptr<Socket>													sock;
	ErrorCallback														errcallback;
	ConnectCallback														connectCallback;
	std::list<shared_ptr<CmdInfo> >										cmdSendList;
	std::list<shared_ptr<CmdInfo> >										cmdRecvList;
	shared_ptr<RedisParser>												parser;
	std::map<std::string, std::map<void*, shared_ptr<SubcribeInfo> > >  sublist;
	NetAddr																redisaddr;
};
}

#endif // REDISCLIENT_REDISCLIENTIMPL_H
