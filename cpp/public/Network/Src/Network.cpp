#include "ASIOServerPool.h"
#include "Network/Network.h"
#include "Base/Base.h"
#include "../version.inl"
//#include "boost/Include/boost/locale.hpp"
using namespace Xunmei::Base;

namespace Xunmei{
namespace Network{
//
//std::string baseCharsetEncodeEx(const std::string& inputstr,const std::string& fromcharset,const std::string& tocharset)
//{
//	if(strcasecmp(fromcharset.c_str(),"utf8") == 0 && strcasecmp(tocharset.c_str(),"gb2312") == 0)
//	{
//		return boost::locale::conv::from_utf(inputstr,"gb2312");
//	}
//	if(strcasecmp(fromcharset.c_str(),"gb2312") == 0 && strcasecmp(tocharset.c_str(),"utf8") == 0)
//	{
//		return boost::locale::conv::to_utf<char>(inputstr,"gb2312");
//	}
//
//	return "";
//}

void  NetworkSystem::printLibVersion()
{
	AppVersion appVersion("Network Lib", r_major, r_minor, r_build, versionalias, __DATE__);
	appVersion.print();
}

void NetworkSystem::init()
{
	//registeCharsetEncode(baseCharsetEncodeEx);
}
void NetworkSystem::uninit()
{
	//registeCharsetEncode(NULL);
}

}
}
