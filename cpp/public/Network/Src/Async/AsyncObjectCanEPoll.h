#ifndef __ASYNCCANEPOLL_H__
#define __ASYNCCANEPOLL_H__
#include "AsyncObjectCan.h"
#ifndef WIN32
#include <sys/epoll.h>

class AsyncObjectCanEPoll :public AsyncObjectCan, public Thread
{
	struct EpoolEvent
	{
		Public::Base::shared_ptr<DoingAsyncInfo>	doAsyncInfo;
		int											events;
	};
public:
	AsyncObjectCanEPoll(const Public::Base::shared_ptr<AsyncManager>& _manager,int threadnum) :AsyncObjectCan(_manager), Thread("AsyncObjectCanEPoll")
	{
		epollHandle = epoll_create(10000);
		if (epollHandle <= 0)
		{
			return;
		}

		for (int i = 0; i < threadnum; i++)
		{
			Public::Base::shared_ptr<Thread> thread = ThreadEx::creatThreadEx("doResultThreadProc", ThreadEx::Proc(&AsyncObjectCanEPoll::doResultThreadProc, this), NULL);
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
	virtual bool addSocket(const Public::Base::shared_ptr<Socket>& sock)
	{	
		{
			int sockfd = sock->getHandle();
			epoll_event pevent;
			memset(&pevent, 0, sizeof(pevent));
			pevent.data.ptr = sock.get();
			pevent.events = EPOLLET;
			epoll_ctl(epollHandle, EPOLL_CTL_ADD, sockfd, &pevent);
		}

		return AsyncObjectCan::addSocket(sock);
	}

	virtual bool deleteSocket(Socket* sockptr, int sockfd)
	{
		{			
			epoll_event pevent;
			memset(&pevent, 0, sizeof(pevent));

			epoll_ctl(epollHandle, EPOLL_CTL_DEL, sockfd, &pevent);
		}

		return AsyncObjectCan::deleteSocket(sockptr, sockfd);
	}

	void _addSocketEvent(const Public::Base::shared_ptr<Socket>& sock, EventType type, const Public::Base::shared_ptr<DoingAsyncInfo>& doasyncinfo, const Public::Base::shared_ptr<AsyncEvent>& event)
	{
		int events = EPOLLET;

		if (doasyncinfo->recvEvent != NULL || doasyncinfo->acceptEvent != NULL)
		{
			events |= EPOLLIN | EPOLLPRI;
		}
		if (doasyncinfo->sendEvent != NULL)
		{
			events |= EPOLLOUT;
		}
		if (doasyncinfo->disconnedEvent != NULL)
		{
			events |= EPOLLHUP | EPOLLERR;
		}

		if (doasyncinfo->events != events)
		{
			epoll_event pevent;
			memset(&pevent, 0, sizeof(pevent));
			pevent.data.ptr = sock.get();
			pevent.events = events;
			epoll_ctl(epollHandle, EPOLL_CTL_MOD, sock->getHandle(), &pevent);

			doasyncinfo->events = events;
		}	
	}
	void _cleanSocketDoingEvent(const Public::Base::shared_ptr<DoingAsyncInfo>& doasyncinfo, int pevents)
	{
		Public::Base::shared_ptr<Socket> sock = doasyncinfo->asyncInfo->sock.lock();
		if (sock == NULL) return;

		int events = doasyncinfo->events;
		if (pevents & EPOLLIN || pevents & EPOLLPRI)
		{
			events &= ~(EPOLLIN | EPOLLPRI);
		}
		if (pevents & EPOLLOUT)
		{
			events &= ~(EPOLLOUT);
		}
		if (pevents & EPOLLHUP || pevents & EPOLLERR)
		{
			events &= ~(EPOLLHUP | EPOLLERR);
		}

		if (doasyncinfo->events != events)
		{
			epoll_event pevent;
			memset(&pevent, 0, sizeof(pevent));
			pevent.data.ptr = sock.get();
			pevent.events = events;
			epoll_ctl(epollHandle, EPOLL_CTL_MOD, sock->getHandle(), &pevent);

			doasyncinfo->events = events;
		}
	}

	void doResultThreadProc(Thread* thread, void*)
	{
		while (thread->looping())
		{
			eventSem.pend(100);

			EpoolEvent event;
			{
				Guard locker(eventMutex);
				if (eventList.size() <= 0)
				{
					continue;
				}
				event = eventList.front();
				eventList.pop_front();
			}

			Public::Base::shared_ptr<AsyncInfo> asyncInfo = event.doAsyncInfo->asyncInfo;
			if (asyncInfo == NULL) continue;
			Public::Base::shared_ptr<Socket> sock = asyncInfo->sock.lock();
			if (sock == NULL) continue;

			Public::Base::shared_ptr<AsyncEvent> asyncevent;

			if (event.events & EPOLLIN || event.events & EPOLLPRI)
			{				
				asyncevent = event.doAsyncInfo->acceptEvent;
				if (asyncevent != NULL)
				{
					asyncevent->doCanEvent(sock);

					//acceptD¨¨¨°a¡Á??¡¥¨¬¨ª?¨®
					asyncevent->doSuccess = true;
					AcceptCanEvent* acceptEvent = (AcceptCanEvent*)asyncevent.get();
					addAccept(sock, acceptEvent->callback);
				}
				asyncevent = event.doAsyncInfo->recvEvent;
				if (asyncevent != NULL)
				{
					if (!asyncevent->doCanEvent(sock))
					{
						Public::Base::shared_ptr<AsyncEvent> eventtmp = event.doAsyncInfo->disconnedEvent;
						if (eventtmp != NULL && eventtmp != NULL)
						{
							eventtmp->doCanEvent(sock);
						}
					}
				}
			}
			if (event.events & EPOLLOUT)
			{
				asyncevent = event.doAsyncInfo->sendEvent;
				if (asyncevent != NULL)
				{
					asyncevent->doCanEvent(sock);
				}
			}
			if (event.events & EPOLLHUP || event.events & EPOLLERR)
			{
				asyncevent = event.doAsyncInfo->disconnedEvent;
				if (asyncevent != NULL && !asyncevent->doSuccess)
				{
					asyncevent->doCanEvent(sock);
				}
			}

			AsyncObject::buildDoingEvent(asyncInfo->sockptr);
		}
	}
	
	void threadProc()
	{
#define MAXEVENTNUM			1024	
		epoll_event workEpoolEvent[MAXEVENTNUM];

		while (looping())
		{
			doThreadConnectProc();
			int pollSize = epoll_wait(epollHandle, workEpoolEvent, MAXEVENTNUM, 10);
			if (pollSize <= 0)
			{
				continue;
			}

			{
				Guard locker(mutex);
				for (int i = 0; i < pollSize; i++)
				{
					Socket* sockptr = (Socket*)workEpoolEvent[i].data.ptr;
					
					std::map<Socket*, Public::Base::shared_ptr<DoingAsyncInfo> >::iterator iter = doingList.find(sockptr);
					if(iter == doingList.end())
					{
						continue;
					}

					_cleanSocketDoingEvent(iter->second ,workEpoolEvent[i].events);

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
	std::list<Public::Base::shared_ptr<Thread> >  threadlist;

	std::list<EpoolEvent>						eventList;
	Mutex										eventMutex;
	Public::Base::Semaphore						eventSem;
	bool										epoolquit;
};

#endif


#endif //__ASYNCCANSELECT_H__