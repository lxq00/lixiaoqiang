#include "RedisDefine.h"
#include "Redis/Redis.h"


namespace Public {
namespace Redis {

struct RedisNameMutex::RedisNameMutexInternal
{
	shared_ptr<RedisKey> key;
	std::string			 keyflag;
	int					 index;
};

RedisNameMutex::RedisNameMutex(const shared_ptr<Redis_Client>& client, const std::string& mutexname,int index)
{
	internal = new RedisNameMutexInternal();
	internal->key = make_shared<RedisKey>(client);
	internal->keyflag = mutexname;
	internal->index = index;
}
RedisNameMutex::~RedisNameMutex()
{
	SAFE_DELETE(internal);
}
bool RedisNameMutex::trylock()
{
	if (!internal->key->setnx(internal->index, internal->keyflag, "1"))
	{
		return false;
	}
	//设置过期时间为10s
	return internal->key->expire(internal->index, internal->keyflag, 10 * 1000);
}
bool RedisNameMutex::lock()
{
	while (!trylock())
	{
		Thread::sleep(10);
	}

	return true;
}
bool RedisNameMutex::unlock()
{
	internal->key->del(internal->index, internal->keyflag);

	return true;
}

}
}