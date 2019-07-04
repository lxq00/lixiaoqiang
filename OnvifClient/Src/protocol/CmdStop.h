#ifndef __ONVIFPROTOCOL_CmdGetStreamURL_H__Stop
#define __ONVIFPROTOCOL_CmdGetStreamURL_H__Stop
#include "CmdObject.h"


class CmdStop :public CmdObject
{
public:
	CmdStop(const OnvifClientDefs::PTZCtrl& _ptz, const std::string& _token) :ptzctrl(_ptz), token(_token)
	{
		action = "http://www.onvif.org/ver20/ptz/wsdl/Stop";
	}
	virtual ~CmdStop() {}

	virtual std::string build(const URL& URL)
	{
		stringstream stream;

		stream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
			<< "<s:Envelope "<< onvif_xml_ns<<">"
			<< buildHeader(URL)
			<< "<s:Body>"
			<< "<Stop xmlns=\"http://www.onvif.org/ver20/ptz/wsdl\">"
			<< "<ProfileToken>"<<token<<"</ProfileToken>"
			<< "<PanTilt>"<< ((ptzctrl.ctrlType == OnvifClientDefs::PTZCtrl::PTZ_CTRL_PAN) ? "true" : "false") << "</PanTilt>"
			<< "<Zoom>"<<((ptzctrl.ctrlType == OnvifClientDefs::PTZCtrl::PTZ_CTRL_ZOOM) ? "true" : "false") << "</Zoom>"
			<< "</Stop>"
			<< "</s:Body></s:Envelope>";

		return stream.str();
	}
private:
	OnvifClientDefs::PTZCtrl		ptzctrl;
	std::string token;
};



#endif //__ONVIFPROTOCOL_H__
