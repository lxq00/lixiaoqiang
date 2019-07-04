#ifndef __ONVIFPROTOCOL_PROFILES_H__GetScopes
#define __ONVIFPROTOCOL_PROFILES_H__GetScopes
#include "CmdObject.h"


class CMDGetScopes :public CmdObject
{
public:
	CMDGetScopes()
	{
		action = "http://www.onvif.org/ver10/device/wsdl/GetScopes";
	}
	virtual ~CMDGetScopes() {}

	virtual std::string build(const URL& URL)
	{
		stringstream stream;

		stream << "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
			<< "<s:Envelope "<< onvif_xml_ns <<">"
			<< "<s:Body xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\">"
			<< "<GetScopes xmlns=\"http://www.onvif.org/ver10/device/wsdl\"/>"
			<< "</s:Body></s:Envelope>";


		return stream.str();
	}
	shared_ptr<OnvifClientDefs::Scopes> scopes;
	virtual bool parse(const XMLObject::Child& body)
	{
		

		return true;
	}
};



#endif //__ONVIFPROTOCOL_H__
