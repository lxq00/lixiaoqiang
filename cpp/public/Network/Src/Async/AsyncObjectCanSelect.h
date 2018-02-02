#ifndef __ASYNCCANSELECT_H__
#define __ASYNCCANSELECT_H__
#include "AsyncObjectCan.h"
class AsyncObjectCanSelect :public AsyncObjectCan, public Thread
{
public:
	AsyncObjectCanSelect(const Public::Base::shared_ptr<AsyncManager>& _manager):AsyncObjectCan(_manager,SuportType_SELECT), Thread("AsyncObjectSelect")
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
			doThreadConnectProc();
			struct timeval tv = { 0, 10000 };
			fd_set   fdread, fdwrite,fderror;
			FD_ZERO(&fdread);
			FD_ZERO(&fdwrite);
			FD_ZERO(&fderror);
			int maxsock = 0;

			buildDoingEvent();

			std::map<Socket*, Public::Base::shared_ptr<DoingAsyncInfo> > doingtmp;
			{
				Guard locker(mutex);
				doingtmp = doingList;
			}
			for (std::map<Socket*, Public::Base::shared_ptr<DoingAsyncInfo> >::iterator iter = doingtmp.begin(); iter != doingtmp.end(); iter++)
			{
				if (iter->second == NULL) continue;;

				Public::Base::shared_ptr<AsyncInfo> asyncinfo = iter->second->asyncInfo;
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
				if (iter->second->disconnedEvent != NULL)
				{
					FD_SET(sockfd, &fderror);
					maxsock = maxsock > sockfd ? maxsock : sockfd;
				}
			}
			int ret = select(maxsock+ 1, &fdread, &fdwrite, &fderror, &tv);
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
				for (std::map<Socket*, Public::Base::shared_ptr<DoingAsyncInfo> >::iterator iter = doingtmp.begin(); iter != doingtmp.end(); iter++)
				{
					if (iter->second == NULL) continue;

					Public::Base::shared_ptr<DoingAsyncInfo> doasyncinfo = iter->second;
					if (doasyncinfo == NULL) continue;

					Public::Base::shared_ptr<AsyncInfo> asyncinfo = doasyncinfo->asyncInfo;
					if (asyncinfo == NULL) continue;

					Public::Base::shared_ptr<Socket> sock = asyncinfo->sock.lock();
					if (sock == NULL) continue;
				
					int sockfd = sock->getHandle();
					if (FD_ISSET(sockfd, &fdread))
					{
						if (doasyncinfo->recvEvent != NULL)
						{
							if (!doasyncinfo->recvEvent->doCanEvent(sock))
							{
								if (doasyncinfo->disconnedEvent != NULL)
								{
									doasyncinfo->disconnedEvent->doCanEvent(sock);
								}
							}
						}
						if (doasyncinfo->acceptEvent != NULL)
						{
							doasyncinfo->acceptEvent->doCanEvent(sock);
						}
					}
					if (FD_ISSET(sockfd, &fdwrite))
					{
						/*if (doasyncinfo->connectEvent != NULL)
						{
							doasyncinfo->connectEvent->doCanEvent(sock);
						}*/
						if (doasyncinfo->sendEvent != NULL)
						{
							doasyncinfo->sendEvent->doCanEvent(sock);
						}
					}
					if (FD_ISSET(sockfd, &fderror))
					{
						if (doasyncinfo->disconnedEvent != NULL && !doasyncinfo->disconnedEvent->doSuccess)
						{
							doasyncinfo->disconnedEvent->doCanEvent(sock);
						}
					}
				}
			}
		}
	}
};



#endif //__ASYNCCANSELECT_H__