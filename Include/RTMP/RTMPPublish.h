#pragma once

#include "Defs.h"
#include "Base/Base.h"
#include "Network/Network.h"
using namespace Public::Base;
using namespace Public::Network;

namespace Public {
namespace RTMP {

class RTMP_API RTMPPublish
{
public:
	typedef Function1<void, RTMPPublish*> DisconnectCallback;
	typedef Function3<void, RTMPPublish*, bool, const std::string&> ConnectCallback;
public:
	RTMPPublish(const std::string& url, const shared_ptr<IOWorker>& worker, const std::string& useragent);
	~RTMPPublish();

	//连接
	bool connect(const DisconnectCallback& discallback, uint32_t timeout_ms);
	bool connect(const ConnectCallback& callback, const DisconnectCallback& discallback, uint32_t timeout_ms);

	bool start();

	//只支持flv数据
	bool writePacket(RTMPType type,uint32_t timestmap /*ms*/,const char* buffer, int maxlen);
private:
	struct RTMPPublishInternal;
	RTMPPublishInternal *internal;
};

}
}
