#ifndef __SIPTRANSPORT_H__
#define __SIPTRANSPORT_H__
#include "Base/Base.h"
#include "Network/Network.h"
#include "SIPStack/SIPStackDefs.h"
using namespace Public::Base;
using namespace Public::Network;

namespace Public{
namespace SIPStack {
	
class SIPSession;

class SIP_API SIPTransport
{
	friend class SIPSession;
public:
	SIPTransport(const NetAddr& _otheraddr,void* _contex):sock(0)
	{
		otheraddr = _otheraddr;
		contex = _contex;
	}
	SIPTransport():sock(0){}
	~ SIPTransport(){}
	
	NetAddr getOtherAddr() const {return otheraddr;}
	
	bool operator < (const SIPTransport& trans) const
	{
		return _toString() < trans._toString();
	}
	bool operator == (const SIPTransport& trans) const
	{
		return _toString() == trans._toString();
	}
	int getSock() const{return sock;}
	void setSock(int _sock) {sock = _sock;}
private:
	std::string _toString() const
	{
		char sroucetmp[128];
		sprintf(sroucetmp,"ip=%s;port=%d;contex=%llu",((NetAddr*)&otheraddr)->getIP().c_str(),((NetAddr*)&otheraddr)->getPort(),(unsigned long long)contex);

		return sroucetmp;
	}
private:
	int			sock;
	NetAddr		otheraddr;
	void*		contex;
};

};
};


#endif //__SIPTRANSPORT_H__

