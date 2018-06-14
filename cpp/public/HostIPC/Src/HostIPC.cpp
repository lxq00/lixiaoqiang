#include "HostIPC/HostIPC.h"
#include "HostIPC_Connecter.h"
#include "HostIPC_Listner.h"

namespace Public {
namespace HostIPC {

static weak_ptr<Network::IOWorker>			g_ioworker;
static Mutex								g_ioworkerMutex;

inline shared_ptr<Network::IOWorker> g_allockIOWorker()
{
	Guard locker(g_ioworkerMutex);
	
	shared_ptr<Network::IOWorker> worker = g_ioworker.lock();
	if (worker == NULL)
	{
		worker = make_shared<Network::IOWorker>(THREADNUM);
		g_ioworker = worker;
	}

	return worker;
}

struct HostIPC::HostIpcInternal
{
	struct CallbackInfo
	{
		void*				userflag;
		void*				object;
		RegsiteHandler		handler;
		MessageType			type;
	};
	Mutex												m_mutex;	
	std::map<std::string, std::list<CallbackInfo> >     m_handlerCallbackList;

	shared_ptr<HostIPCListener>							m_listener;
	shared_ptr<HostIPCConnecter>						m_connecter;
	uint32_t											m_port;
	shared_ptr<Timer>									m_timer;

	std::string					  m_svrtype;
	std::string					  m_svrname;

	void onPoolTimerProc(unsigned long param)
	{
		if (m_listener != NULL)
		{
			m_listener->onPoolTimerProc(param);
		}
		else if (m_connecter != NULL)
		{
			m_connecter->onPoolTimerProc(param);
		}
	}
	void recvCallback(HostIPCObject* obj, const Json::Value& datacmd)
	{
		std::string str = datacmd.toStyledString();
		std::string cmdflag = String::tolower(datacmd[HOSTIPC_MESSAGE_HEADER_CMD].asString());
		std::string srcsvrtype = String::tolower(datacmd[HOSTIPC_MESSAGE_HEADER_FROM].asString());
		std::string tosvrtype = String::tolower(datacmd[HOSTIPC_MESSAGE_HEADER_TO].asString());
		MessageType type = (MessageType)datacmd[HOSTIPC_MESSAGE_HEADER_TYPE].asInt();

		if (type == MessageType_Response && !datacmd[HOSTIPC_MESSAGE_HEADER_CMDACK].empty())
		{
			cmdflag = String::tolower(datacmd[HOSTIPC_MESSAGE_HEADER_CMDACK].asString());
		}

		//只有listen的才有中转功能
		if (strcmp(cmdflag.c_str(), String::tolower(HOSTIPC_DEFAULTCMD_HEARTBEAT).c_str()) != 0 && m_listener != NULL && strcmp(tosvrtype.c_str(), String::tolower(m_svrtype).c_str()) != 0)
		{
			shared_ptr<HostIPCObject> toobj = m_listener->getObject(tosvrtype);
			if(toobj != NULL) toobj->sendPackage(datacmd);
		}
		else
		{
			if (m_listener != NULL && strcmp(cmdflag.c_str(), String::tolower(HOSTIPC_DEFAULTCMD_HEARTBEAT).c_str()) == 0)
			{
				m_listener->doHeartBeat(obj, datacmd);
			}

			std::list<CallbackInfo> callbacklist;

			{
				Guard locker(m_mutex);
				std::map < std::string, std::list<CallbackInfo> >::iterator iter = m_handlerCallbackList.find(cmdflag);
				if (iter == m_handlerCallbackList.end())
				{
					return;
				}
				callbacklist = iter->second;
			}
			for (std::list<CallbackInfo>::iterator citer = callbacklist.begin(); citer != callbacklist.end(); citer++)
			{
				if (citer->type == MessageType_All || citer->type == type)
				{
					citer->handler(cmdflag, srcsvrtype, datacmd);
				}
			}
		}
	}
};

HostIPC::HostIPC()
{
	m_internal = new HostIpcInternal();
	m_internal->m_port = 0;
	m_internal->m_timer = make_shared<Timer>("HostIpcInternal");
	m_internal->m_timer->start(Timer::Proc(&HostIpcInternal::onPoolTimerProc, m_internal), 0, 1000);
}
HostIPC::~HostIPC()
{
	stop();

	SAFE_DELETE(m_internal);
}

bool HostIPC::startConnect(uint32_t port, const std::string& svrtype, const std::string& svrname)
{
	Guard locker(m_internal->m_mutex);
	m_internal->m_port = port;
	m_internal->m_svrtype = svrtype;
	m_internal->m_svrname = svrname;
	m_internal->m_connecter = make_shared<HostIPCConnecter>(g_allockIOWorker());
	m_internal->m_connecter->startConnect(port, svrtype,svrname,RecvPackageCallback(&HostIpcInternal::recvCallback,m_internal));

	return true;
}
bool HostIPC::startListen(uint32_t port, const std::string& svrtype, const std::string& svrname)
{
	m_internal->m_port = port;
	m_internal->m_svrtype = svrtype;
	m_internal->m_svrname = svrname;
	m_internal->m_listener = make_shared<HostIPCListener>(g_allockIOWorker());
	m_internal->m_listener->startListen(port, RecvPackageCallback(&HostIpcInternal::recvCallback, m_internal));

	return true;
}
bool HostIPC::stop()
{
	m_internal->m_timer = NULL;
	Guard locker(m_internal->m_mutex);
	m_internal->m_connecter = NULL;
	m_internal->m_listener = NULL;

	return true;
}

//注册监听命令
bool HostIPC::registeHandler(void* object, void* userflag, const std::string& cmd, const RegsiteHandler& handler, MessageType type)
{
	std::string cmdflag = String::tolower(cmd);

	Guard locker(m_internal->m_mutex);
	std::map < std::string, std::list<HostIpcInternal::CallbackInfo> >::iterator iter = m_internal->m_handlerCallbackList.find(cmdflag);
	if (iter == m_internal->m_handlerCallbackList.end())
	{
		m_internal->m_handlerCallbackList[cmdflag] = std::list<HostIpcInternal::CallbackInfo>();
		iter = m_internal->m_handlerCallbackList.find(cmdflag);
	}

	HostIpcInternal::CallbackInfo info;
	info.userflag = userflag;
	info.object = object;
	info.type = type;
	info.handler = handler;
	iter->second.push_back(info);

	return true;
}
bool HostIPC::unregistHandler(void* userflag)
{
	Guard locker(m_internal->m_mutex);
	for (std::map < std::string, std::list<HostIpcInternal::CallbackInfo> >::iterator iter = m_internal->m_handlerCallbackList.begin(); iter != m_internal->m_handlerCallbackList.end(); iter++)
	{
		for (std::list<HostIpcInternal::CallbackInfo>::iterator citer = iter->second.begin(); citer != iter->second.end(); )
		{
			if (citer->userflag == userflag)
			{
				iter->second.erase(citer++);
			}
			else
			{
				citer++;
			}
		}
	}
	

	return true;
}
bool HostIPC::unregistObjectHandler(void* object)
{
	Guard locker(m_internal->m_mutex);
	for (std::map < std::string, std::list<HostIpcInternal::CallbackInfo> >::iterator iter = m_internal->m_handlerCallbackList.begin(); iter != m_internal->m_handlerCallbackList.end(); iter++)
	{
		for (std::list<HostIpcInternal::CallbackInfo>::iterator citer = iter->second.begin(); citer != iter->second.end();)
		{
			if (citer->object == object)
			{
				iter->second.erase(citer++);
			}
			else
			{
				citer++;
			}
		}
	}

	return true;
}

uint32_t HostIPC::sendCommand(const std::string& cmd, const Json::Value& data, const std::string& svrtype, const std::string& cmdack)
{
	Json::Value tmp = data;
	
	tmp[HOSTIPC_MESSAGE_HEADER_CMD] = cmd;
	if (cmdack != "") tmp[HOSTIPC_MESSAGE_HEADER_CMDACK] = cmdack;
	if(tmp[HOSTIPC_MESSAGE_HEADER_SN].empty()) tmp[HOSTIPC_MESSAGE_HEADER_SN] = allocMessageSn();
	tmp[HOSTIPC_MESSAGE_HEADER_TO] = svrtype;
	if(tmp[HOSTIPC_MESSAGE_HEADER_FROM].empty())  tmp[HOSTIPC_MESSAGE_HEADER_FROM] = m_internal->m_svrtype;

	shared_ptr<HostIPCObject> obj;

	{
		Guard locker(m_internal->m_mutex);
		if (m_internal->m_connecter != NULL) obj = m_internal->m_connecter;
		else if(m_internal->m_listener != NULL)
		{
			obj = m_internal->m_listener->getObject(svrtype);
		}
	}

	if (obj == NULL) return 0;
	if (!obj->isConnected()) return 0;

	if (!obj->sendPackage(tmp)) return 0;

	return (uint32_t)tmp[HOSTIPC_MESSAGE_HEADER_SN].asInt();
}
bool HostIPC::broadcastCommand(const std::string& cmd, const Json::Value& data)
{
	Json::Value tmp = data;

	tmp[HOSTIPC_MESSAGE_HEADER_CMD] = cmd;
	tmp[HOSTIPC_MESSAGE_HEADER_TYPE] = (int)MessageType_Request;
	if (tmp[HOSTIPC_MESSAGE_HEADER_SN].empty()) tmp[HOSTIPC_MESSAGE_HEADER_SN] = allocMessageSn();

	m_internal->recvCallback(NULL, tmp);

	return true;
}
uint32_t HostIPC::getWorkPort()const
{
	return m_internal->m_port;
}

Json::Value HostIPC::buildRequestCommand()
{
	Json::Value tmp;

	tmp[HOSTIPC_MESSAGE_HEADER_TYPE] = (int)MessageType_Request;
	tmp[HOSTIPC_MESSAGE_HEADER_SN] = allocMessageSn();

	return tmp;
}
Json::Value HostIPC::buildResponseCommand(const Json::Value& data)
{
	Json::Value tmp = data;
	tmp[HOSTIPC_MESSAGE_HEADER_TYPE] = (int)MessageType_Response;
	tmp[HOSTIPC_MESSAGE_HEADER_CONTENT] = Json::Value();

	return tmp;
}

uint32_t HostIPC::sendRequest(const std::string& cmd, const Json::Value& content, const std::string& svrtype, const std::string& cmdack)
{
	Json::Value req = HostIPC::buildRequestCommand();
	req[HOSTIPC_MESSAGE_HEADER_CONTENT] = content;

	return sendCommand(cmd, req, svrtype,cmdack);
}
bool HostIPC::sendSuccessResponse(const Json::Value& req, const Json::Value& ackcontent)
{
	Json::Value ack = HostIPC::buildResponseCommand(req);
	ack[HOSTIPC_MESSAGE_HEADER_RESULT] = 1;

	if (!ackcontent.empty()) ack[HOSTIPC_MESSAGE_HEADER_CONTENT] = ackcontent;

	return sendCommand(req[HOSTIPC_MESSAGE_HEADER_CMD].asString(), ack, req[HOSTIPC_MESSAGE_HEADER_FROM].asString()) != 0;
}
bool HostIPC::sendErrorResponse(const Json::Value& req, const std::string& messageerr)
{
	Json::Value ack = HostIPC::buildResponseCommand(req);
	ack[HOSTIPC_MESSAGE_HEADER_RESULT] = 0;
	ack[HOSTIPC_MESSAGE_HEADER_ERRORMSG] = messageerr;

	return sendCommand(req[HOSTIPC_MESSAGE_HEADER_CMD].asString(), ack, req[HOSTIPC_MESSAGE_HEADER_FROM].asString()) != 0;
}

}
}

