#include "redissyncclientimpl.h"
namespace redisclient {

RedisSyncClientImpl::RedisSyncClientImpl(const shared_ptr<IOWorker>& _worker):worker(_worker)
{}

RedisSyncClientImpl::~RedisSyncClientImpl()
{
    close();
}

void RedisSyncClientImpl::close()
{
	weak_ptr<Socket> socktmp = sock;
	sock = NULL;

	while (socktmp.lock() != NULL)
	{
		Thread::sleep(10);
	}
}

bool RedisSyncClientImpl::isConnected() const
{
	shared_ptr<Socket> tmp = sock;
	if (tmp == NULL) return false;

	return tmp->getStatus() == NetStatus_connected;
}

bool RedisSyncClientImpl::connect(const NetAddr& addr, uint32_t connecttime, uint32_t cmdtimeout)
{
	sock = TCPClient::create(worker);

	return sock->connect(addr);
}

bool RedisSyncClientImpl::command(const std::string& cmdstr, RedisValue& retvalue)
{
	if (sock->send(cmdstr.c_str(), cmdstr.length()) != cmdstr.length())
	{
		return false;
	}

#define MAXBUFFERLEN	1024*10
	char buffer[MAXBUFFERLEN];

	shared_ptr<RedisValue> val;

	while (val == NULL)
	{
		int recvlen = sock->recv(buffer, MAXBUFFERLEN);

		if (recvlen <= 0) break;

		std::string cmdstrtmp = std::string(buffer, recvlen);

		if (!input(buffer, recvlen))
		{
			break;
		}

		val = result();
	}

	if (val == NULL) return false;

	retvalue = *val.get();

	return true;
}


}
