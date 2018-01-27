#ifndef __ASYNCCANPOLL_H__
#define __ASYNCCANPOLL_H__
#include "IAsyncEvent.h"
#include <WinSock2.h>

#define MAXPOLLSIZE		1024
class AsyncCanPoll :public IAsyncCanObject,public Thread
{
public:
	AsyncCanPoll(IAsyncManager* _asyncmanager,const weak_ptr<IEventManager>& manager)
		:IAsyncCanObject(manager), Thread("AsyncCanPoll"), asyncmanager(_asyncmanager)
	{
		createThread();
	}
	~AsyncCanPoll()
	{
		destroyThread();
	}
private:
	void threadProc()
	{
		while (looping())
		{
			int timeout = 10;
			pollfd fdset[MAXPOLLSIZE] = {0};
			int fdnum = 0;
			{
				std::list<int> readlist, writelist, errlist;
				shared_ptr<IEventManager> eventmanager = manager.lock();
				if (eventmanager == NULL || !eventmanager->getAsyncEventList(asyncmanager,readlist,writelist,errlist))
				{
					break;
				}
				std::map<int, int> socketPosMap;
				for (std::list<int>::iterator iter = readlist.begin(); iter != readlist.end(); iter++)
				{
					std::map<int, int>::iterator positer = socketPosMap.find(*iter);
					if (positer == socketPosMap.end())
					{
						socketPosMap[*iter] = fdnum;
						fdnum++;
						positer = socketPosMap.find(*iter);
					}
					fdset[positer->second].fd = *iter;
					fdset[positer->second].events |= POLLIN | POLLPRI | POLLERR | POLLHUP | POLLNVAL;
				}
				for (std::list<int>::iterator iter = writelist.begin(); iter != writelist.end(); iter++)
				{
					std::map<int, int>::iterator positer = socketPosMap.find(*iter);
					if (positer == socketPosMap.end())
					{
						socketPosMap[*iter] = fdnum;
						fdnum++;
						positer = socketPosMap.find(*iter);
					}
					fdset[positer->second].fd = *iter;
					fdset[positer->second].events |= POLLOUT | POLLERR | POLLHUP | POLLNVAL;
				}
				for (std::list<int>::iterator iter = errlist.begin(); iter != errlist.end(); iter++)
				{
					std::map<int, int>::iterator positer = socketPosMap.find(*iter);
					if (positer == socketPosMap.end())
					{
						socketPosMap[*iter] = fdnum;
						fdnum++;
						positer = socketPosMap.find(*iter);
					}
					fdset[positer->second].fd = *iter;
					fdset[positer->second].events |= POLLERR | POLLHUP | POLLNVAL;
				}
			}
#ifdef WIN32
			int ret = WSAPoll(fdset, fdnum,timeout);
#else
			int ret = pool(fdset, fdnum, timeout);
#endif
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
				for (int i = 0; i < fdnum; i++)
				{
					if (fdset[i].events & POLLIN || fdset[i].events & POLLPRI)
					{
						shared_ptr<IAsyncCanEvent> event = eventmanager->findCanEvent(fdset[i].fd, EventType_Read);
						if (event == NULL)
						{
							continue;
						}
						event->doCanEvent();
					}
					if (fdset[i].events & POLLOUT)
					{
						shared_ptr<IAsyncCanEvent> event = eventmanager->findCanEvent(fdset[i].fd, EventType_Write);
						if (event == NULL)
						{
							continue;
						}
						event->doCanEvent();
					}
					if (fdset[i].events & POLLERR || fdset[i].events & POLLHUP || fdset[i].events & POLLNVAL)
					{
						shared_ptr<IAsyncCanEvent> event = eventmanager->findCanEvent(fdset[i].fd, EventType_Error);
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