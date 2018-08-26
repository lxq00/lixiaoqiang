#include "RTMP/RTMPPull.h"
#include "LibRTMPClient.h"

namespace Public {
namespace RTMP {

struct RTMPPull::RTMPPullInternal:public LibRTMPClient
{
	RTMPPull*				clientptr;
	RTMPPull::DisconnectCallback		disconnectcallback;
	RTMPPull::ReadCallback			readcallback;
	RTMPPull::ConnectCallback			connectCallback;

	RTMPPullInternal(const std::string& url, const shared_ptr<IOWorker>& worker, const std::string& useragent)
		:LibRTMPClient(url,worker,useragent){}

	~RTMPPullInternal() {}

	void rtmpDiscallback()
	{
		disconnectcallback(clientptr);
	}

	void rtmpReadCallback(RTMPType type, const void* data, size_t bytes, uint32_t timestamp)
	{
		PacketInfo info;
		info.data = (char*)data;
		info.size = bytes;
		info.type = type;
		info.timestamp = timestamp;

		readcallback(clientptr, info);
	}

	void rtmpConnectCallback(bool ret, const std::string& errinfo)
	{
		connectCallback(clientptr, ret, errinfo);
	}
};

RTMPPull::RTMPPull(const std::string& url, const shared_ptr<IOWorker>& worker, const std::string& useragent)
{
	internal = new RTMPPullInternal(url, worker, useragent);
	internal->clientptr = this;
}
RTMPPull::~RTMPPull()
{
	SAFE_DELETE(internal);
}

bool RTMPPull::connect(const DisconnectCallback& discallback, uint32_t timeout_ms)
{
	internal->disconnectcallback = discallback;

	return internal->connect(LibRTMPClient::DisconnectCallback(&RTMPPullInternal::rtmpDiscallback,internal),timeout_ms);
}
bool RTMPPull::connect(const ConnectCallback& callback, const DisconnectCallback& discallback, uint32_t timeout_ms)
{
	internal->connectCallback = callback;
	internal->disconnectcallback = discallback;
	return internal->connect(LibRTMPClient::ConnectCallback(&RTMPPullInternal::rtmpConnectCallback, internal), LibRTMPClient::DisconnectCallback(&RTMPPullInternal::rtmpDiscallback,internal), timeout_ms);
}
bool RTMPPull::start(const ReadCallback& callback)
{
	internal->readcallback = callback;
	return internal->start(true, LibRTMPClient::ReadCallback(&RTMPPullInternal::rtmpReadCallback, internal));
}

bool RTMPPull::pause(bool pasue)
{
	return internal->pause(pasue);
}

bool RTMPPull::seek(double timestamp_ms)
{
	return internal->seek(timestamp_ms);
}

}
}
