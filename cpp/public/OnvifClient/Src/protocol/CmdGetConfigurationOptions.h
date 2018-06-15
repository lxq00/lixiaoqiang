#ifndef __ONVIFPROTOCOL_H__GetConfigurationOptions
#define __ONVIFPROTOCOL_H__GetConfigurationOptions
#include "CmdObject.h"


class CmdGetConfigurationOptions :public CmdObject
{
public:
	CmdGetConfigurationOptions(const std::string& _token):token(_token) {}
	virtual ~CmdGetConfigurationOptions() {}

	virtual std::string build(const URI& uri)
	{
		stringstream stream;

		stream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
			<< "<s:Envelope " << onvif_xml_ns << ">"
			<< buildHeader(uri)
			<< "<s:Body>"
			<< "<GetConfigurationOptions xmlns=\"http://www.onvif.org/ver20/ptz/wsdl\">"
			<< "<ConfigurationToken>"<< token <<"</ConfigurationToken>"
			<< "</GetConfigurationOptions>"
			<< "</s:Body></s:Envelope>";

		return stream.str();
	}
	shared_ptr<OnvifClientDefs::ConfigurationOptions> options;
	virtual bool parse(XMLN * p_xml)
	{
		options = make_shared<OnvifClientDefs::ConfigurationOptions>();

		XMLN * p_res = xml_node_soap_get(p_xml, "tptz:GetConfigurationOptionsResponse");
		if (NULL == p_res)
		{
			return false;
		}

		XMLN * p_opt = xml_node_soap_get(p_res, "tptz:PTZConfigurationOptions");
		if (NULL == p_opt)
		{
			return false;
		}

		XMLN * p_spaces = xml_node_soap_get(p_opt, "tt:Spaces");
		if (NULL == p_spaces)
		{
			return false;
		}

		XMLN * p_abs_pan_space = xml_node_soap_get(p_spaces, "tt:AbsolutePanTiltPositionSpace");
		if (p_abs_pan_space)
		{
			XMLN * p_xrange = xml_node_soap_get(p_abs_pan_space, "tt:XRange");
			if (p_xrange)
			{
				XMLN * p_min = xml_node_soap_get(p_xrange, "tt:Min");
				if (p_min && p_min->data)
				{
					options->absolute_pantilt_x.min = (float)atoi(p_min->data);
				}

				XMLN * p_max = xml_node_soap_get(p_xrange, "tt:Max");
				if (p_max && p_max->data)
				{
					options->absolute_pantilt_x.max = (float)atoi(p_max->data);
				}
			}

			XMLN * p_yrange = xml_node_soap_get(p_abs_pan_space, "tt:YRange");
			if (p_yrange)
			{
				XMLN * p_min = xml_node_soap_get(p_yrange, "tt:Min");
				if (p_min && p_min->data)
				{
					options->absolute_pantilt_y.min = (float)atoi(p_min->data);
				}

				XMLN * p_max = xml_node_soap_get(p_yrange, "tt:Max");
				if (p_max && p_max->data)
				{
					options->absolute_pantilt_y.max = (float)atoi(p_max->data);
				}
			}
		}

		XMLN * p_abs_zoom_space = xml_node_soap_get(p_spaces, "tt:AbsoluteZoomPositionSpace");
		if (p_abs_zoom_space)
		{
			XMLN * p_xrange = xml_node_soap_get(p_abs_zoom_space, "tt:XRange");
			if (p_xrange)
			{
				XMLN * p_min = xml_node_soap_get(p_xrange, "tt:Min");
				if (p_min && p_min->data)
				{
					options->absolute_zoom.min = (float)atoi(p_min->data);
				}

				XMLN * p_max = xml_node_soap_get(p_xrange, "tt:Max");
				if (p_max && p_max->data)
				{
					options->absolute_zoom.max = (float)atoi(p_max->data);
				}
			}
		}

		XMLN * p_rel_pan_space = xml_node_soap_get(p_spaces, "tt:RelativePanTiltTranslationSpace");
		if (p_rel_pan_space)
		{
			XMLN * p_xrange = xml_node_soap_get(p_rel_pan_space, "tt:XRange");
			if (p_xrange)
			{
				XMLN * p_min = xml_node_soap_get(p_xrange, "tt:Min");
				if (p_min && p_min->data)
				{
					options->relative_pantilt_x.min = (float)atoi(p_min->data);
				}

				XMLN * p_max = xml_node_soap_get(p_xrange, "tt:Max");
				if (p_max && p_max->data)
				{
					options->relative_pantilt_x.max = (float)atoi(p_max->data);
				}
			}

			XMLN * p_yrange = xml_node_soap_get(p_rel_pan_space, "tt:YRange");
			if (p_yrange)
			{
				XMLN * p_min = xml_node_soap_get(p_yrange, "tt:Min");
				if (p_min && p_min->data)
				{
					options->relative_pantilt_y.min = (float)atoi(p_min->data);
				}

				XMLN * p_max = xml_node_soap_get(p_yrange, "tt:Max");
				if (p_max && p_max->data)
				{
					options->relative_pantilt_y.max = (float)atoi(p_max->data);
				}
			}
		}

		XMLN * p_rel_zoom_space = xml_node_soap_get(p_spaces, "tt:RelativeZoomTranslationSpace");
		if (p_rel_zoom_space)
		{
			XMLN * p_xrange = xml_node_soap_get(p_rel_zoom_space, "tt:XRange");
			if (p_xrange)
			{
				XMLN * p_min = xml_node_soap_get(p_xrange, "tt:Min");
				if (p_min && p_min->data)
				{
					options->relative_zoom.min = (float)atoi(p_min->data);
				}

				XMLN * p_max = xml_node_soap_get(p_xrange, "tt:Max");
				if (p_max && p_max->data)
				{
					options->relative_zoom.max = (float)atoi(p_max->data);
				}
			}
		}

		XMLN * p_con_pan_space = xml_node_soap_get(p_spaces, "tt:ContinuousPanTiltVelocitySpace");
		if (p_con_pan_space)
		{
			XMLN * p_xrange = xml_node_soap_get(p_con_pan_space, "tt:XRange");
			if (p_xrange)
			{
				XMLN * p_min = xml_node_soap_get(p_xrange, "tt:Min");
				if (p_min && p_min->data)
				{
					options->continuous_pantilt_x.min = (float)atoi(p_min->data);
				}

				XMLN * p_max = xml_node_soap_get(p_xrange, "tt:Max");
				if (p_max && p_max->data)
				{
					options->continuous_pantilt_x.max = (float)atoi(p_max->data);
				}
			}

			XMLN * p_yrange = xml_node_soap_get(p_con_pan_space, "tt:YRange");
			if (p_yrange)
			{
				XMLN * p_min = xml_node_soap_get(p_yrange, "tt:Min");
				if (p_min && p_min->data)
				{
					options->continuous_pantilt_y.min = (float)atoi(p_min->data);
				}

				XMLN * p_max = xml_node_soap_get(p_yrange, "tt:Max");
				if (p_max && p_max->data)
				{
					options->continuous_pantilt_y.max = (float)atoi(p_max->data);
				}
			}
		}

		XMLN * p_con_zoom_space = xml_node_soap_get(p_spaces, "tt:ContinuousZoomVelocitySpace");
		if (p_con_zoom_space)
		{
			XMLN * p_xrange = xml_node_soap_get(p_rel_zoom_space, "tt:XRange");
			if (p_xrange)
			{
				XMLN * p_min = xml_node_soap_get(p_xrange, "tt:Min");
				if (p_min && p_min->data)
				{
					options->continuous_zoom.min = (float)atoi(p_min->data);
				}

				XMLN * p_max = xml_node_soap_get(p_xrange, "tt:Max");
				if (p_max && p_max->data)
				{
					options->continuous_zoom.max = (float)atoi(p_max->data);
				}
			}
		}

		XMLN * p_pan_speed_space = xml_node_soap_get(p_spaces, "tt:PanTiltSpeedSpace");
		if (p_pan_speed_space)
		{
			XMLN * p_xrange = xml_node_soap_get(p_pan_speed_space, "tt:XRange");
			if (p_xrange)
			{
				XMLN * p_min = xml_node_soap_get(p_xrange, "tt:Min");
				if (p_min && p_min->data)
				{
					options->pantilt_speed.min = (float)atoi(p_min->data);
				}

				XMLN * p_max = xml_node_soap_get(p_xrange, "tt:Max");
				if (p_max && p_max->data)
				{
					options->pantilt_speed.max = (float)atoi(p_max->data);
				}
			}
		}

		XMLN * p_zoom_speed_space = xml_node_soap_get(p_spaces, "tt:ZoomSpeedSpace");
		if (p_zoom_speed_space)
		{
			XMLN * p_xrange = xml_node_soap_get(p_zoom_speed_space, "tt:XRange");
			if (p_xrange)
			{
				XMLN * p_min = xml_node_soap_get(p_xrange, "tt:Min");
				if (p_min && p_min->data)
				{
					options->zoom_speed.min = (float)atoi(p_min->data);
				}

				XMLN * p_max = xml_node_soap_get(p_xrange, "tt:Max");
				if (p_max && p_max->data)
				{
					options->zoom_speed.max = (float)atoi(p_max->data);
				}
			}
		}

		XMLN * p_timeout = xml_node_soap_get(p_opt, "tt:PTZTimeout");
		if (p_timeout)
		{
			XMLN * p_min = xml_node_soap_get(p_timeout, "tt:Min");
			if (p_min && p_min->data)
			{
				options->timeout.min = (float)onvif_parse_time(p_min->data);
			}

			XMLN * p_max = xml_node_soap_get(p_timeout, "tt:Max");
			if (p_max && p_max->data)
			{
				options->timeout.max = (float)onvif_parse_time(p_max->data);
			}
		}

		options->used = 1;

		return true;
	}

private:
	std::string token;
};



#endif //__ONVIFPROTOCOL_H__
