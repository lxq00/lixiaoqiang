#ifndef __ONVIFPROTOCOL_CmdGetStreamUri_H__ContinuousMove
#define __ONVIFPROTOCOL_CmdGetStreamUri_H__ContinuousMove
#include "CmdObject.h"


class CmdContinuousMove :public CmdObject
{
public:
	CmdContinuousMove(const OnvifClientDefs::PTZCtrl& _ptz,const std::string& _token):ptzctrl(_ptz),token(_token) 
	{
		action = "http://www.onvif.org/ver20/ptz/wsdl/ContinuousMove";
	}
	virtual ~CmdContinuousMove() {}

	virtual std::string build(const URI& uri)
	{
		stringstream stream;

		stream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
			<< "<s:Envelope "<< onvif_xml_ns  << ">"
			<< buildHeader(uri)
			<< "<s:Body>"
			<< "<ContinuousMove xmlns=\"http://www.onvif.org/ver20/ptz/wsdl\">"
			<< "<ProfileToken>" << token << "</ProfileToken>"
			<< "<Velocity>";
		if (ptzctrl.ctrlType == OnvifClientDefs::PTZCtrl::PTZ_CTRL_PAN)
		{
			stream << "<PanTilt x=\"%0.1f\" y=\"%0.1f\" " << ptzctrl.panTiltY;
		}
		else if (ptzctrl.ctrlType == OnvifClientDefs::PTZCtrl::PTZ_CTRL_ZOOM)
		{
			stream << "<Zoom x=\"%0.1f\" "<< ptzctrl.zoom;
		}
		stream << "xmlns=\"http://www.onvif.org/ver10/schema\" />"
			<< "</Velocity>"
			<< "<Timeout>PT" << ptzctrl.duration << "S</Timeout>"
			<< "</ContinuousMove>"
			<< "</s:Body></s:Envelope>";

		return stream.str();
	}

	shared_ptr<OnvifClientDefs::ContinuousMove> move;
	virtual bool parse(XMLN * p_xml) { return false; }

private:
	OnvifClientDefs::PTZCtrl		ptzctrl;
	std::string token;
};



#endif //__ONVIFPROTOCOL_H__
