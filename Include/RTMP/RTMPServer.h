
#pragma once
#include "Defs.h"
#include "Base/Base.h"
#include "Network/Network.h"
using namespace Public::Base;
using namespace Public::Network;

namespace Public {
namespace RTMP {

class RTMP_API RTMPServer
{
	struct RTMPServerInternal;
public:
	class RTMP_API Publish
	{
		friend struct RTMPServerInternal;
		Publish(void* ptr);
	public:
		typedef Function1<void, Publish*> DisconnectCallback;
		typedef Function2<void, Publish*, const PacketInfo&> ReadCallback;
	public:
		~Publish();
	
		void setCallback(const ReadCallback& readcallback, const DisconnectCallback& discallback);
	private:
		struct PublishInternal;
		PublishInternal * internal;
	};

	class RTMP_API Server
	{
		friend struct RTMPServerInternal;
		Server(void* ptr);
	public:
		typedef Function1<void, Server*> DisconnectCallback;
		/*true-pausing, false-resuing*/
		typedef Function2<void, Server*, bool> PauseCallback;
		typedef Function2<void, Server*, uint32_t> SeekCallback;
	public:
		~Server();

		void setCallback(const DisconnectCallback& discallback,const PauseCallback& pause,const SeekCallback& seek);

		//只支持flv数据
		bool writePacket(RTMPType type, uint32_t timestmap /*ms*/, const char* buffer, int maxlen);
	private:
		struct ServerInternal;
		ServerInternal * internal;
	};
	
	//const char* app, const char* stream, const char* type
	typedef Function4<void, const shared_ptr<Publish>&, const std::string&, const std::string&, const std::string&> PublishCallback;
	//const char* app, const char* stream, double start, double duration, bool reset
	typedef Function6<void, const shared_ptr<Server>&, const std::string&, const std::string&, double,double,bool> ServerCallback;
public:
	RTMPServer(uint32_t port,const shared_ptr<IOWorker>& worker);
	~RTMPServer();

	bool start(const PublishCallback& pcallback, const ServerCallback& scallback);
	bool stop();
private:
	RTMPServerInternal * internal;
};


}
}
