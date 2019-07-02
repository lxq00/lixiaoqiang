#pragma once

#include "Base/Base.h"
#include "Network/Network.h"
#include "db/db.h"
#include "common/redisprotocol.h"
using namespace Public::Base;
using namespace Public::Network;


#define MAXUSERTIMEOUT		5*60*1000
#define MAXBUFFERLEN		10240

typedef Function3<void, void*,uint32_t, const shared_ptr<RedisValue>&> RecvDataCallback;

class Connection
{
public:
	Connection(const shared_ptr<Socket>& _sock,const RecvDataCallback& _callback):
		datacallback(_callback),socket(_sock),dbindex(0),prevusedtime(Time::getCurrentMilliSecond())
	{
		buffer = new char[MAXBUFFERLEN + 100];
		parser = make_shared<RedisParser>();

		socket->async_recv(buffer, MAXBUFFERLEN, Socket::ReceivedCallback(&Connection::recvCallback, this));
	}
	~Connection()
	{
		SAFE_DELETEARRAY(buffer);
		parser = NULL;
	}

	void sendResponse(const RedisValue& val)
	{
		RedisBuilder builder;
		builder.build(val);

		shared_ptr<Socket> tmp = socket;

		Guard locker(mutex);
		sendlist.push_back(builder.buffer);
		if (sendlist.size() == 1 && tmp != NULL)
		{
			std::string& data = sendlist.front();

			tmp->async_send(data.c_str(), data.length(), Socket::SendedCallback(&Connection::socketSendCallback, this));
		}
	}
	bool isTimeoutOrError()
	{
		uint64_t nowusedtime = prevusedtime;
		uint64_t nowtime = Time::getCurrentMilliSecond();
		if (nowtime > nowusedtime && nowtime - nowusedtime >= MAXUSERTIMEOUT)
		{
			return true;
		}
		if (parser == NULL) return true;

		return false;
	}
	void setDBIndex(uint32_t index)
	{
		dbindex = index;
	}
private:
	void recvCallback(const weak_ptr<Socket>& user, const char* buftmp, int len)
	{
		shared_ptr < Socket> tmp = socket;
		if (tmp == NULL || len <= 0 || buftmp == NULL || parser == NULL) return;


		prevusedtime = Time::getCurrentMilliSecond();
		if (!parser->input(buffer, len))
		{
			parser == NULL;
			return;
		}

		shared_ptr<RedisValue> value = parser->result();
		if (value != NULL)
		{
			datacallback(tmp.get(),dbindex,value);
		}

		tmp->async_recv(buffer, MAXBUFFERLEN, Socket::ReceivedCallback(&Connection::recvCallback, this));
	}
	void socketSendCallback(const weak_ptr<Socket>& user, const char* buffer, int len)
	{
		shared_ptr < Socket> tmp = socket;
		if (tmp == NULL) return;


		Guard locker(mutex);
		
		if (sendlist.size() <= 0) return;
		sendlist.pop_front();

		if (sendlist.size() > 0)
		{
			std::string& data = sendlist.front();

			tmp->async_send(data.c_str(), data.length(), Socket::SendedCallback(&Connection::socketSendCallback, this));
		}
	}
private:
	Mutex						mutex;
	RecvDataCallback			datacallback;
	shared_ptr<Socket>			socket;
	uint32_t					dbindex;
	char*						buffer;
	std::list<std::string>		sendlist;

	uint64_t					prevusedtime;
	shared_ptr<RedisParser>		parser;
};