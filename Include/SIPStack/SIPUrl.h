#ifndef __SIPURL_H__
#define __SIPURL_H__
#include "Network/Network.h"
#include "Base/Base.h"
#include <stdlib.h>
using namespace Public::Base;
using namespace Public::Network;

namespace Public{
namespace SIPStack {

class SIP_API SIPUrl
{
public:
	SIPUrl(const std::string & _username,const std::string & _host= "",const std::string& _port = "5060"):username(_username),host(_host),port(atoi(_port.c_str())){}
	SIPUrl(const std::string & _username,const std::string & _host = "",const int& _port = 5060):username(_username),host(_host),port(_port){}
	SIPUrl(const std::string& _username,const NetAddr& addr):username(_username)
	{
		host = ((NetAddr*)&addr)->getIP();
		port = ((NetAddr*)&addr)->getPort();
	}
	~SIPUrl(){}
	
	std::string getUserName() const {return username;}
	std::string getHost() const {return host;}
	int getPort() const {return port;}
private:
	std::string username;
	std::string host;
	int			port;
};

};
};


#endif //__SIPURL_H__
