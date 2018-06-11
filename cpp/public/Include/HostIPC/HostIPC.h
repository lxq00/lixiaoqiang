#ifndef __HOSTIPC_DEFAULT_H__
#define __HOSTIPC_DEFAULT_H__
#include "Defs.h"
#include "Base/Base.h"
#include "JSON/json.h"
using namespace Public::Base;

namespace Public {
namespace HostIPC {

#define HOSTIPC_SVRTYPE_HOST		"host"
#define HOSTIPC_SVRTYPE_DEVICE		"device"
#define HOSTIPC_SVRTYPE_MQTT		"mqtt"
#define HOSTIPC_SVRTYPE_API		"api"
#define HOSTIPC_SVRTYPE_WEB		"web"

#define HOSTIPC_DEFAULTCMD_HEARTBEAT		"heartbeat"


#define HOSTIPC_MESSAGE_HEADER_CMD		"cmd"
#define HOSTIPC_MESSAGE_HEADER_CMDACK	"cmdack"
#define HOSTIPC_MESSAGE_HEADER_NAME	"name"
#define HOSTIPC_MESSAGE_HEADER_TYPE		"type"
#define HOSTIPC_MESSAGE_HEADER_PID		"pid"
#define HOSTIPC_MESSAGE_HEADER_SN		"sn"
#define HOSTIPC_MESSAGE_HEADER_CONTENT		"content"
#define HOSTIPC_MESSAGE_HEADER_RESULT		"result"
#define HOSTIPC_MESSAGE_HEADER_ERRORMSG	"errormsg"
#define HOSTIPC_MESSAGE_HEADER_FROM		"from"
#define HOSTIPC_MESSAGE_HEADER_TO			"to"

class HostIPC_API HostIPC
{
public:
	typedef enum {
		MessageType_Request,
		MessageType_Response,
		MessageType_All,
	}MessageType;
	typedef Function3<void, const std::string& /*cmd*/, const std::string& /*srcsvrtype*/, const Json::Value& /*data*/> RegsiteHandler;
public:
	HostIPC();
	~HostIPC();

	bool startConnect(uint32_t port,const std::string& svrtype,const std::string& svrname);
	bool startListen(uint32_t port,const std::string& svrtype, const std::string& svrname);
	bool stop();

	//注册监听命令
	bool registeHandler(void* object, void* userflag,const std::string& cmd, const RegsiteHandler& handler, MessageType type = MessageType_All);
	bool unregistHandler(void* userflag);
	bool unregistObjectHandler(void* object);

	//返回消息的序列号，> 0 成功， == 0 失败，本机发出的消息序列号不会重复
	uint32_t sendCommand(const std::string& cmd, const Json::Value& data, const std::string& tosvrtype, const std::string& cmdack = "");
	//广播订阅消息
	bool broadcastCommand(const std::string& cmd, const Json::Value& data);

	uint32_t getWorkPort()const ;

	//返回消息的序列号，> 0 成功， == 0 失败，本机发出的消息序列号不会重复
	uint32_t sendRequest(const std::string& cmd, const Json::Value& content, const std::string& tosvrtype, const std::string& cmdack = "");
	bool sendSuccessResponse(const Json::Value& req, const Json::Value& ackcontent = Json::Value());
	bool sendErrorResponse(const Json::Value& req, const std::string& messageerr);

	static Json::Value buildRequestCommand();
	static Json::Value buildResponseCommand(const Json::Value& data);
private:
	struct HostIpcInternal;
	HostIpcInternal* m_internal;
};

}
}


#endif //__HOSTIPC_DEFAULT_H__
