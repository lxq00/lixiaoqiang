#ifndef __ONVIFCLIENT_DEFS_H__
#define __ONVIFCLIENT_DEFS_H__
#include "Base/Base.h"
#include "Network/Network.h"
using namespace Public::Base;
using namespace Public::Network;

#ifdef WIN32
#ifdef ONVIFCLIENT_EXPORTS1
#define ONVIFCLIENT_API __declspec(dllexport)
#else
#define ONVIFCLIENT_API __declspec(dllimport)
#endif
#else
#define ONVIFCLIENT_API
#endif


namespace Public {
namespace Onvif {
namespace OnvifClientDefs {

struct XAddr
{
	int     port;           // onvif port
	std::string    host;       // ip of xaddrs
	std::string    url;       // /onvif/device_service

	XAddr():port(0){}
};

//设备信息
struct Info
{
	std::string Manufacturer;		//工厂信息
	std::string Model;				//设备信息
	std::string FirmwareVersion;	//固件版本
	std::string SerialNumber;		//序列号
	std::string HardwareId;			//硬件标识
//	std::string Name;				//设备名称
};

//设备能力集
struct Capabilities
{
	struct _Medis{
		XAddr xaddr;
		BOOL RTPMulticast;
		BOOL RTP_TCP;
		BOOL RTP_RTSP_TCP;
		BOOL Support;

		_Medis():RTPMulticast(false),RTP_TCP(false),RTP_RTSP_TCP(false),Support(false){}
	}Media;
	struct _PTZ{
		XAddr		xaddr;
		BOOL        Support;

		_PTZ():Support(false){}
	}PTZ;
	struct _Events {
		XAddr		xaddr;
		BOOL        Support;

		_Events():Support(false){}
	}Events;
	struct _Mmessage {
		XAddr		xaddr;
		BOOL        Support;

		_Mmessage():Support(false){}
	}Mmessage;
} ;

//暂时不解析GetScopes，太复杂，目前没需求
typedef struct {

}Scopes;

struct _VideoSource {
	int  width, height, x, y;

	int  use_count;
	std::string token;
	std::string stream_name;
	std::string source_token;

	_VideoSource():width(0),height(0),x(0),y(0),use_count(0){}
};

typedef enum {
	VIDEO_ENCODING_JPEG = 0,
	VIDEO_ENCODING_MPEG4 = 1,
	VIDEO_ENCODING_H264 = 2,
	VIDEO_ENCODING_UNKNOWN = 3,
}VIDEO_ENCODING;

typedef enum {
	H264_PROFILE_Baseline = 0,
	H264_PROFILE_Main = 1,
	H264_PROFILE_Extended = 2,
	H264_PROFILE_High = 3,
}H264_PROFILE;

struct _VideoEncoder
{
	float quality;
	int  width;
	int  height;
	int  use_count;
	int  session_timeout;

	std::string name;
	std::string token;

	VIDEO_ENCODING  encoding;

	int  framerate_limit;
	int  encoding_interval;
	int  bitrate_limit;

	/* H264Configuration */
	int  gov_len;
	H264_PROFILE  h264_profile;

	_VideoEncoder():quality(0),width(0),height(0),use_count(0),session_timeout(0),framerate_limit(0),encoding_interval(0),bitrate_limit(0),gov_len(0){}
} ;

struct _Range
{
	float min;
	float max;

	_Range() { min = max = 0; }
};

struct _PTZConfig
{
	int  use_count;
	int  def_timeout;

	std::string name;
	std::string token;
	std::string nodeToken;

	struct _Speed
	{
		int pan_tilt_x;
		int pan_tilt_y;
		int zoom;

		_Speed():pan_tilt_x(0),pan_tilt_y(0),zoom(0){}
	} def_speed;

	_Range pantilt_x;
	_Range pantilt_y;
	_Range zoom;

	_PTZConfig():use_count(0),def_timeout(0){}
} ;

struct ConfigurationOptions
{
	int used;

	_Range absolute_pantilt_x;
	_Range absolute_pantilt_y;
	_Range absolute_zoom;

	_Range relative_pantilt_x;
	_Range relative_pantilt_y;
	_Range relative_zoom;

	_Range continuous_pantilt_x;
	_Range continuous_pantilt_y;
	_Range continuous_zoom;

	_Range pantilt_speed;
	_Range zoom_speed;
	_Range timeout;

	ConfigurationOptions():used(0){}
};

//配置信息
struct ProfileInfo {
	shared_ptr<_VideoSource> VideoSource;
	shared_ptr<_VideoEncoder> VideoEncoder;
	shared_ptr<_PTZConfig> PTZConfig;


	std::string name;
	std::string token;
	bool fixed;

	ProfileInfo():fixed(false){}
};

struct Profiles {
	std::vector<ProfileInfo> infos;
};

struct NetworkInterfaces {
	std::string		name;
	std::string		macaddr;
	std::string		ipaddr;
	bool			dhcp;

	NetworkInterfaces():dhcp(false){}
};
typedef struct {

}VideoEncoderConfigurations;

typedef struct {

}ContinuousMove;

typedef struct{

}AbsoluteMove;

struct PTZCtrl
{
	enum {
		PTZ_CTRL_PAN  =  0,
		PTZ_CTRL_ZOOM =  1,
	}ctrlType;
	double           panTiltX;
	double           panTiltY;
	float           zoom;
	int				duration;

	PTZCtrl():panTiltX(0),panTiltY(0),zoom(0),duration(0){}
} ;


struct StreamUrl
{
	std::string url;
};

struct SnapUrl
{
	std::string url;
};


}
}
}



#endif //__ONVIFCLIENT_H__
