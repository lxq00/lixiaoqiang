#ifndef __HOSTIPC_CONNECTER__LISTERN_H__
#define __HOSTIPC_CONNECTER__LISTERN_H__

#include "HostIPC_Object.h"
namespace Public {
namespace HostIPC {

#define MAXOUTOFFLINETIME		60000

class HostIPCListener
{
	struct ConnectInfo
	{
		uint64_t					startTime;
		shared_ptr<HostIPCObject>	obj;
	};
private:
	Mutex												m_mutex;
	shared_ptr<Socket>									m_sock;
	shared_ptr<Network::IOWorker>						m_worker;
	std::map<std::string, shared_ptr<HostIPCObject> >   m_svrList;
	std::list<shared_ptr<HostIPCObject> >				m_freeList;
	std::list<ConnectInfo>								m_connectList;
	RecvPackageCallback									m_recvcalblack;

	void acceptCallback(Socket* s, Socket* newsock)
	{
		Guard locker(m_mutex);
		ConnectInfo info;

		info.startTime = Time::getCurrentMilliSecond();
		info.obj = make_shared<HostIPCObject>();
		info.obj->initSocket(shared_ptr<Socket>(newsock), m_recvcalblack);

		m_connectList.push_back(info);
	}
	void disconnectCallback(HostIPCObject* obj)
	{
		Guard locker(m_mutex);

		for (std::map<std::string, shared_ptr<HostIPCObject> >::iterator iter = m_svrList.begin(); iter != m_svrList.end(); iter++)
		{
			if (iter->second.get() == obj)
			{
				m_freeList.push_back(iter->second);
				m_svrList.erase(iter);
				break;
			}
		}

		for (std::list<ConnectInfo>::iterator iter = m_connectList.begin(); iter != m_connectList.end(); iter++)
		{
			if (iter->obj.get() == obj)
			{
				m_freeList.push_back(iter->obj);
				m_connectList.erase(iter);
				break;
			}
		}
	}
public:
	HostIPCListener(uint32_t threadNum)
	{
		m_worker = make_shared<Network::IOWorker>(threadNum);
	}
	~HostIPCListener()
	{
	}

	bool startListen(uint16_t falg, const RecvPackageCallback& recvcallback)
	{
		m_recvcalblack = recvcallback;
		m_sock = make_shared<TCPServer>(*m_worker.get());
		m_sock->bind(NetAddr(falg));
		m_sock->async_accept(Socket::AcceptedCallback(&HostIPCListener::acceptCallback, this));

		return true;
	}

	void doHeartBeat(HostIPCObject* obj, const Json::Value& data)
	{
		std::string svrtype = data[HOSTIPC_MESSAGE_HEADER_CONTENT][HOSTIPC_MESSAGE_HEADER_TYPE].asString();

		Guard locker(m_mutex);
		for (std::list<ConnectInfo>::iterator iter = m_connectList.begin(); iter != m_connectList.end(); iter++)
		{
			if (iter->obj.get() == obj)
			{
				std::map<std::string, shared_ptr<HostIPCObject> >::iterator siter = m_svrList.find(String::tolower(svrtype));
				if (siter != m_svrList.end()) m_freeList.push_back(siter->second);

				m_svrList[String::tolower(svrtype)] = iter->obj;
				m_connectList.erase(iter);
				break;
			}
		}
	}
	shared_ptr<HostIPCObject> getObject(const std::string& svrtype)
	{
		Guard locker(m_mutex);
		std::map<std::string, shared_ptr<HostIPCObject> >::iterator iter = m_svrList.find(String::tolower(svrtype));
		if (iter == m_svrList.end())
		{
			return shared_ptr<HostIPCObject>();
		}

		return iter->second;
	}
	void onPoolTimerProc(unsigned long)
	{
		{
			Guard locker(m_mutex);

			uint64_t nowTime = Time::getCurrentMilliSecond();

			for (std::list<ConnectInfo>::iterator iter = m_connectList.begin(); iter != m_connectList.end(); )
			{
				if (nowTime < iter->startTime || nowTime > iter->startTime + MAXOUTOFFLINETIME)
				{
					m_freeList.push_back(iter->obj);
					m_connectList.erase(iter++);
				}
				else
				{
					iter++;
				}
			}
		}
		{
			std::list<shared_ptr<HostIPCObject> > tmplist;
			{
				Guard locker(m_mutex);
				tmplist = m_freeList;
				m_freeList.clear();
			}
		}
	}
};


}
}

#endif //__HOSTIPC_CONNECTER__LISTERN_H__