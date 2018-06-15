#ifndef __ONVIFPROTOCOL_CmdGetStreamUri_H__GetVideoEncoderConfigurations
#define __ONVIFPROTOCOL_CmdGetStreamUri_H__GetVideoEncoderConfigurations
#include "CmdObject.h"


class CmdGetVideoEncoderConfigurations :public CmdObject
{
public:
	CmdGetVideoEncoderConfigurations(){}
	virtual ~CmdGetVideoEncoderConfigurations() {}

	virtual std::string build(const URI& uri)
	{
		stringstream stream;

		stream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
			<< "<s:Envelope " << onvif_xml_ns << ">"
			<< buildHeader(uri)
			<< "<s:Body>"
			<< "<trt:GetVideoEncoderConfigurations></trt:GetVideoEncoderConfigurations>"
			<< "</s:Body></s:Envelope>";

		return stream.str();
	}
	virtual bool parse(const std::string& data) = 0;
};



#endif //__ONVIFPROTOCOL_H__
