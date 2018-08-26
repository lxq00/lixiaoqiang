#include "RTMP/RTMPPublish.h"
#include "libRTMPClient.h"

namespace Public {
namespace RTMP {

struct RTMPPublish::RTMPPublishInternal :public LibRTMPClient
{
	RTMPPublish*				clientptr;
	RTMPPublish::DisconnectCallback		disconnectcallback;
	RTMPPublish::ConnectCallback			connectCallback;

	RTMPPublishInternal(const std::string& url, const shared_ptr<IOWorker>& worker, const std::string& useragent)
		:LibRTMPClient(url, worker, useragent) {}

	void rtmpDiscallback()
	{
		disconnectcallback(clientptr);
	}
	
	void rtmpConnectCallback(bool ret, const std::string& errinfo)
	{
		connectCallback(clientptr, ret, errinfo);
	}
};

RTMPPublish::RTMPPublish(const std::string& url, const shared_ptr<IOWorker>& worker, const std::string& useragent)
{
	internal = new RTMPPublishInternal(url, worker, useragent);
	internal->clientptr = this;
}
RTMPPublish::~RTMPPublish()
{
	SAFE_DELETE(internal);
}

bool RTMPPublish::connect(const DisconnectCallback& discallback, uint32_t timeout_ms)
{
	internal->disconnectcallback = discallback;

	return internal->connect(LibRTMPClient::DisconnectCallback(&RTMPPublishInternal::rtmpDiscallback, internal), timeout_ms);
}
bool RTMPPublish::connect(const ConnectCallback& callback, const DisconnectCallback& discallback, uint32_t timeout_ms)
{
	internal->connectCallback = callback;
	internal->disconnectcallback = discallback;
	return internal->connect(LibRTMPClient::ConnectCallback(&RTMPPublishInternal::rtmpConnectCallback, internal), LibRTMPClient::DisconnectCallback(&RTMPPublishInternal::rtmpDiscallback, internal), timeout_ms);
}
bool RTMPPublish::start()
{
	return internal->start(false, NULL);
}
bool RTMPPublish::writePacket(RTMPType type, uint32_t timestmap /*ms*/, const char* buffer, int maxlen)
{
	return internal->writePacket(type, timestmap, buffer, maxlen);
}

}
}
