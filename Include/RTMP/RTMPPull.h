#pragma once

#include "Defs.h"
#include "Base/Base.h"
#include "Network/Network.h"
using namespace Public::Base;
using namespace Public::Network;

namespace Public {
namespace RTMP {

class RTMP_API RTMPPull
{
public:
	typedef Function1<void, RTMPPull*> DisconnectCallback;
	typedef Function2<void, RTMPPull*,const PacketInfo&> ReadCallback;
	typedef Function3<void, RTMPPull*, bool, const std::string&> ConnectCallback;
public:
	RTMPPull(const std::string& url,const shared_ptr<IOWorker>& worker,const std::string& useragent);
	virtual ~RTMPPull();

	//Á¬½Ó
	bool connect(const DisconnectCallback& discallback, uint32_t timeout_ms);
	bool connect(const ConnectCallback& callback, const DisconnectCallback& discallback, uint32_t timeout_ms);

	bool start(const ReadCallback& callback);

	/*true-pausing, false-resuing*/
	bool pause(bool pasue);

	bool seek(double timestamp_ms);
private:
	struct RTMPPullInternal;
	RTMPPullInternal *internal;
};

}
}
