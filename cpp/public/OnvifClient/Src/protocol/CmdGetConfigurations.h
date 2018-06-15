#ifndef __ONVIFPROTOCOL_CmdGetStreamUri_H__GetConfigurations
#define __ONVIFPROTOCOL_CmdGetStreamUri_H__GetConfigurations
#include "CmdObject.h"


class CmdGetConfigurations :public CmdObject
{
public:
	CmdGetConfigurations(){}
	virtual ~CmdGetConfigurations() {}

	virtual std::string build(const URI& uri)
	{
		stringstream stream;

		stream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
			<< "<s:Envelope " << onvif_xml_ns << ">"
			<< buildHeader(uri)
			<< "<s:Body>"
			<< "<GetConfigurations xmlns=\"http://www.onvif.org/ver20/ptz/wsdl\" />"
			<< "</s:Body></s:Envelope>";

		return stream.str();
	}

	shared_ptr<OnvifClientDefs::_PTZConfig> ptzcfg;
	virtual bool parse(XMLN * p_xml)
	{
		ptzcfg = make_shared<OnvifClientDefs::_PTZConfig>();

		XMLN * p_res = xml_node_soap_get(p_xml, "tptz:GetConfigurationsResponse");
		if (NULL == p_res)
		{
			return false;
		}

		XMLN * p_cfg = xml_node_soap_get(p_res, "tptz:PTZConfiguration");
		if (p_cfg)
		{
			ptzcfg->token = xml_attr_get(p_cfg, "token");
		}
		else
		{
			return false;
		}

		XMLN * p_name = xml_node_soap_get(p_cfg, "tt:Name");
		if (p_name && p_name->data)
		{
			ptzcfg->name = p_name->data;
		}

		XMLN * p_use_count = xml_node_soap_get(p_cfg, "tt:UseCount");
		if (p_use_count && p_use_count->data)
		{
			ptzcfg->use_count = atoi(p_use_count->data);
		}

		XMLN * p_node_token = xml_node_soap_get(p_cfg, "tt:NodeToken");
		if (p_node_token && p_node_token->data)
		{
			ptzcfg->nodeToken = p_node_token->data;
		}

		XMLN * p_pantilt_limits = xml_node_soap_get(p_cfg, "tt:PanTiltLimits");
		if (p_pantilt_limits)
		{
			XMLN * p_range = xml_node_soap_get(p_pantilt_limits, "tt:Range");
			if (p_range)
			{
				XMLN * p_xrange = xml_node_soap_get(p_range, "tt:XRange");
				if (p_xrange)
				{
					XMLN * p_min = xml_node_soap_get(p_xrange, "tt:Min");
					if (p_min && p_min->data)
					{
						ptzcfg->pantilt_x.min = (float)atoi(p_min->data);
					}

					XMLN * p_max = xml_node_soap_get(p_xrange, "tt:Max");
					if (p_max && p_max->data)
					{
						ptzcfg->pantilt_x.max = (float)atoi(p_max->data);
					}
				}

				XMLN * p_yrange = xml_node_soap_get(p_range, "tt:YRange");
				if (p_yrange)
				{
					XMLN * p_min = xml_node_soap_get(p_yrange, "tt:Min");
					if (p_min && p_min->data)
					{
						ptzcfg->pantilt_y.min = (float)atoi(p_min->data);
					}

					XMLN * p_max = xml_node_soap_get(p_yrange, "tt:Max");
					if (p_max && p_max->data)
					{
						ptzcfg->pantilt_y.min = (float)atoi(p_max->data);
					}
				}
			}
		}

		XMLN * p_zoom_limits = xml_node_soap_get(p_cfg, "tt:ZoomLimits");
		if (p_zoom_limits)
		{
			XMLN * p_range = xml_node_soap_get(p_zoom_limits, "tt:Range");
			if (p_range)
			{
				XMLN * p_xrange = xml_node_soap_get(p_range, "tt:XRange");
				if (p_xrange)
				{
					XMLN * p_min = xml_node_soap_get(p_xrange, "tt:Min");
					if (p_min && p_min->data)
					{
						ptzcfg->zoom.min = (float)atoi(p_min->data);
					}

					XMLN * p_max = xml_node_soap_get(p_xrange, "tt:Max");
					if (p_max && p_max->data)
					{
						ptzcfg->zoom.max = (float)atoi(p_max->data);
					}
				}
			}
		}

		return true;
	}
};



#endif //__ONVIFPROTOCOL_H__
