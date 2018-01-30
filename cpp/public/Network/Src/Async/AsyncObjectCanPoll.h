#ifndef __ASYNCCANPOLL_H__
#define __ASYNCCANPOLL_H__
#include "AsyncObjectCan.h"
#ifndef WIN32
#include <poll.h>
#define MAXPOLLSIZE		1024
class AsyncObjectCanPoll :public AsyncObjectCan,public Thread
{
public:
	AsyncObjectCanPoll(const Public::Base::shared_ptr<AsyncManager>& _manager):AsyncObjectCan(_manager, SuportType_POOL), Thread("AsyncObjectCanPoll")
	{
		createThread();
	}
	~AsyncObjectCanPoll()
	{
		destroyThread();
	}
private:
	void threadProc()
	{
		while (looping())
		{
			int timeout = 10;
			struct pollfd fdset[MAXPOLLSIZE];
			memset(fdset,0,sizeof(fdset));
			int fdnum = 0;

			buildDoingEvent();

			std::vector<Public::Base::shared_ptr<DoingAsyncInfo> > doingsocklist;
			{
				std::map<int, Public::Base::shared_ptr<DoingAsyncInfo> > doingtmp;
				{
					Guard locker(mutex);
					doingtmp = doingList;
				}
				for (std::map<int, Public::Base::shared_ptr<DoingAsyncInfo> >::iterator iter = doingtmp.begin(); iter != doingtmp.end() && fdnum < MAXPOLLSIZE; iter++)
				{
					if (iter->second == NULL) continue;;

					Public::Base::shared_ptr<AsyncInfo> asyncinfo = iter->second->asyncInfo.lock();
					if (asyncinfo == NULL) continue;

					Public::Base::shared_ptr<Socket> sock = asyncinfo->sock.lock();
					if (sock == NULL) continue;

					int sockfd = sock->getHandle();

					doingsocklist.push_back(iter->second);

					bool haveEvent = false;

					if (iter->second->acceptEvent != NULL || iter->second->recvEvent != NULL 
						|| (iter->second->disconnedEvent != NULL && iter->second->disconnedEvent->runTimes++ % EVENTRUNTIMES == 0))
					{
						fdset[fdnum].fd = sockfd;
						fdset[fdnum].events |= POLLIN | POLLPRI | POLLERR | POLLHUP | POLLNVAL;

						haveEvent = true;
					}

					if ((iter->second->connectEvent != NULL && iter->second->connectEvent->runTimes ++ % EVENTRUNTIMES == 0) || iter->second->sendEvent != NULL)
					{
						fdset[fdnum].fd = sockfd;
						fdset[fdnum].events |= POLLOUT | POLLERR | POLLHUP | POLLNVAL;

						haveEvent = true;
					}
					if (haveEvent) fdnum++;
				}
			}
			
			int ret = poll(fdset, fdnum, timeout);
			

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
				for (int i = 0; i < fdnum; i++)
				{
					Public::Base::shared_ptr<DoingAsyncInfo> info = doingsocklist[i];
					if (info == NULL) continue;

					Public::Base::shared_ptr<AsyncInfo> asyncinfo = info->asyncInfo.lock();
					if (asyncinfo == NULL) continue;

					Public::Base::shared_ptr<Socket> sock = asyncinfo->sock.lock();
					if (sock == NULL) continue;
					int sockfd = sock->getHandle();

					if (fdset[i].fd != sockfd)
					{
						assert(0);
					}

					if (fdset[i].events & POLLIN || fdset[i].events & POLLPRI)
					{
						if (info->acceptEvent != NULL)
						{
							info->acceptEvent->doCanEvent(sock);
						}
						if (info->recvEvent != NULL)
						{
							info->recvEvent->doCanEvent(sock);
						}
						if (info->disconnedEvent != NULL)
						{
							info->disconnedEvent->doCanEvent(sock);
						}
					}
					if (fdset[i].events & POLLOUT)
					{
						if (info->connectEvent != NULL)
						{
							info->connectEvent->doCanEvent(sock);
						}
						if (info->sendEvent != NULL)
						{
							info->sendEvent->doCanEvent(sock);
						}
					}
					if (fdset[i].events & POLLERR || fdset[i].events & POLLHUP || fdset[i].events & POLLNVAL)
					{
						if (info->disconnedEvent != NULL)
						{
							info->disconnedEvent->doCanEvent(sock);
						}
					}
				}
			}
		}
	}
};

#endif

#endif //__ASYNCCANSELECT_H__