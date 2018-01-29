#ifndef __ASYNCCANSELECT_H__
#define __ASYNCCANSELECT_H__
#include "AsyncObjectCan.h"
class AsyncObjectCanSelect :public AsyncObjectCan, public Thread
{
public:
	AsyncObjectCanSelect(const Public::Base::shared_ptr<AsyncManager>& _manager):AsyncObjectCan(_manager), Thread("AsyncObjectSelect")
	{
		createThread();
	}
	~AsyncObjectCanSelect()
	{
		destroyThread();
	}
private:
	void threadProc()
	{
		while (looping())
		{
			struct timeval tv = { 0, 10 };
			fd_set   fdread, fdwrite;
			FD_ZERO(&fdread);
			FD_ZERO(&fdwrite);
			int maxsock = 0;

			buildDoingEvent();

			std::map<const Socket*, Public::Base::shared_ptr<DoingAsyncInfo> > doingtmp;
			{
				Guard locker(mutex);
				doingtmp = doingList;
			}
			for (std::map<const Socket*, Public::Base::shared_ptr<DoingAsyncInfo> >::iterator iter = doingtmp.begin(); iter != doingtmp.end(); iter++)
			{
				if (iter->second == NULL) continue;;

				Public::Base::shared_ptr<AsyncInfo> asyncinfo = iter->second->asyncInfo.lock();
				if (asyncinfo == NULL) continue;

				Public::Base::shared_ptr<Socket> sock = asyncinfo->sock.lock();
				if (sock == NULL) continue;

				int sockfd = sock->getHandle();
				if ((iter->second->connectEvent != NULL && iter->second->connectEvent->runTimes++ % EVENTRUNTIMES == 0) 
					|| iter->second->sendEvent != NULL)
				{
					FD_SET(sockfd, &fdwrite);
					maxsock = maxsock > sockfd ? maxsock : sockfd;
				}
				if (iter->second->acceptEvent != NULL || iter->second->recvEvent != NULL 
					|| (iter->second->disconnedEvent != NULL && iter->second->disconnedEvent->runTimes++ % EVENTRUNTIMES == 0))
				{
					FD_SET(sockfd, &fdread);
					maxsock = maxsock > sockfd ? maxsock : sockfd;
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
				for (std::map<const Socket*, Public::Base::shared_ptr<DoingAsyncInfo> >::iterator iter = doingtmp.begin(); iter != doingtmp.end(); iter++)
				{
					if (iter->second == NULL) continue;;

					Public::Base::shared_ptr<AsyncInfo> asyncinfo = iter->second->asyncInfo.lock();
					if (asyncinfo == NULL) continue;

					Public::Base::shared_ptr<Socket> sock = asyncinfo->sock.lock();
					if (sock == NULL) continue;
				
					int sockfd = sock->getHandle();
					if (FD_ISSET(sockfd, &fdread))
					{
						if (iter->second->recvEvent != NULL)
						{
							iter->second->recvEvent->doCanEvent(sock);
						}
						if (iter->second->acceptEvent != NULL)
						{
							iter->second->acceptEvent->doCanEvent(sock);
						}
						if (iter->second->disconnedEvent != NULL)
						{
							iter->second->disconnedEvent->doCanEvent(sock);
						}
					}
					if (FD_ISSET(sockfd, &fdwrite))
					{
						if (iter->second->connectEvent != NULL)
						{
							iter->second->connectEvent->doCanEvent(sock);
						}
						if (iter->second->sendEvent != NULL)
						{
							iter->second->sendEvent->doCanEvent(sock);
						}
					}
				}
			}
		}
	}
};



#endif //__ASYNCCANSELECT_H__