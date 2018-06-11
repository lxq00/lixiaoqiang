#ifndef __SIPDIALOG_H__
#define __SIPDIALOG_H__
#include "Base/Base.h"
#include "SIPStack/SIPTransport.h"
#include "SIPStack/SIPStackDefs.h"
using namespace Public::Base;

namespace Public{
namespace SIPStack{

class SIPSession;
class SIP_API SipDialog
{
public:
	SipDialog();
	SipDialog(const SIPSession* session);
	SipDialog(const SipDialog& dialog);
	~SipDialog();
	
	shared_ptr<SIPSession> getSession() {return session;}
	
	bool operator< (const SipDialog& dialog) const
	{
		return strcmp(callId.c_str(),dialog.callId.c_str()) < 0;
	}
	bool operator == (const SipDialog& dialog) const
	{
		return strcmp(callId.c_str(),dialog.callId.c_str()) == 0;
	}
	SipDialog& operator = (const SipDialog& dialog)
	{
		_clone(dialog);

		return *this;
	}
private:
	void _clone(const SipDialog& dialog);
private:	
	shared_ptr<SIPSession>	session;
	std::string				callId;
};

};
};

#endif //__SIPDIALOG_H__
