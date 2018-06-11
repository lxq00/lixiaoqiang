#ifndef __SIPAUTHENTICATE_H__
#define __SIPAUTHENTICATE_H__
#include "eXosip2/eX_setup.h"
#include "Base/Base.h"
using namespace Public::Base;
class SIPAuthenticate
{
public:
	static osip_www_authenticate_t * getWWWAuthenticate(const osip_message_t * msg,const std::string& username);
	static const char* unquote(const char* str);
	static bool Authenticate(const osip_message_t * msg,const std::string& name,const std::string& passwd);
};


#endif //__SIPAUTHENTICATE_H__
