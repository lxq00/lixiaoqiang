#ifndef __ASYNCCANPOLL_H__
#define __ASYNCCANPOLL_H__
#include "AsyncObjectCan.h"
//#ifndef WIN32
#ifndef WIN32
#include <poll.h>
#endif
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
			doThreadConnectProc();
			
			int timeout = 10;
			struct pollfd fdset[MAXPOLLSIZE];
			memset(fdset,0,sizeof(fdset));
			int fdnum = 0;

			buildDoingEvent();
			std::map<int,Public::Base::shared_ptr<DoingAsyncInfo> > doingsocklist;
			{
				std::map<Socket*, Public::Base::shared_ptr<DoingAsyncInfo> > doingtmp;
				{
					Guard locker(mutex);
					doingtmp = doingList;
				}
				for (std::map<Socket*, Public::Base::shared_ptr<DoingAsyncInfo> >::iterator iter = doingtmp.begin(); iter != doingtmp.end() && fdnum < MAXPOLLSIZE; iter++)
				{
					if (iter->second == NULL) continue;;

					Public::Base::shared_ptr<AsyncInfo> asyncinfo = iter->second->asyncInfo;
					if (asyncinfo == NULL) continue;

					Public::Base::shared_ptr<Socket> sock = asyncinfo->sock.lock();
					if (sock == NULL) continue;

					int sockfd = sock->getHandle();

					doingsocklist[sockfd] = iter->second;

					bool haveEvent = false;

					if (iter->second->acceptEvent != NULL || iter->second->recvEvent != NULL 
						|| (iter->second->disconnedEvent != NULL && iter->second->disconnedEvent->runTimes++ % EVENTRUNTIMES == 0))
					{
						fdset[fdnum].fd = sockfd;
						fdset[fdnum].events |= POLLIN | POLLPRI | POLLERR | POLLHUP | POLLNVAL;

						haveEvent = true;
					}

				if (/*(iter->second->connectEvent != NULL && iter->second->connectEvent->runTimes++ % EVENTRUNTIMES == 0) ||*/
					iter->second->sendEvent != NULL)
				{
					fdset[fdnum].fd = sockfd;
					fdset[fdnum].events |= POLLOUT | POLLERR | POLLHUP | POLLNVAL;

						haveEvent = true;
					}
					if (haveEvent) fdnum++;
				}
			}
#ifdef WIN32
			int ret = WSAPoll(fdset, fdnum, timeout);
#else
			int ret = poll(fdset, fdnum, timeout);
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
				for (int i = 0; i < fdnum; i++)
				{
					std::map<int, Public::Base::shared_ptr<DoingAsyncInfo> >::iterator iter = doingsocklist.find(fdset[i].fd);
					if (iter == doingsocklist.end()) continue;

					Public::Base::shared_ptr<DoingAsyncInfo> info = iter->second;
					if (info == NULL) continue;

					Public::Base::shared_ptr<AsyncInfo> asyncinfo = info->asyncInfo;
					if (asyncinfo == NULL) continue;

					Public::Base::shared_ptr<Socket> sock = asyncinfo->sock.lock();
					if (sock == NULL) continue;
					
					if (fdset[i].revents & POLLIN || fdset[i].revents & POLLPRI)
					{
						if (info->acceptEvent != NULL)
						{
							info->acceptEvent->doCanEvent(sock);
						}
						if (info->recvEvent != NULL)
						{
							if (!info->recvEvent->doCanEvent(sock))
							{
								if (info->disconnedEvent != NULL)
								{
									info->disconnedEvent->doCanEvent(sock);
								}
							}
						}
					}
					if (fdset[i].revents & POLLOUT)
					{
						/*if (info->connectEvent != NULL)
						{
							info->connectEvent->doCanEvent(sock);
						}*/
						if (info->sendEvent != NULL)
						{
							info->sendEvent->doCanEvent(sock);
						}
					}
					if (fdset[i].revents & POLLERR || fdset[i].revents & POLLHUP || fdset[i].revents & POLLNVAL)
					{
						if (info->disconnedEvent != NULL && !info->disconnedEvent->doSuccess)
						{
							info->disconnedEvent->doCanEvent(sock);
						}
					}
				}
			}
		}
	}
};
//#endif
#endif //__ASYNCCANSELECT_H__