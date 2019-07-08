#ifndef __ONVIFPROTOCOL_CmdGetStreamURL_H__GetConfigurations
#define __ONVIFPROTOCOL_CmdGetStreamURL_H__GetConfigurations
#include "CmdObject.h"


class CmdGetConfigurations :public CmdObject
{
public:
	CmdGetConfigurations()
	{
		action = "http://www.onvif.org/ver20/ptz/wsdl/GetConfigurations";
	}
	virtual ~CmdGetConfigurations() {}

	virtual std::string build(const URL& URL)
	{
		stringstream stream;

		stream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
			<< "<s:Envelope " << onvif_xml_ns << ">"
			<< buildHeader(URL)
			<< "<s:Body>"
			<< "<GetConfigurations xmlns=\"http://www.onvif.org/ver20/ptz/wsdl\" />"
			<< "</s:Body></s:Envelope>";

		return stream.str();
	}

	shared_ptr<OnvifClientDefs::PTZConfig> ptzcfg;
	virtual bool parse(const XMLObject::Child& body)
	{
		ptzcfg = make_shared<OnvifClientDefs::PTZConfig>();

		const XMLObject::Child& resp = body.getChild("tptz:GetConfigurationsResponse");
		if (!resp) return false;


		const XMLObject::Child& cfg = resp.getChild("tptz:PTZConfiguration");
		if (!cfg) return false;

		ptzcfg->token = cfg.attribute("token");

		ptzcfg->name = cfg.getChild("tt:Name").data();
		ptzcfg->use_count = cfg.getChild("tt:UseCount").data().readInt();
		ptzcfg->nodeToken = cfg.getChild("tt:NodeToken").data();

		const XMLObject::Child& limits = cfg.getChild("tt:PanTiltLimits");
		if (limits)
		{
			const XMLObject::Child& range = limits.getChild("tt:Range");
			if (range)
			{
				const XMLObject::Child& xrange = range.getChild("tt:XRange");
				if (xrange)
				{
					ptzcfg->pantilt_x.min = xrange.getChild("tt:Min").data().readFloat();
					ptzcfg->pantilt_x.max = xrange.getChild("tt:Max").data().readFloat();
				}
				const XMLObject::Child& yrange = range.getChild("tt:YRange");
				if (yrange)
				{
					ptzcfg->pantilt_y.min = yrange.getChild("tt:Min").data().readFloat();
					ptzcfg->pantilt_y.max = yrange.getChild("tt:Max").data().readFloat();
				}
			}
		}

		const XMLObject::Child& zommlimits = cfg.getChild("tt:ZoomLimits");
		if (zommlimits)
		{
			const XMLObject::Child& range = zommlimits.getChild("tt:Range");
			if (range)
			{
				const XMLObject::Child& xrange = range.getChild("tt:XRange");
				if (xrange)
				{
					ptzcfg->zoom.min = xrange.getChild("tt:Min").data().readFloat();
					ptzcfg->zoom.max = xrange.getChild("tt:Max").data().readFloat();
				}
			}
		}

		return true;
	}
};



#endif //__ONVIFPROTOCOL_H__
