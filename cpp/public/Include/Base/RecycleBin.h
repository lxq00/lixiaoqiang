#ifndef __RECYCLEBINPUBLICBASE_H__
#define __RECYCLEBINPUBLICBASE_H__
#include "Base/Thread.h"
#include "Base/Mutex.h"
#include "Base/Semaphore.h"
#include "Base/ThreadPool.h"
#include "Base/BaseTemplate.h"
#include <string>
#include <queue>
using namespace std;

///该头文件定义的为一个资源自动释放机制
///如果需要释放资源、为非当前执行线程，可以让资源放入回收器中，资源回收器自动释放
namespace Public{
namespace Base{

class BASE_API IResource
{
public:
	IResource(){}
	virtual ~IResource(){}

	static void realse(void* res)
	{
		IResource* resource = (IResource*)res;
		if(resource != NULL)
		{
			delete(resource);
		}
	}
};

template<class T>
class Resource:public IResource
{
public:
	Resource(T res,std::string file,int line):resource(res),resfile(file),resline(line){}
	~Resource()
	{
		if(resource != NULL)
		{
			delete (resource);
		}
	}

private:
	T			resource;
	std::string resfile;
	int			resline;
};


template<typename T>
class ResourceFifo
{
public:
	bool put(T res)
	{
		resMutex.enter();
		resList.push(res);
		resSem.post();
		resMutex.leave();

		return true;
	}
	bool get(T& res,int timeout = 100)
	{
		if(resSem.pend(timeout) < 0 || resList.size() == 0)
		{
			return false;
		}
		resMutex.enter();
		res = resList.front();
		resList.pop();
		resMutex.leave();

		return true;
	}
private:
	Mutex 			resMutex;
	Semaphore		resSem;
	std::queue<T> 	resList;
};

class BASE_API RecourceProc
{
	struct RecourceProcInternal;
public:
	RecourceProc();
	~RecourceProc();
	void put(IResource* res,uint64_t timeout);
	static RecourceProc* instance();
private:
	RecourceProcInternal * internal;
};

template<class T> void _RESOURCE_DELETE(T &t ,const std::string& file,int line,uint64_t timeout)
{
	if(t == NULL)
	{
		return;
	}
	Resource<T>* xmsres = new Resource<T>(t,file,line);

	RecourceProc::instance()->put(xmsres,timeout);

	t = NULL;
}

///资源回收函数接口
#define RESOURCE_DELETE(t) _RESOURCE_DELETE(t,__FILE__,__LINE__,0)
#define OTHER_DELETE(t) _RESOURCE_DELETE(t,__FILE__,__LINE__,0)

///超时删除，timeout为毫秒
#define OTHER_DELETETIMEOUT(t,timeout) _RESOURCE_DELETE(t,__FILE__,__LINE__,timeout)
};
};

#endif //__RECYCLEBINPUBLICBASE_H__

