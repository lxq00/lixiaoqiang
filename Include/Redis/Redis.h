#pragma once
#include "Defs.h"
#include "Base/Base.h"
#include "Network/Network.h"

using namespace Public::Base;
using namespace Public::Network;	
	
namespace Public{
namespace Redis{

class Redis_Client;

class REDIS_API RedisMQ
{
public:
	typedef Function2<void,const std::string&,const std::string&>  MessageHandler;
public:
	RedisMQ(const shared_ptr<IOWorker>& _worker, const NetAddr& addr, const std::string& password, int index);
	~RedisMQ();
		
	void publish(const std::string& channelName,const std::string& value);
	void subscrib(const std::string& channelName,const MessageHandler& handler);
	void unsubscrib(const std::string& channelName);
private:
	struct RedisMQInternal;
	RedisMQInternal* internal;
};

class REDIS_API RedisHash
{
public:
	RedisHash(const shared_ptr<Redis_Client>& client,const std::string& hashname,int index);
	~RedisHash();
	uint32_t size();
	bool exists(const std::string& key);
	bool set(const std::string& key,const Value& val);
	bool set(const std::map<std::string, Value>& vals);
	Value get(const std::string& key);
	std::map<std::string, Value> get(const std::vector<std::string>& keys);
	bool remove(const std::string& key);
	bool remove(const std::vector<std::string>& keys);
	std::set<std::string> keys();
private:
	struct RedisHashInternal;
	RedisHashInternal* internal;
};

class REDIS_API RedisSet
{
public:
	RedisSet(const shared_ptr<Redis_Client>& client, const std::string& setname, int index);
	~RedisSet();
	uint32_t size();
	bool exists(const Value& val);
	bool insert(const Value& val);
	bool insert(const vector<Value>& vals);
	bool remove(const Value& val);
	bool remove(const vector<Value>& vals);
	vector<Value> members();
private:
	struct RedisSetInternal;
	RedisSetInternal* internal;
};

class REDIS_API RedisList
{
public:
	RedisList(const shared_ptr<Redis_Client>& client, const std::string& listname, int index);
	~RedisList();
	uint32_t size();
	bool push_back(const Value& val);
	Value pop();
private:
	struct RedisListInternal;
	RedisListInternal* internal;
};

class REDIS_API RedisSortedSet
{
public:
	RedisSortedSet(const shared_ptr<Redis_Client>& client, const std::string& setname, int index);
	~RedisSortedSet();
	bool push_back(const Value& key, const Value&val);
	std::vector<Value> values(int startpos, int stoppos);
	Value front();
	bool del(const Value& val);
	uint32_t size();
private:
	struct RedisSortedSetInternal;
	RedisSortedSetInternal* internal;
};

class REDIS_API RedisNameMutex
{
public:
	RedisNameMutex(const shared_ptr<Redis_Client>& client, const std::string& mutexname, int index);
	~RedisNameMutex();
	bool trylock();
	bool lock();
	bool unlock();
private:
	struct RedisNameMutexInternal;
	RedisNameMutexInternal* internal;
};

class REDIS_API RedisKey
{
public:
	RedisKey(const shared_ptr<Redis_Client>& client);
	~RedisKey();
	
	bool setnx(int index,const std::string& key, const Value& val);
	bool set(int index, const std::string& key,const Value& val);
	bool expire(int index, const std::string& key, int ttl_ms);
	Value get(int index, const std::string& key);
	bool exists(int index, const std::string& key);
	bool del(int index, const std::string& key);
	bool del(int index, const std::vector<std::string>& keys);
	std::vector<std::string> keys(int index, const std::string& pattern);
	bool rename(int index, const std::string& oldkey,const std::string& newkey);
	int len(int index, const std::string& key);
	
	int incr(int index, const std::string& key,int val = 1);
	int decr(int index, const std::string& key,int val = 1);
private:
	struct 	RedisKeyInternal;
	RedisKeyInternal* internal;
};


class REDIS_API Redis_Client
{
public:
	Redis_Client();
	virtual ~Redis_Client();

	bool init(const shared_ptr<IOWorker>& worker, const NetAddr& addr, const std::string& password);
	bool uninit();
	bool ping();
	bool command(int index, const std::string& cmd, const std::deque<Value>& args, void* retval = NULL);
private:
	struct Redis_ClientInternal;
	Redis_ClientInternal* internal;
};


}	
}
