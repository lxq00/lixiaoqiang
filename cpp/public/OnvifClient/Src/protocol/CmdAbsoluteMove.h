#ifndef __ONVIFPROTOCOL_CmdGetStreamUri_H__AbsoluteMove
#define __ONVIFPROTOCOL_CmdGetStreamUri_H__AbsoluteMove
#include "CmdObject.h"


class CmdAbsoluteMove :public CmdObject
{
public:
	CmdAbsoluteMove(const OnvifClientDefs::PTZCtrl& _ptz,const std::string& _token):ptzctrl(_ptz),token(_token) {}
	virtual ~CmdAbsoluteMove() {}

	virtual std::string build(const URI& uri)
	{
		stringstream stream;

		stream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
			<< "<s:Envelope "<< onvif_xml_ns  << ">"
			<< buildHeader(uri)
			<< "<s:Body xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\">"
			<< "<AbsoluteMove xmlns=\"http://www.onvif.org/ver20/ptz/wsdl\">"
			<< "<ProfileToken>"<< token <<"</ProfileToken>"
			<< "<Position>"
			<< "<PanTilt x=\""<< ptzctrl.panTiltX <<"\" y=\""<<ptzctrl.panTiltY<<"\" xmlns=\"http://www.onvif.org/ver10/schema\" />"
			<< "<Zoom x=\""<<ptzctrl.zoom<<"\" xmlns=\"http://www.onvif.org/ver10/schema\" />"
			<< "</Position>"
			<< "</AbsoluteMove>"
			<< "</s:Body></s:Envelope>";

		return stream.str();
	}
	virtual bool parse(const std::string& data) = 0;

private:
	OnvifClientDefs::PTZCtrl		ptzctrl;
	std::string token;
};



#endif //__ONVIFPROTOCOL_H__
