#ifndef __ASYNCCANEPOLL_H__
#define __ASYNCCANEPOLL_H__
#include "AsyncObjectCan.h"
#ifdef WIN32

class AsyncObjectCanEPoll :public AsyncObjectCan, public Thread
{
public:
	AsyncObjectCanEPoll(int threadnum) :AsyncObjectCan(), Thread("AsyncObjectCanEPoll")
	{
		epollHandle = epoll_create(10000);
		if (epollHandle <= 0)
		{
			return;
		}

		for (uint32_t i = 0; i < threadnum; i++)
		{
			shared_ptr<Thread> thread = ThreadEx::creatThreadEx("doResultThreadProc", ThreadEx::Proc(&AsyncObjectCanEPoll::doResultThreadProc, this), NULL);
			thread->createThread();
			threadlist.push_back(thread);
		}

		createThread();
	}
	~AsyncObjectCanEPoll()
	{
		if (epollHandle > 0)
		{
			::close(epollHandle);
			epollHandle = -1;
		}
		threadlist.clear();
		destroyThread();
	}
private:
	virtual void cleanSocketByHandle(int handle)
	{
		epoll_event pevent;
		memset(&pevent, 0, sizeof(pevent));
		pevent.data.ptr = 0;

		epoll_ctl(epollHandle, EPOLL_CTL_DEL, handle, &pevent);
	}
	virtual void addSocketAllEvent(const shared_ptr<Socket>& sock, int event)
	{
		int sockfd = sock->getHandle();

		epoll_event pevent;
		memset(&pevent, 0, sizeof(pevent));
		pevent.data.ptr = sock.get();
		pevent.events = EPOLLET;
		if (event & EventType_Read || event & EventType_Error)
		{
			pevent.events |= EPOLLIN | EPOLLPRI;
		}
		if (event & EventType_Write)
		{
			pevent.events |= EPOLLOUT;
		}
		if (event & event & EventType_Error)
		{
			pevent.events |= EPOLLHUP | EPOLLERR;
		}
		epoll_ctl(epollHandle, EPOLL_CTL_ADD, eventDo->getHandle(), &pevent);
	}
	virtual void cleanSocketAllEvent(const shared_ptr<Socket>& sock)
	{
		deleteSocket(sock.get());
	}
	virtual bool deleteSocket(const Socket* sock)
	{
		AsyncObject::deleteSocket(sock);

		int sockfd = sock->getHandle();

		cleanSocketByHandle(sockfd);
		
		return true;
	}

	void doResultThreadProc(Thread* thread, void*)
	{
		while (thread->looping())
		{
			eventSem.pend(100);

			EpoolEvent event;
			{
				Guard locker(eventMutex);
				if (eventList.size())
				{
					continue;
				}
				event = eventList.front();
				eventList.pop_front();
			}

			shared_ptr<AsyncInfo> asyncInfo = event.doAsyncInfo->asyncInfo.lock();
			if (asyncInfo == NULL) continue;
			shared_ptr<Socket> sock = asyncInfo->sock.lock();
			if (sock == NULL) continue;

			if (event.events & EPOLLIN || event.events & EPOLLPRI)
			{
				if (event.doAsyncInfo->acceptEvent != NULL)
				{
					event.doAsyncInfo->acceptEvent->doCanEvent(sock);
				}
				if (event.doAsyncInfo->recvEvent != NULL)
				{
					event.doAsyncInfo->recvEvent->doCanEvent(sock);
				}
				if (event.doAsyncInfo->disconnedEvent != NULL)
				{
					event.doAsyncInfo->disconnedEvent->doCanEvent(sock);
				}
			}
			if (event.events & EPOLLOUT)
			{
				if (event.doAsyncInfo->sendEvent != NULL)
				{
					event.doAsyncInfo->sendEvent->doCanEvent(sock);
				}
				if (event.doAsyncInfo->connectEvent != NULL)
				{
					event.doAsyncInfo->connectEvent->doCanEvent(sock);
				}
			}
			if (event.events & EPOLLHUP || event.events & EPOLLERR)
			{
				if (event.doAsyncInfo->disconnedEvent != NULL)
				{
					event.doAsyncInfo->disconnedEvent->doCanEvent(sock);
				}
			}

			AsyncObject::buildDoingEvent(sock.get());
		}
	}
	
	void threadProc()
	{
#define MAXEVENTNUM			1024	
		epoll_event workEpoolEvent[MAXEVENTNUM];

		while (looping())
		{
			int pollSize = epoll_wait(epollHandle, workEpoolEvent, MAXEVENTNUM, 100);
			if (pollSize <= 0)
			{
				continue;
			}

			{
				std::map<const Socket*, shared_ptr<DoingAsyncInfo> > doingtmp;
				{
					Guard locker(mutex);
					doingtmp = doingList;
				}
				for (int i = 0; i < pollSize; i++)
				{
					Socket* sockptr = (Socket*)workEpoolEvent[i].data.ptr;

					std::map<const Socket*, shared_ptr<DoingAsyncInfo> >::iterator iter = doingtmp.find(sockptr);
					if(iter == doingtmp.end())
					{
						continue;
					}

					EpoolEvent event;
					event.doAsyncInfo = iter->second;
					event.events = workEpoolEvent[i].events;

					{
						Guard locker(eventMutex);
						eventList.push_back(event);
						eventSem.post();
					}
				}
			}
		}
	}
private:
	int 							epollHandle;
	std::list<shared_ptr<Thread> >  threadlist;


	struct EpoolEvent
	{
		shared_ptr<DoingAsyncInfo>		doAsyncInfo;
		int								events;
	};
	std::list<EpoolEvent>			eventList;
	Mutex							eventMutex;
	Semaphore						eventSem;
};

#endif


#endif //__ASYNCCANSELECT_H__