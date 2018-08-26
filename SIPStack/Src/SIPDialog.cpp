#include "eXosip2/eXosip.h"
#include "SIPStack/SIPDialog.h"
#include "SIPStack/SIPSession.h"

namespace Public{
namespace SIPStack {

SipDialog::SipDialog():callId("")
{}
SipDialog::SipDialog(const SIPSession* _session):callId("")
{
	if(_session == NULL)
	{
		return;
	}
	session = make_shared<SIPSession>(*_session);

	osip_message_t* msg = (osip_message_t*)session->getSipMessage();
	if(NULL != msg && msg->message != NULL)
	{
		::free(msg->message);
		msg->message = NULL;
		msg->message_length = 0;
	}
	if (NULL != msg)
	{
		osip_list_special_free (&msg->bodies, (void (*)(void *)) &osip_body_free);
		if(msg->call_id != NULL && msg->call_id->number != NULL)
		{
			callId = msg->call_id->number;
		}
	}
}
SipDialog::SipDialog(const SipDialog& dialog):callId("")
{
	session = dialog.session;
	callId = dialog.callId;
}
SipDialog::~SipDialog()
{
}
void SipDialog::_clone(const SipDialog& dialog)
{
	callId = "";
	session = dialog.session;
	callId = dialog.callId;
}

};
};

