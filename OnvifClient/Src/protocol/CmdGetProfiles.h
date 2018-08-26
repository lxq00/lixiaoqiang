#ifndef __ONVIFPROTOCOL_PROFILES_H__
#define __ONVIFPROTOCOL_PROFILES_H__
#include "CmdObject.h"


class CMDGetProfiles :public CmdObject
{
public:
	CMDGetProfiles()
	{
		action = "http://www.onvif.org/ver10/media/wsdl/GetProfiles";
	}
	virtual ~CMDGetProfiles() {}

	virtual std::string build(const URL& URL)
	{
		stringstream stream;

		stream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
			<< "<s:Envelope " << onvif_xml_ns << ">"
			<< buildHeader(URL)
			<< "<s:Body>"
			<< "<trt:GetProfiles></trt:GetProfiles>"
			<< "</s:Body></s:Envelope>";

		return stream.str();
	}
	shared_ptr<OnvifClientDefs::Profiles> profileInfo;
	virtual bool parse(XMLN * p_xml)
	{
		profileInfo = make_shared<OnvifClientDefs::Profiles>();


		XMLN * p_res = xml_node_soap_get(p_xml, "trt:GetProfilesResponse");
		if (NULL == p_res)
		{
			return false;
		}

		XMLN * p_profiles = xml_node_soap_get(p_res, "trt:Profiles");
		while (p_profiles)
		{
			OnvifClientDefs::ProfileInfo profile;

			profile.fixed = onvif_parse_bool(xml_attr_get(p_profiles, "fixed")) == TRUE;
			profile.token = xml_attr_get(p_profiles, "token");

			if (parseProfileInfo(p_profiles, profile))
			{
				profileInfo->infos.push_back(profile);
			}

			p_profiles = p_profiles->next;
		}

		return true;
	}
	virtual bool parseProfileInfo(XMLN * p_profiles,OnvifClientDefs::ProfileInfo& info)
	{
		OnvifClientDefs::ProfileInfo* profileInfo = &info;

		XMLN * p_name = xml_node_soap_get(p_profiles, "tt:Name");
		if (p_name && p_name->data)
		{
			profileInfo->name = p_name->data;
		}
		else
		{
			return false;
		}

		XMLN * p_video_src = xml_node_soap_get(p_profiles, "tt:VideoSourceConfiguration");
		if (p_video_src)
		{
			if (!parseVideoSource(p_video_src,info))
			{
				profileInfo->VideoSource = NULL;
			}
		}

		XMLN * p_video_enc = xml_node_soap_get(p_profiles, "tt:VideoEncoderConfiguration");
		if (p_video_enc)
		{
			if (!parseVideoEncoder(p_video_enc, info))
			{
				profileInfo->VideoEncoder = NULL;
			}
		}

		XMLN * p_ptz_cfg = xml_node_soap_get(p_profiles, "tt:PTZConfiguration");
		if (p_ptz_cfg)
		{
			if (!parsePTZCfg(p_ptz_cfg, info))
			{
				profileInfo->PTZConfig = NULL;
			}
		}

		return TRUE;
	}
private:
	bool parseVideoSource(XMLN * p_node, OnvifClientDefs::ProfileInfo& info)
	{
		info.VideoSource = make_shared<OnvifClientDefs::_VideoSource>();

		info.VideoSource->token = xml_attr_get(p_node, "token");
		XMLN * p_name = xml_node_soap_get(p_node, "tt:Name");
		if (p_name && p_name->data)
		{
			info.VideoSource->stream_name = p_name->data;
		}
		else
		{
			return false;
		}

		XMLN * p_usecount = xml_node_soap_get(p_node, "tt:UseCount");
		if (p_usecount && p_usecount->data)
		{
			info.VideoSource->use_count = atoi(p_usecount->data);
		}

		XMLN * p_token = xml_node_soap_get(p_node, "tt:SourceToken");
		if (p_token && p_token->data)
		{
			info.VideoSource->source_token = p_node->data;
		}
		else
		{
			return false;
		}

		XMLN * p_bounds = xml_node_soap_get(p_node, "tt:Bounds");
		if (p_bounds)
		{
			info.VideoSource->height = atoi(xml_attr_get(p_bounds, "height"));
			info.VideoSource->width = atoi(xml_attr_get(p_bounds, "width"));
			info.VideoSource->x = atoi(xml_attr_get(p_bounds, "x"));
			info.VideoSource->y = atoi(xml_attr_get(p_bounds, "y"));
		}
		else
		{
			return false;
		}

		return true;
	}

	bool parseVideoEncoder(XMLN * p_node, OnvifClientDefs::ProfileInfo& info)
	{
		info.VideoEncoder = make_shared<OnvifClientDefs::_VideoEncoder>();

		info.VideoEncoder->token = xml_attr_get(p_node, "token");

		XMLN * p_name = xml_node_soap_get(p_node, "tt:Name");
		if (p_name && p_name->data)
		{
			info.VideoEncoder->name = p_name->data;
		}
		else
		{
			return false;
		}

		XMLN * p_usecount = xml_node_soap_get(p_node, "tt:UseCount");
		if (p_usecount && p_usecount->data)
		{
			info.VideoEncoder->use_count = atoi(p_usecount->data);
		}

		XMLN * p_encoding = xml_node_soap_get(p_node, "tt:Encoding");
		if (p_encoding && p_encoding->data)
		{
			info.VideoEncoder->encoding = onvif_parse_encoding(p_encoding->data);
		}
		else
		{
			return FALSE;
		}

		XMLN * p_resolution = xml_node_soap_get(p_node, "tt:Resolution");
		if (p_resolution)
		{
			XMLN * p_width = xml_node_soap_get(p_resolution, "tt:Width");
			if (p_width && p_width->data)
			{
				info.VideoEncoder->width = atoi(p_width->data);
			}

			XMLN * p_height = xml_node_soap_get(p_resolution, "tt:Height");
			if (p_height && p_height->data)
			{
				info.VideoEncoder->height = atoi(p_height->data);
			}
		}

		XMLN * p_quality = xml_node_soap_get(p_node, "tt:Quality");
		if (p_quality && p_quality->data)
		{
			info.VideoEncoder->quality = (float)atoi(p_quality->data);
		}

		XMLN * p_rate_ctl = xml_node_soap_get(p_node, "tt:RateControl");
		if (p_rate_ctl)
		{
			XMLN * p_fr_limit = xml_node_soap_get(p_rate_ctl, "tt:FrameRateLimit");
			if (p_fr_limit && p_fr_limit->data)
			{
				info.VideoEncoder->framerate_limit = atoi(p_fr_limit->data);
			}

			XMLN * p_en_int = xml_node_soap_get(p_rate_ctl, "tt:EncodingInterval");
			if (p_en_int && p_en_int->data)
			{
				info.VideoEncoder->encoding_interval = atoi(p_en_int->data);
			}

			XMLN * p_bt_limit = xml_node_soap_get(p_rate_ctl, "tt:BitrateLimit");
			if (p_bt_limit && p_bt_limit->data)
			{
				info.VideoEncoder->bitrate_limit = atoi(p_bt_limit->data);
			}
		}

		if (info.VideoEncoder->encoding == OnvifClientDefs::VIDEO_ENCODING_H264)
		{
			XMLN * p_h264 = xml_node_soap_get(p_node, "tt:H264");
			if (p_h264)
			{
				XMLN * p_gov_len = xml_node_soap_get(p_h264, "tt:GovLength");
				if (p_gov_len && p_gov_len->data)
				{
					info.VideoEncoder->gov_len = atoi(p_gov_len->data);
				}

				XMLN * p_h264_profile = xml_node_soap_get(p_h264, "tt:H264Profile");
				if (p_h264_profile && p_h264_profile->data)
				{
					info.VideoEncoder->h264_profile = onvif_parse_h264_profile(p_h264_profile->data);
				}
			}
		}

		XMLN * p_time = xml_node_soap_get(p_node, "tt:SessionTimeout");
		if (p_time && p_time->data)
		{
			info.VideoEncoder->session_timeout = onvif_parse_time(p_time->data);
		}

		return true;
	}

	bool parsePTZCfg(XMLN * p_node, OnvifClientDefs::ProfileInfo& info)
	{
		info.PTZConfig = make_shared<OnvifClientDefs::_PTZConfig>();

		info.PTZConfig->token = xml_attr_get(p_node, "token");

		XMLN * p_name = xml_node_soap_get(p_node, "tt:Name");
		if (p_name && p_name->data)
		{
			info.PTZConfig->name = p_name->data;
		}
		else
		{
			return FALSE;
		}

		XMLN * p_usecount = xml_node_soap_get(p_node, "tt:UseCount");
		if (p_usecount && p_usecount->data)
		{
			info.PTZConfig->use_count = atoi(p_usecount->data);
		}

		XMLN * p_node_token = xml_node_soap_get(p_node, "tt:NodeToken");
		if (p_node_token && p_node_token->data)
		{
			info.PTZConfig->nodeToken = p_node_token->data;
		}

		XMLN * p_def_speed = xml_node_soap_get(p_node, "tt:DefaultPTZSpeed");
		if (p_def_speed)
		{
			XMLN * p_pantilt = xml_node_soap_get(p_def_speed, "tt:PanTilt");
			if (p_pantilt)
			{
				info.PTZConfig->def_speed.pan_tilt_x = atoi(xml_attr_get(p_pantilt, "x"));
				info.PTZConfig->def_speed.pan_tilt_y = atoi(xml_attr_get(p_pantilt, "y"));
			}

			XMLN * p_zoom = xml_node_soap_get(p_def_speed, "tt:Zoom");
			if (p_zoom)
			{
				info.PTZConfig->def_speed.zoom = atoi(xml_attr_get(p_zoom, "x"));
			}
		}

		XMLN * p_time = xml_node_soap_get(p_node, "tt:DefaultPTZTimeout");
		if (p_time && p_time->data)
		{
			info.PTZConfig->def_timeout = onvif_parse_time(p_time->data);
		}

		return true;
	}

};



#endif //__ONVIFPROTOCOL_H__
