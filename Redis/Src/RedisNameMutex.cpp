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
	return internal->key->setnx(internal->index,internal->keyflag, "1");
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