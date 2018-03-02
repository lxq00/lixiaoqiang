#include "SimpleIO/socket_object.h"
#include "Network/Network.h"
#include "Base/Base.h"
#include "../version.inl"
using namespace Public::Base;

namespace Public{
namespace Network{

void  NetworkSystem::printVersion()
{
	AppVersion appVersion("Network Lib", r_major, r_minor, r_build, versionalias, __DATE__);
	appVersion.print();
}

void NetworkSystem::init()
{
	initNetwork();
}
void NetworkSystem::uninit()
{
}

}
}
