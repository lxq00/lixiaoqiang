#ifndef __ONVIFPROTOCOL_CmdGetStreamURL_H__GetVideoEncoderConfigurations
#define __ONVIFPROTOCOL_CmdGetStreamURL_H__GetVideoEncoderConfigurations
#include "CmdObject.h"


class CmdGetVideoEncoderConfigurations :public CmdObject
{
public:
	CmdGetVideoEncoderConfigurations()
	{
		action = "http://www.onvif.org/ver10/media/wsdl/GetVideoEncoderConfigurations";
	}
	virtual ~CmdGetVideoEncoderConfigurations() {}

	virtual std::string build(const URL& URL)
	{
		stringstream stream;

		stream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
			<< "<s:Envelope " << onvif_xml_ns << ">"
			<< buildHeader(URL)
			<< "<s:Body>"
			<< "<trt:GetVideoEncoderConfigurations></trt:GetVideoEncoderConfigurations>"
			<< "</s:Body></s:Envelope>";

		return stream.str();
	}
	shared_ptr<OnvifClientDefs::VideoEncoderConfigurations> encoder;
	virtual bool parse(const XMLObject::Child& body) { return false; }
};



#endif //__ONVIFPROTOCOL_H__
