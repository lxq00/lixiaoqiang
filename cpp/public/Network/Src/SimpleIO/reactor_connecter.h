#ifndef __SIMPILE_REACTOR_CONNECTER_H__
#define __SIMPILE_REACTOR_CONNECTER_H__
#include "reactor_object.h"
class Reactor_Connecter:public Reactor
{
	struct ConnectInfo
	{
		int									fd;
		Public::Base::shared_ptr<Reactor_Object>			obj;
	};
public:
	Reactor_Connecter()
	{
		poolTimer = new Timer("Reactor_Connecter");
		poolTimer->start(Timer::Proc(&Reactor_Connecter::poolTimerProc, this), 0, 10);
	}
	virtual ~Reactor_Connecter()
	{
		poolTimer = NULL;
	}
	bool addConnect(const Public::Base::shared_ptr<Reactor_Object>& obj)
	{
		Public::Base::shared_ptr<Socket> sock = obj->sock();
		if (sock == NULL) return false;
		int fd = sock->getHandle();
		if (fd == -1) return false;

		int fdssize = connectList.size();
		if (fdssize < fd)
		{
			connectList.resize(fd + 1);
			while (fdssize != fd + 1)
			{
				connectList[fdssize].fd = -1;
				fdssize++;
			}
		}
		connectList[fd].obj = obj;
		connectList[fd].fd = fd;

		return true;
	}
	void delConnect(Reactor_Object* obj)
	{
		Public::Base::shared_ptr<Socket> sock = obj->sock();
		if (sock == NULL) return;
		int fd = sock->getHandle();
		if (fd == -1) return;

		connectList[fd].fd = -1;
		connectList[fd].obj = NULL;
	}
private:
	void poolTimerProc(unsigned long)
	{
		for (uint32_t i = 0; i < connectList.size(); i++)
		{
			if (connectList[i].fd == -1) continue;

			connectList[i].obj->connectEvent();
		}
	}
private:
	std::vector<ConnectInfo>		connectList;
	Public::Base::shared_ptr<Timer>				poolTimer;
};

#endif //__SIMPILE_REACTOR_EPOLL_H__
