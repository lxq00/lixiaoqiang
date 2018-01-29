#ifndef __ASYNCMANAGER_H__
#define __ASYNCMANAGER_H__
#include "AsyncObjectIOCP.h"
#include "AsyncObjectCanEPoll.h"
#include "AsyncObjectCanPoll.h"
#include "AsyncObjectCanSelect.h"

class AsyncManager:public Public::Base::enable_shared_from_this<AsyncManager>
{
public:
	AsyncManager() {}
	bool init(int threadnum) 
	{
		Public::Base::shared_ptr<AsyncObject> async;

		int type = SuportType();
#ifdef WIN32
		if (type & SuportType_IOCP)
		{
			async = new AsyncObjectIOCP(shared_from_this(),threadnum);
			asyncList.push_back(async);
		}
#else
		if (type & SuportType_EPOOL)
		{
			async = new AsyncObjectCanEPoll(shared_from_this(), threadnum);
			asyncList.push_back(async);
		}
#endif
		else if (type & SuportType_POOL)
		{
			for (int i = 0; i < threadnum; i++)
			{
				async = new AsyncObjectCanPoll(shared_from_this());
				asyncList.push_back(async);
			}
		}
		else if (type & SuportType_SELECT)
		{
			for (int i = 0; i < threadnum; i++)
			{
				async = new AsyncObjectCanSelect(shared_from_this());
				asyncList.push_back(async);
			}
		}

		return true;
	}
	virtual ~AsyncManager() 
	{
		Guard locker(mutex);
		asyncList.clear();
	}

	virtual bool deletSocket(const Public::Base::shared_ptr<AsyncObject>& asynctmp, int sockfd)
	{
		shared_ptr<AsyncObject> async = asynctmp;
		return async->deleteSocket(sockfd);
	}

	virtual shared_ptr<AsyncObject> addSocket(int sockfd)
	{
		Guard locker(mutex);
		
		uint32_t idel = 0;
		shared_ptr<AsyncObject> async;
		for (std::list<Public::Base::shared_ptr<AsyncObject> >::iterator iter = asyncList.begin(); iter != asyncList.end(); iter++)
		{
			if (async == NULL || (*iter)->getSocketCount() < idel)
			{
				async = *iter;
				idel = (*iter)->getSocketCount();
			}
		}
		async->addSocket(sockfd);

		return async;
	}
private:
	Mutex								mutex;
	std::list<Public::Base::shared_ptr<AsyncObject> > asyncList;
};

#endif //__ASYNCMANAGER_H__
