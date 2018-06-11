#ifndef __HOSTIPC_CONNECTER_H__
#define __HOSTIPC_CONNECTER_H__

#include "HostIPC_Object.h"

namespace Public {
namespace HostIPC {

class HostIPCConnecter:public HostIPCObject
{
protected:
	shared_ptr<Network::IOWorker> m_worker;
	uint16_t					  m_svrPort;
	uint64_t					  m_prevtime;
	std::string					  m_svrtype;
	std::string					  m_svrname;

	void socketDisconnectCallback(Socket* s, const char* str)
	{
		HostIPCObject::socketDisconnectCallback(s, str);

		Guard locker(m_mutex);
		m_sock = make_shared<TCPClient>(*m_worker.get());
		m_status = false;

		m_sock->async_connect(NetAddr("127.0.0.1", m_svrPort), Socket::ConnectedCallback(&HostIPCConnecter::socketConnectCallback, this));
	}

	void socketConnectCallback(Socket* s)
	{
		m_prevtime = 0;

		setSocketConnected();

		m_sock->setSocketBuffer(MAXBUFFERSIZE, MAXBUFFERSIZE);
		m_sock->setSocketTimeout(500, 500);
		m_sock->setDisconnectCallback(Socket::DisconnectedCallback(&HostIPCConnecter::socketDisconnectCallback, this));
	}
public:
	HostIPCConnecter(uint32_t threadNum)
	{
		m_prevtime = 0;
		m_worker = make_shared<Network::IOWorker>(threadNum);
	}
	~HostIPCConnecter()
	{
	}
	bool startConnect(uint16_t falg,const std::string& svrtype, const std::string& svrname,const RecvPackageCallback& recvcallback)
	{
		m_svrPort = falg;
		m_svrtype = svrtype;
		m_svrname = svrname;
		m_callback = recvcallback;
		socketDisconnectCallback(NULL, "");

		return true;
	}
	bool isConnected()
	{
		return m_status;
	}
	void onPoolTimerProc(unsigned long)
	{
		uint64_t nowtime = Time::getCurrentMilliSecond();

		if (m_status && (nowtime >= m_prevtime + HEARTBEATTIME || nowtime < m_prevtime))
		{
			Json::Value val = HostIPC::buildRequestCommand();
			val[HOSTIPC_MESSAGE_HEADER_CMD] = HOSTIPC_DEFAULTCMD_HEARTBEAT;
			val[HOSTIPC_MESSAGE_HEADER_FROM] = m_svrtype;

			Json::Value content;
			content[HOSTIPC_MESSAGE_HEADER_NAME] = m_svrname;
			content[HOSTIPC_MESSAGE_HEADER_TYPE] = m_svrtype;
			content[HOSTIPC_MESSAGE_HEADER_PID] = Process::getProcessId();
			val[HOSTIPC_MESSAGE_HEADER_CONTENT] = content;

			sendPackage(&val);
			m_prevtime = nowtime;
		}
	}
};

}
}

#endif //__HOSTIPC_CONNECTER_H__