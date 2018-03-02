#ifndef __SIMPILE_REACTOR_EPOLL_H__
#define __SIMPILE_REACTOR_EPOLL_H__
#include "reactor_connecter.h"

#ifdef SIMPLEIO_SUPPORT_EPOLL
#include <sys/epoll.h>

class Reactor_Epoll:public Reactor_Connecter,public Thread
{
	struct EpollEvent
	{
		Public::Base::shared_ptr<Reactor_Object>	obj;
		int							fd;
		epoll_event					ev;
	};
public:
	Reactor_Epoll():Thread("Reactor_Epoll"), epoll_fd(-1)
	{
		epoll_fd = epoll_create(1);
		createThread();
	}
	virtual ~Reactor_Epoll() 
	{
		destroyThread();
		close(epoll_fd);

		{
			Guard lock(mutex);
			for (std::vector<EpollEvent*>::iterator it = retired.begin(); it != retired.end(); ++it)
				delete *it;
			retired.clear();
		}
	}
private:
	virtual Handle addSocket(const Public::Base::shared_ptr<Reactor_Object>& obj)
	{
		Public::Base::shared_ptr<Socket> sock = obj->sock();
		if (sock == NULL) return NULL;
		int fd = sock->getHandle();
		if (fd == -1) return NULL;

		EpollEvent* event = new EpollEvent;
		event->obj = obj;
		event->fd = fd;
		event->ev.data.ptr = event;
		event->ev.events = 0;
		
		epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event->ev);

		usercount++;

		return event;
	}
	virtual void delSocket(Reactor_Object* obj, Handle handle)
	{
		EpollEvent* event = (EpollEvent*)handle;
		
		event->ev.events = 0;

		epoll_ctl(epoll_fd, EPOLL_CTL_DEL, event->fd, &event->ev);

		event->fd = -1;

		{
			Guard lock(mutex);
			retired.push_back(event);
		}		

		usercount--;
	}

	virtual void setInEvent(const Public::Base::shared_ptr<Reactor_Object>& obj, Handle handle)
	{
		EpollEvent* event = (EpollEvent*)handle;

		event->ev.events |= EPOLLIN;
		epoll_ctl(epoll_fd, EPOLL_CTL_MOD, event->fd, &event->ev);
	}
	virtual void clearInEvent(const Public::Base::shared_ptr<Reactor_Object>& obj, Handle handle)
	{
		EpollEvent* event = (EpollEvent*)handle;

		event->ev.events &= ~((short)EPOLLIN);
		epoll_ctl(epoll_fd, EPOLL_CTL_MOD, event->fd, &event->ev);
	}
	virtual void setOutEvent(const Public::Base::shared_ptr<Reactor_Object>& obj, Handle handle)
	{
		EpollEvent* event = (EpollEvent*)handle;

		event->ev.events |= EPOLLOUT;
		epoll_ctl(epoll_fd, EPOLL_CTL_MOD, event->fd, &event->ev);
	}
	virtual void clearOutEvent(const Public::Base::shared_ptr<Reactor_Object>& obj, Handle handle)
	{
		EpollEvent* event = (EpollEvent*)handle;

		event->ev.events &= ~((short)EPOLLOUT);
		epoll_ctl(epoll_fd, EPOLL_CTL_MOD, event->fd, &event->ev);
	}
	virtual void setErrEvent(const Public::Base::shared_ptr<Reactor_Object>& obj, Handle handle)
	{
		EpollEvent* event = (EpollEvent*)handle;

		event->ev.events |= (EPOLLERR | EPOLLHUP);
		epoll_ctl(epoll_fd, EPOLL_CTL_MOD, event->fd, &event->ev);
	}
	virtual void clearErrEvent(const Public::Base::shared_ptr<Reactor_Object>& obj, Handle handle)
	{
		EpollEvent* event = (EpollEvent*)handle;

		event->ev.events &= ~((short)(EPOLLERR | EPOLLHUP));
		epoll_ctl(epoll_fd, EPOLL_CTL_MOD, event->fd, &event->ev);
	}
	void threadProc()
	{
#define MAXEVENTNUM			256	
		epoll_event ev_buf[MAXEVENTNUM];

		while (looping())
		{
			//  Destroy retired event sources.
			{
				Guard lock(mutex);
				for (std::vector<EpollEvent*>::iterator it = retired.begin(); it != retired.end(); ++it)
					delete *it;
				retired.clear();
			}		

			int rc = epoll_wait(epoll_fd, &ev_buf[0], MAXEVENTNUM,500);
			if (rc < 0)
			{
				Thread::sleep(10);
				continue;
			}
			if (rc == 0)
			{
				continue;
			}

			for (int i = 0; i < rc; i++)
			{
				EpollEvent* event = (EpollEvent*)ev_buf[i].data.ptr;
				if(event == NULL) continue;

				if(event->fd == -1) continue;
				if (ev_buf[i].events & (EPOLLERR | EPOLLHUP))
					event->obj->errEvent();

				if (event->fd == -1) continue;
				if (ev_buf[i].events & EPOLLOUT)
					event->obj->outEvent();

				if (event->fd == -1) continue;
				if (ev_buf[i].events & EPOLLIN)
					event->obj->inEvent();
			}
		}
	}
private:
	Mutex						mutex;
	std::vector<EpollEvent*>	retired;
	int							epoll_fd;
};

#endif

#endif //__SIMPILE_REACTOR_EPOLL_H__
