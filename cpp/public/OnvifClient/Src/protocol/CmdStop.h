#ifndef __ONVIFPROTOCOL_CmdGetStreamUri_H__Stop
#define __ONVIFPROTOCOL_CmdGetStreamUri_H__Stop
#include "CmdObject.h"


class CmdStop :public CmdObject
{
public:
	CmdStop(const OnvifClientDefs::PTZCtrl& _ptz,const std::string& _token):ptzctrl(_ptz),token(_token) {}
	virtual ~CmdStop() {}

	virtual std::string build(const URI& uri)
	{
		stringstream stream;

		stream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
			<< "<s:Envelope "<< onvif_xml_ns<<">"
			<< buildHeader(uri)
			<< "<s:Body>"
			<< "<Stop xmlns=\"http://www.onvif.org/ver20/ptz/wsdl\">"
			<< "<ProfileToken>"<<token<<"</ProfileToken>"
			<< "<PanTilt>"<< ((ptzctrl.ctrlType == OnvifClientDefs::PTZCtrl::PTZ_CTRL_PAN) ? "true" : "false") << "</PanTilt>"
			<< "<Zoom>"<<((ptzctrl.ctrlType == OnvifClientDefs::PTZCtrl::PTZ_CTRL_ZOOM) ? "true" : "false") << "</Zoom>"
			<< "</Stop>"
			<< "</s:Body></s:Envelope>";

		return stream.str();
	}
	virtual bool parse(const std::string& data) = 0;

private:
	OnvifClientDefs::PTZCtrl		ptzctrl;
	std::string token;
};



#endif //__ONVIFPROTOCOL_H__
