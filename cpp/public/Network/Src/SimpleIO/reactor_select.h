#ifndef __SIMPLEIO_REACTOR_SELECT_H__
#define __SIMPLEIO_REACTOR_SELECT_H__
#include "reactor_connecter.h"

#ifdef SIMPLEIO_SUPPORT_SELECT

class Reactor_Select :public Reactor_Connecter, public Thread
{
	struct SelectEvent
	{
		Public::Base::shared_ptr<Reactor_Object>	obj;
		int							fd;
	};
public:
	Reactor_Select() :Thread("Reactor_Select")
	{
		FD_ZERO(&readset);
		FD_ZERO(&writeset);
		FD_ZERO(&errset);

		createThread();
	}
	virtual ~Reactor_Select() 
	{
		destroyThread();

		{
			Guard locker(mutex);
			newlist.clear();
			dellist.clear();
		}
	}
private:
	virtual Handle addSocket(const Public::Base::shared_ptr<Reactor_Object>& obj)
	{
		Public::Base::shared_ptr<Socket> sock = obj->sock();
		if (sock == NULL) return NULL;
		
		SelectEvent event;
		event.fd = sock->getHandle();
		event.obj = obj;

		if (event.fd == -1) return NULL;

		{
			Guard locker(mutex);
			newlist.push_back(event);
		}

		usercount++;

		return obj.get();
	}
	virtual void delSocket(Reactor_Object* obj, Handle handle)
	{
		int sockfd = obj->sock()->getHandle();
	
		{
			Guard locker(mutex);
			dellist.push_back(obj);
		}		

		FD_CLR(sockfd, &readset);
		FD_CLR(sockfd, &writeset);
		FD_CLR(sockfd, &errset);

		usercount--;
	}

	virtual void setInEvent(const Public::Base::shared_ptr<Reactor_Object>& obj, Handle handle)
	{
		Public::Base::shared_ptr<Socket> sock = obj->sock();
		if (sock == NULL) return;

		int fd = sock->getHandle();

		FD_SET(fd, &readset);
	}
	virtual void clearInEvent(const Public::Base::shared_ptr<Reactor_Object>& obj, Handle handle)
	{
		Public::Base::shared_ptr<Socket> sock = obj->sock();
		if (sock == NULL) return;

		int fd = sock->getHandle();

		FD_CLR(fd, &readset);
	}
	virtual void setOutEvent(const Public::Base::shared_ptr<Reactor_Object>& obj, Handle handle)
	{
		Public::Base::shared_ptr<Socket> sock = obj->sock();
		if (sock == NULL) return;

		int fd = sock->getHandle();

		FD_SET(fd, &writeset);
	}
	virtual void clearOutEvent(const Public::Base::shared_ptr<Reactor_Object>& obj, Handle handle)
	{
		Public::Base::shared_ptr<Socket> sock = obj->sock();
		if (sock == NULL) return;

		int fd = sock->getHandle();

		FD_CLR(fd, &writeset);
	}
	virtual void setErrEvent(const Public::Base::shared_ptr<Reactor_Object>& obj, Handle handle)
	{
		Public::Base::shared_ptr<Socket> sock = obj->sock();
		if (sock == NULL) return;

		int fd = sock->getHandle();

		FD_SET(fd, &errset);
	}
	virtual void clearErrEvent(const Public::Base::shared_ptr<Reactor_Object>& obj, Handle handle)
	{
		Public::Base::shared_ptr<Socket> sock = obj->sock();
		if (sock == NULL) return;

		int fd = sock->getHandle();

		FD_CLR(fd, &errset);
	}
	void threadProc()
	{
		struct timeval tv = { 0, 500000 };

		std::map<void*, SelectEvent> selectmap;
		int maxsockfd = 0;

		while (looping())
		{
			bool sockchanged = false;
			{
				Guard locker(mutex);
				while (dellist.size() > 0)
				{
					void* delsock = dellist.front();
					dellist.pop_front();

					selectmap.erase(delsock);

					sockchanged = true;
				}
				while (newlist.size() > 0)
				{
					SelectEvent newsock = newlist.front();
					newlist.pop_front();

					selectmap[newsock.obj.get()] = newsock;
					sockchanged = true;
				}
			}			
			if (sockchanged)
			{
				maxsockfd = 0;
				for (std::map<void*, SelectEvent>::iterator iter = selectmap.begin(); iter != selectmap.end(); iter++)
				{
					if (maxsockfd < iter->second.fd) maxsockfd = iter->second.fd;
				}
			}
			fd_set _read, _write, _err;
			memcpy(&_read, &readset, sizeof(fd_set));
			memcpy(&_write, &writeset, sizeof(fd_set));
			memcpy(&_err, &errset, sizeof(fd_set));
			
			int ret = select(maxsockfd + 1, &_read, &_write, &_err, &tv);
			if (ret < 0)
			{
				Thread::sleep(10);
				continue;
			}
			if (ret == 0)
			{
				continue;
			}

			for (std::map<void*, SelectEvent>::iterator iter = selectmap.begin(); iter != selectmap.end(); iter++)
			{
				if (FD_ISSET(iter->second.fd, &_err))
				{
					iter->second.obj->errEvent();
				}
				if (FD_ISSET(iter->second.fd, &_write))
				{
					iter->second.obj->outEvent();
				}
				if (FD_ISSET(iter->second.fd, &_read))
				{
					iter->second.obj->inEvent();
				}
			}			
		}
	}
private:
	Mutex						mutex;
	std::list<void*>			dellist;
	std::list<SelectEvent>		newlist;	
	fd_set						readset;
	fd_set						writeset;
	fd_set						errset;
};

#endif

#endif //__SIMPLEIO_REACTOR_SELECT_H__
