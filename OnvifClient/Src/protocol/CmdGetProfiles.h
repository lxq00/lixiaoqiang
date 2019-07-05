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
	virtual bool parse(const XMLObject::Child& body)
	{
		profileInfo = make_shared<OnvifClientDefs::Profiles>();

		XMLObject::Child resp = body.getChild("trt:GetProfilesResponse");
		if (!resp) return false;
		
		XMLObject::Child& p_profiles = resp.firstChild("trt:Profiles");
		while (p_profiles)
		{
			OnvifClientDefs::ProfileInfo profile;

			profile.fixed = p_profiles.attribute("fixed").readBool();
			profile.token = p_profiles.attribute("token");

			if (parseProfileInfo(p_profiles, profile))
			{
				profileInfo->infos.push_back(profile);
			}

			p_profiles = resp.nextChild();
		}

		return true;
	}
	virtual bool parseProfileInfo(const XMLObject::Child& p_profiles,OnvifClientDefs::ProfileInfo& info)
	{
		OnvifClientDefs::ProfileInfo* profileInfo = &info;

		const XMLObject::Child& pname = p_profiles.getChild("tt:Name");
		if (pname)
		{
			profileInfo->name = pname.data();
		}
		else
		{
			return false;
		}

		const XMLObject::Child& videosrc = p_profiles.getChild("tt:VideoSourceConfiguration");
		if (videosrc)
		{
			if (!parseVideoSource(videosrc,info))
			{
				profileInfo->VideoSource = NULL;
			}
		}

		const XMLObject::Child& videoenc = p_profiles.getChild("tt:VideoEncoderConfiguration");
		if (videoenc)
		{
			if (!parseVideoEncoder(videoenc, info))
			{
				profileInfo->VideoEncoder = NULL;
			}
		}
		const XMLObject::Child& ptzcfg = p_profiles.getChild("tt:PTZConfiguration");
		if (ptzcfg)
		{
			if (!parsePTZCfg(ptzcfg, info))
			{
				profileInfo->PTZConfig = NULL;
			}
		}

		return TRUE;
	}
private:
	bool parseVideoSource(const XMLObject::Child& videosrc, OnvifClientDefs::ProfileInfo& info)
	{
		info.VideoSource = make_shared<OnvifClientDefs::_VideoSource>();

		info.VideoSource->token = videosrc.attribute("token");
		info.VideoSource->stream_name = videosrc.getChild("tt:Name").data();
		info.VideoSource->use_count = videosrc.getChild("tt:UseCount").data().readInt();
		info.VideoSource->source_token = videosrc.getChild("tt:SourceToken").data();

		if (info.VideoSource->stream_name == "" || info.VideoSource->source_token == "") return false;


		const XMLObject::Child& bounds = videosrc.getChild("tt:Bounds");
		if (!bounds) return false;

		info.VideoSource->height = bounds.attribute("height").readInt();
		info.VideoSource->width = bounds.attribute("width").readInt();
		info.VideoSource->x = bounds.attribute("x").readInt();
		info.VideoSource->y = bounds.attribute("y").readInt();

		return true;
	}

	bool parseVideoEncoder(const XMLObject::Child& videoenc, OnvifClientDefs::ProfileInfo& info)
	{
		info.VideoEncoder = make_shared<OnvifClientDefs::_VideoEncoder>();

		info.VideoEncoder->token = videoenc.attribute("token");
		info.VideoEncoder->name = videoenc.getChild("token");
		
		if (info.VideoEncoder->name == "" || info.VideoEncoder->token == "") return false;

		info.VideoEncoder->use_count = videoenc.getChild("tt:UseCount").data().readInt();
		info.VideoEncoder->encoding = onvif_parse_encoding(videoenc.getChild("tt:Encoding").data());

		info.VideoEncoder->width = videoenc.getChild("tt:Resolution").getChild("tt:Width").data().readInt();
		info.VideoEncoder->height = videoenc.getChild("tt:Resolution").getChild("tt:Height").data().readInt();
		info.VideoEncoder->quality = videoenc.getChild("tt:Quality").data().readFloat();
		
		info.VideoEncoder->framerate_limit = videoenc.getChild("tt:RateControl").getChild("tt:FrameRateLimit").data().readInt();
		info.VideoEncoder->encoding_interval = videoenc.getChild("tt:RateControl").getChild("tt:EncodingInterval").data().readInt();
		info.VideoEncoder->bitrate_limit = videoenc.getChild("tt:RateControl").getChild("tt:BitrateLimit").data().readInt();


		if (info.VideoEncoder->encoding == OnvifClientDefs::VIDEO_ENCODING_H264)
		{
			info.VideoEncoder->gov_len = videoenc.getChild("tt:H264").getChild("tt:GovLength").data().readInt();
			info.VideoEncoder->h264_profile = onvif_parse_h264_profile(videoenc.getChild("tt:H264").getChild("tt:H264Profile").data());
		}

		info.VideoEncoder->session_timeout = videoenc.getChild("tt:SessionTimeout").data().readInt();

		return true;
	}

	bool parsePTZCfg(const XMLObject::Child& ptzcfg, OnvifClientDefs::ProfileInfo& info)
	{
		info.PTZConfig = make_shared<OnvifClientDefs::_PTZConfig>();

		info.PTZConfig->token = ptzcfg.attribute("token");
		info.PTZConfig->name = ptzcfg.getChild("tt:Name").data();

		if (info.PTZConfig->token == "" || info.PTZConfig->name == "") return false;

		info.PTZConfig->use_count = ptzcfg.getChild("tt:UseCount").data().readInt();
		info.PTZConfig->nodeToken = ptzcfg.getChild("tt:NodeToken").data();

		info.PTZConfig->def_speed.pan_tilt_x = ptzcfg.getChild("tt:DefaultPTZSpeed").getChild("tt:PanTilt").attribute("x").readInt();
		info.PTZConfig->def_speed.pan_tilt_y = ptzcfg.getChild("tt:DefaultPTZSpeed").getChild("tt:PanTilt").attribute("y").readInt();
		
		info.PTZConfig->def_speed.zoom = ptzcfg.getChild("tt:DefaultPTZSpeed").getChild("tt:Zoom").attribute("x").readInt();

		info.PTZConfig->def_timeout = ptzcfg.getChild("tt:DefaultPTZTimeout").data().readInt();

		return true;
	}

};



#endif //__ONVIFPROTOCOL_H__
