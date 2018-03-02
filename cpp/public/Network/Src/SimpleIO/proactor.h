#ifndef __PROACTOR_H__
#define __PROACTOR_H__

#include "simpleio_def.h"

class UsingLocker
{
public:
	class LockerManager
	{
	public:
		LockerManager() {}
		~LockerManager() {}
		static LockerManager* instance() 
		{
			static LockerManager manager;
			return &manager;
		}
		bool lock(Socket* sockptr)
		{
			Guard locker(mutex);
			std::map<Socket*, std::set<uint32_t> >::iterator iter = usingList.find(sockptr);
			if (iter == usingList.end())
			{
				return false;
			}
			iter->second.insert(Thread::getCurrentThreadID());

			return true;
		}
		bool unlock(Socket* sockptr)
		{
			Guard locker(mutex);
			std::map<Socket*, std::set<uint32_t> >::iterator iter = usingList.find(sockptr);
			if (iter == usingList.end()) return true;
			iter->second.erase(Thread::getCurrentThreadID());

			return true;
		}
		bool addSock(Socket* sockptr)
		{
			Guard locker(mutex);
			usingList[sockptr] = std::set<uint32_t>();

			return true;
		}
		bool delSock(Socket* sockptr)
		{
			while (1)
			{
				{
					Guard locker(mutex);
					std::map<Socket*, std::set<uint32_t> >::iterator iter = usingList.find(sockptr);
					if (iter == usingList.end()) break;
					else if (iter->second.size() == 0)
					{
						usingList.erase(iter);
						break;
					}
					else if (iter->second.size() == (uint32_t)1 && *iter->second.begin() == (uint32_t)Thread::getCurrentThreadID())
					{
						usingList.erase(iter);
						break;
					}
				}
				Thread::sleep(10);
			}

			return true;
		}
	private:
		Mutex								mutex;
		std::map<Socket*, std::set<uint32_t> > usingList;
	};
public:
	UsingLocker(const Public::Base::shared_ptr<Socket>& _sockptr) :sockptr(_sockptr.get()) {}
	UsingLocker(Socket* _sockptr):sockptr(_sockptr) {}
	~UsingLocker()
	{
		LockerManager::instance()->unlock(sockptr);
	}
	bool lock() 
	{
		return LockerManager::instance()->lock(sockptr);
	}
private:
	Socket* sockptr;
};

class Proactor;
class Proactor_Object
{
public:
	Proactor_Object(const Public::Base::shared_ptr<Proactor>& _proactor) :proactorobj(_proactor) {}
	virtual ~Proactor_Object() {}

	Public::Base::shared_ptr<Proactor> proactor() { return proactorobj; }
private:
	Public::Base::shared_ptr<Proactor>	proactorobj;
};

//前摄器，完成事件读写等事件处理
class Proactor:public Public::Base::enable_shared_from_this<Proactor>
{
public:
	typedef Function2<void, const Public::Base::shared_ptr<Socket>& , bool> UpdateSocketStatusCallback;
	typedef Function2<Public::Base::shared_ptr<Socket>, int, const NetAddr&> BuildSocketCallback;
public:
	//threadnum 线程数
	Proactor(const UpdateSocketStatusCallback& _statusCallback,const BuildSocketCallback& buildCallback,uint32_t threadnum)
		:statusCallback(_statusCallback),buildcallback(buildCallback) {}
	virtual ~Proactor() {}

	virtual int createSocket(NetType type) = 0;

	//添加socket
	virtual shared_ptr<Proactor_Object> addSocket(const Public::Base::shared_ptr<Socket>& sock) = 0;
	//删除socket
	virtual bool delSocket(const Public::Base::shared_ptr<Proactor_Object>& obj, Socket* sockptr) = 0;

	virtual bool addConnect(const Public::Base::shared_ptr<Proactor_Object>& obj,const NetAddr& othreaddr, const Socket::ConnectedCallback& callback) = 0;
	virtual bool addAccept(const Public::Base::shared_ptr<Proactor_Object>& obj, const Socket::AcceptedCallback& callback) = 0;
	virtual bool addRecvfrom(const Public::Base::shared_ptr<Proactor_Object>& obj, char* buffer, int bufferlen, const Socket::RecvFromCallback& callback) = 0;
	virtual bool addRecv(const Public::Base::shared_ptr<Proactor_Object>& obj, char* buffer, int bufferlen, const Socket::ReceivedCallback& callback) = 0;
	virtual bool addSendto(const Public::Base::shared_ptr<Proactor_Object>& obj, const char* buffer, int bufferlen, const NetAddr& otheraddr, const Socket::SendedCallback& callback) = 0;
	virtual bool addSend(const Public::Base::shared_ptr<Proactor_Object>& obj, const char* buffer, int bufferlen, const Socket::SendedCallback& callback) = 0;
	virtual bool addDisconnect(const Public::Base::shared_ptr<Proactor_Object>& obj, const Socket::DisconnectedCallback& callback) = 0;

	static uint32_t buildThreadNum(int threadnum)
	{
		if (threadnum <= 0) threadnum = 1;
		if (threadnum > 128) threadnum = 128;

		return threadnum;
	}
protected:
	UpdateSocketStatusCallback statusCallback;
	BuildSocketCallback		buildcallback;
};



#endif //__PROACTOR_H__
