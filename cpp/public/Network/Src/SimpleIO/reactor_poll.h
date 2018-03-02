#ifndef __SIMPLEIO_REACTOR_POLL_H__
#define __SIMPLEIO_REACTOR_POLL_H__
#include "reactor_connecter.h"

#ifdef SIMPLEIO_SUPPORT_POLL

#ifndef WIN32
#include <poll.h>
#define  SOCKET int
#endif

class Reactor_Poll :public Reactor_Connecter,public Thread
{
	struct PoolEvent
	{
		Public::Base::shared_ptr<Reactor_Object>	obj;
		int							fd;
		int							index;
	};
public:
	Reactor_Poll() :Thread("Reactor_Poll"), retired(false)
	{
		createThread();
	}
	virtual ~Reactor_Poll()
	{
		destroyThread();
	}
private:
	virtual Handle addSocket(const Public::Base::shared_ptr<Reactor_Object>& obj)
	{
		Public::Base::shared_ptr<Socket> sock = obj->sock();
		if (sock == NULL) return NULL;
		int fd = sock->getHandle();
		if (fd == -1) return NULL;

		{
			GuardWriteMutex locker(mutex);
			int fdssize = objList.size();
			if (fdssize < fd)
			{
				objList.resize(fd + 1);
				while (fdssize != fd + 1)
				{
					objList[fdssize].fd = -1;
					fdssize++;
				}
			}
			pollfd pfd = { (SOCKET)fd, 0, 0 };
			pollset.push_back(pfd);
			objList[fd].index = pollset.size() - 1;
			objList[fd].obj = obj;
			objList[fd].fd = fd;
		}		
		
		usercount++;

		return obj.get();
	}
	virtual void delSocket(Reactor_Object* obj, Handle handle)
	{
		Public::Base::shared_ptr<Socket> sock = obj->sock();
		if (sock == NULL) return;
		int fd = sock->getHandle();
		if (fd == -1) return;

		{
			GuardWriteMutex locker(mutex);
			objList[fd].fd = -1;
			pollset[objList[fd].index].events = 0;
		}

		retired = true;
		usercount--;
	}

	virtual void setInEvent(const Public::Base::shared_ptr<Reactor_Object>& obj, Handle handle)
	{
		Public::Base::shared_ptr<Socket> sock = obj->sock();
		if (sock == NULL) return;
		int fd = sock->getHandle();
		if (fd == -1) return;

		GuardReadMutex locker(mutex);
		pollset[objList[fd].index].events |= POLLIN;
	}
	virtual void clearInEvent(const Public::Base::shared_ptr<Reactor_Object>& obj, Handle handle)
	{
		Public::Base::shared_ptr<Socket> sock = obj->sock();
		if (sock == NULL) return;
		int fd = sock->getHandle();
		if (fd == -1) return;

		GuardReadMutex locker(mutex);
		pollset[objList[fd].index].events &= ~((short)POLLIN);
	}
	virtual void setOutEvent(const Public::Base::shared_ptr<Reactor_Object>& obj, Handle handle)
	{
		Public::Base::shared_ptr<Socket> sock = obj->sock();
		if (sock == NULL) return;
		int fd = sock->getHandle();
		if (fd == -1) return;

		GuardReadMutex locker(mutex);
		pollset[objList[fd].index].events |= POLLOUT;
	}
	virtual void clearOutEvent(const Public::Base::shared_ptr<Reactor_Object>& obj, Handle handle)
	{
		Public::Base::shared_ptr<Socket> sock = obj->sock();
		if (sock == NULL) return;
		int fd = sock->getHandle();
		if (fd == -1) return;

		GuardReadMutex locker(mutex);
		pollset[objList[fd].index].events &= ~((short)POLLOUT);
	}
	virtual void setErrEvent(const Public::Base::shared_ptr<Reactor_Object>& obj, Handle handle)
	{
		Public::Base::shared_ptr<Socket> sock = obj->sock();
		if (sock == NULL) return;
		int fd = sock->getHandle();
		if (fd == -1) return;

		GuardReadMutex locker(mutex);
		pollset[objList[fd].index].events |= POLLERR | POLLHUP;
	}
	virtual void clearErrEvent(const Public::Base::shared_ptr<Reactor_Object>& obj, Handle handle)
	{
		Public::Base::shared_ptr<Socket> sock = obj->sock();
		if (sock == NULL) return;
		int fd = sock->getHandle();
		if (fd == -1) return;

		GuardReadMutex locker(mutex);
		pollset[objList[fd].index].events &= ~((short)POLLERR | POLLHUP);
	}
	void threadProc()
	{
		int timeout = 500;

		while (looping())
		{
			//  Destroy retired event sources.
			if (retired)
			{
				GuardWriteMutex locker(mutex);
				uint32_t i = 0;
				while (i < pollset.size())
				{
					if (pollset[i].fd == -1)
						pollset.erase(pollset.begin() + i);
					else
					{
						objList[pollset[i].fd].index = i;
						i++;
					}
				}
				retired = false;
			}

			//  Wait for events.
#ifdef WIN32
			int rc = WSAPoll(&pollset[0], pollset.size(), timeout);
#else
			int rc = poll(&pollset[0], pollset.size(), timeout);
#endif
			if (rc < 0)
			{
				Thread::sleep(10);
				continue;
			}
			if (rc == 0)
			{
				continue;
			}

			GuardReadMutex locker(mutex);
			for (int i = 0; i < rc; i++)
			{
				if (pollset[i].fd == -1)
					continue;
				if (pollset[i].revents & (POLLERR | POLLHUP))
					objList[pollset[i].fd].obj->errEvent();

				if (pollset[i].fd == -1)
					continue;
				if (pollset[i].revents & POLLOUT)
					objList[pollset[i].fd].obj->outEvent();

				if (pollset[i].fd == -1)
					continue;
				if (pollset[i].revents & POLLIN)
					objList[pollset[i].fd].obj->inEvent();
			}	
		}
	}
private:
	ReadWriteMutex				mutex;
	std::vector<PoolEvent>		objList;
	std::vector<pollfd>			pollset;
	bool						retired;
};

#endif

#endif //__SIMPLEIO_REACTOR_POLL_H__
