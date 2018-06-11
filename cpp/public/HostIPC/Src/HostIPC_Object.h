#ifndef __HOSTIPC_OBJECT_H__
#define __HOSTIPC_OBJECT_H__

#include "HostIPC/HostIPC.h"
#include "Network/Network.h"

using namespace Public::Network;

namespace Public {
namespace HostIPC {

#define MAXPACKAGELEN 		5*1024*1024
#define MAXBUFFERSIZE		MAXPACKAGELEN + 100

#define HEARTBEATTIME		10000
#define THREADNUM			8

struct IPCProtocol
{
	char flag[4];
	uint32_t len;
};

#define IPCPROTOCOLFLAG	"~!!~"


class HostIPCObject;
typedef Function2<void, HostIPCObject*, const Json::Value& > RecvPackageCallback;
typedef Function1<void, HostIPCObject*> DisconnectCallback;

static uint32_t g_messageSN = 0;
static Mutex g_messageSNMutex;

static uint32_t allocMessageSn()
{
	Guard locker(g_messageSNMutex);
	while (1)
	{
		g_messageSN++;
		if (g_messageSN != 0) break;
	}

	return g_messageSN;
}

class HostIPCObject
{
protected:	
	shared_ptr<Socket>  m_sock;
	bool				m_status;
	Mutex				m_mutex;	
	DisconnectCallback	m_disconnectedcallback;	
	RecvPackageCallback	m_callback;
private:	
	char*				m_recvBuffer;
	uint32_t			m_bufferLen;	
	std::list<shared_ptr<Json::Value> >m_messageList;
	Semaphore			m_sem;

	std::list<shared_ptr<Thread> > m_thread;
	bool				m_quit;
public:
	HostIPCObject():m_quit(false)
	{
		m_recvBuffer = new char[MAXBUFFERSIZE + 10];
		m_bufferLen = 0;
		m_status = false;

		for (int i = 0; i < THREADNUM; i++)
		{
			shared_ptr<Thread> thread = ThreadEx::creatThreadEx("HostIPCObject", ThreadEx::Proc(&HostIPCObject::threadDoProc, this), NULL);
			m_thread.push_back(thread);
			thread->createThread();
		}
	}
	~HostIPCObject()
	{
		m_quit = true;
		SAFE_DELETEARRAY(m_recvBuffer);
		m_bufferLen = 0;
		m_sock = NULL;
		m_thread.clear();
	}
	bool isConnected()
	{
		return m_status;
	}

	bool sendPackage(const Json::Value& pkt)
	{
		return sendPackage(&pkt);
	}
	bool sendPackage(const shared_ptr<Json::Value>& pkt)
	{
		return sendPackage(pkt.get());
	}
	bool setDisConnectCallback(const DisconnectCallback& callback)
	{
		m_disconnectedcallback = callback;
		return true;
	}
	bool startRecvPackage(const RecvPackageCallback& callback)
	{
		Guard locker(m_mutex);
		m_callback = callback;

		if (callback && m_status)
		{
			m_sock->async_recv(Socket::ReceivedCallback(&HostIPCObject::socketRecvCallback, this));
		}

		return true;
	}
	void initSocket(const shared_ptr<Socket>& newsock,const RecvPackageCallback& recvcallback)
	{
		Guard locker(m_mutex);
		m_sock = newsock;
		m_status = true;
		if(m_callback == NULL)m_callback = recvcallback;
		m_sock->setSocketBuffer(MAXBUFFERSIZE, MAXBUFFERSIZE);
		m_sock->setSocketTimeout(500, 500);
		m_sock->setDisconnectCallback(Socket::DisconnectedCallback(&HostIPCObject::socketDisconnectCallback, this));
	
		if (m_callback != NULL && m_status && m_sock != NULL)
		{
			m_sock->async_recv(Socket::ReceivedCallback(&HostIPCObject::socketRecvCallback, this));
		}
	}
protected:
	void threadDoProc(Thread*thread,void* param)
	{
		while (!m_quit)
		{
			m_sem.pend(100);
			shared_ptr<Json::Value> val;
			RecvPackageCallback callback;
			{
				Guard locker(m_mutex);
				if (m_messageList.size() <= 0) continue;
				val = m_messageList.front();
				m_messageList.pop_front();
				callback = m_callback;
			}

			callback(this,*val.get());
		}
	}
	void setSocketConnected()
	{
		Guard locker(m_mutex);
		m_status = true;
		if (m_callback != NULL && m_status)
		{
			m_sock->async_recv(Socket::ReceivedCallback(&HostIPCObject::socketRecvCallback, this));
		}
	}
	void socketRecvCallback(Socket* s, const char* buf, int len)
	{
		do
		{
			Guard locker(m_mutex);

			if (buf == NULL || len < 0 || m_bufferLen + len > MAXBUFFERSIZE)
			{
				break;
			}

			memcpy(m_recvBuffer + m_bufferLen, buf, len);
			m_bufferLen += len;
			m_recvBuffer[m_bufferLen] = 0;
			
			const char* buftmp = m_recvBuffer;
			int buftmplen = m_bufferLen;
			while (buftmplen > 0)
			{
				const char* prostart = strstr(buftmp, IPCPROTOCOLFLAG);
				if (prostart == NULL) break;
				

				IPCProtocol* header = (IPCProtocol*)prostart;
				if (buftmplen - (prostart - buftmp) - sizeof(IPCProtocol) < header->len) break;

				std::string data = std::string(prostart + sizeof(IPCProtocol), header->len);

				shared_ptr<Json::Value> val(new Json::Value());
				Json::Reader reader;
				if(reader.parse(data,*val.get()))
				{
					m_messageList.push_back(val);
					m_sem.post();
				}
				buftmplen -= (buftmp - prostart) + sizeof(IPCProtocol) + header->len;
				buftmp = prostart + sizeof(IPCProtocol) + header->len;
			}
			if (buftmplen > 0 && buftmp != m_recvBuffer)
			{
				memmove(m_recvBuffer, buftmp, buftmplen);
			}
			m_bufferLen = buftmplen;
		} while (0);
		if (m_callback != NULL && m_status && m_sock != NULL)
		{
			m_sock->async_recv(Socket::ReceivedCallback(&HostIPCObject::socketRecvCallback, this));
		}
	}
	void socketDisconnectCallback(Socket* s,const char* str)
	{
		m_disconnectedcallback(this);
		Guard locker(m_mutex);
		m_sock = NULL;
		m_status = false;
	}

	bool sendPackage(const Json::Value* pkt)
	{
		if (pkt == NULL) return false;

		std::string pktstr = pkt->toStyledString();

		if (pktstr.length() > MAXPACKAGELEN)
		{
			logwarn("%s %s:%d len > MAXBUFFERSIZE send error!", __FILE__, __FUNCTION__, __LINE__);
			return false;
		}
		int sendbufflen = sizeof(IPCProtocol) + pktstr.length();
		char* sendbuffer = new (std::nothrow)char[sendbufflen + 100];
		if (sendbuffer == NULL)
		{
			logwarn("%s %s:%d alloc send buffer error!", __FILE__, __FUNCTION__, __LINE__);
			return false;
		}

		IPCProtocol* header = (IPCProtocol*)sendbuffer;
		memcpy(header->flag, IPCPROTOCOLFLAG,4);
		header->len = pktstr.length();

		memcpy(sendbuffer + sizeof(IPCProtocol), pktstr.c_str(), pktstr.length());

		Guard locker(m_mutex);
		if (m_sock == NULL || !m_status)
		{
			SAFE_DELETEARRAY(sendbuffer);
			logwarn("%s %s:%d socket not ready!", __FILE__, __FUNCTION__, __LINE__);
			return false;
		}

		int sendlen = m_sock->send(sendbuffer, sendbufflen);
		if(sendlen != sendbufflen)
		{
			SAFE_DELETEARRAY(sendbuffer);
			logwarn("%s %s:%d send header len error!", __FILE__, __FUNCTION__, __LINE__);
			return false;
		}
		SAFE_DELETEARRAY(sendbuffer);

		return true;
	}
};

}
}

#endif //__HOSTIPC_OBJECT_H__
