#ifndef __ASYNCCANSELECT_H__
#define __ASYNCCANSELECT_H__
#include "IAsyncEvent.h"

class AsyncCanSelect :public IAsyncCanObject,public Thread
{
public:
	AsyncCanSelect(IAsyncManager* _asyncmanager,const weak_ptr<IEventManager>& manager) 
		:IAsyncCanObject(manager), Thread("AsyncCanSelect"), asyncmanager(_asyncmanager)
	{
		createThread();
	}
	~AsyncCanSelect()
	{
		destroyThread();
	}
private:
	void threadProc()
	{
		while (looping())
		{
			struct timeval tv = { 0, 10 };
			fd_set   fdread,fdwrite;
			FD_ZERO(&fdread);
			FD_ZERO(&fdwrite);

			std::list<int> readlist, writelist, errlist;
			int maxsock = 0;
			{
				shared_ptr<IEventManager> eventmanager = manager.lock();
				if (eventmanager == NULL || !eventmanager->getAsyncEventList(asyncmanager,readlist,writelist,errlist))
				{
					break;
				}
			}
			{
				for (std::list<int>::iterator iter = readlist.begin(); iter != readlist.end(); iter++)
				{
					FD_SET(*iter, &fdread);
					maxsock = maxsock > *iter ? maxsock : *iter;
				}
				for (std::list<int>::iterator iter = writelist.begin(); iter != writelist.end(); iter++)
				{
					FD_SET(*iter, &fdwrite);
					maxsock = maxsock > *iter ? maxsock : *iter;
				}
			}
			int ret = select(maxsock+ 1, &fdread, &fdwrite, NULL, &tv);
			if (ret < 0)
			{
				Thread::sleep(10);
				continue;
			}
			if (ret == 0)
			{  
				continue;
			}
			{
				shared_ptr<IEventManager> eventmanager = manager.lock();
				if (eventmanager == NULL)
				{
					break;
				}
				for (std::list<int>::iterator iter = readlist.begin(); iter != readlist.end(); iter++)
				{
					if (FD_ISSET(*iter, &fdread))
					{
						shared_ptr<IAsyncCanEvent> event = eventmanager->findCanEvent(*iter, EventType_Read);
						if (event == NULL)
						{
							continue;
						}
						event->doCanEvent();
					}
				}
				for (std::list<int>::iterator iter = writelist.begin(); iter != writelist.end(); iter++)
				{
					if (FD_ISSET(*iter, &fdwrite))
					{
						shared_ptr<IAsyncCanEvent> event = eventmanager->findCanEvent(*iter, EventType_Write);
						if (event == NULL)
						{
							continue;
						}
						event->doCanEvent();
					}
				}
			}
		}
	}
private:
	IAsyncManager * asyncmanager;
};



#endif //__ASYNCCANSELECT_H__