#ifndef __ONVIFPROTOCOL_H__GetSnapURL
#define __ONVIFPROTOCOL_H__GetSnapURL
#include "CmdObject.h"


class CmdGetSnapURL :public CmdObject
{
public:
	CmdGetSnapURL(const std::string& _token) :token(_token)
	{
		action = "http://www.onvif.org/ver10/media/wsdl/GetSnapshotURL";
	}
	virtual ~CmdGetSnapURL() {}

	virtual std::string build(const URL& URL)
	{
		stringstream stream;

		stream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
			<< "<s:Envelope " << onvif_xml_ns << ">"
			<< buildHeader(URL)
			<< "<s:Body xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\">"
			<< "<GetSnapshotURL xmlns=\"http://www.onvif.org/ver10/media/wsdl\">"
			<< "<trt:ProfileToken>" << token << "</trt:ProfileToken>"
			<< "</GetSnapshotURL>"
			<< "</s:Body></s:Envelope>";

		return stream.str();
	}
	shared_ptr<OnvifClientDefs::SnapUrl> snapurl;
	virtual bool parse(const XMLObject::Child& body)
	{
		snapurl = make_shared<OnvifClientDefs::SnapUrl>();

		const XMLObject::Child& resp = body.getChild("trt:GetSnapshotURLResponse");
		if (!resp) return false;

		snapurl->url = resp.getChild("trt:MediaURL").getChild("trt:URL").data();

		if (snapurl->url == "") return false;

		return true;
	}

private:
	std::string token;
};



#endif //__ONVIFPROTOCOL_H__
