#ifndef __ONVIFCLIENT_DEFS_H__
#define __ONVIFCLIENT_DEFS_H__
#include "Base/Base.h"
#include "Network/Network.h"
using namespace Public::Base;
using namespace Public::Network;

#ifdef WIN32
#ifdef ONVIFCLIENT_EXPORTS
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

typedef struct
{
	int     port;           // onvif port
	std::string    host;       // ip of xaddrs
	std::string    url;       // /onvif/device_service
} XAddr;

//设备信息
typedef struct
{
	std::string Manufacturer;		//工厂信息
	std::string Model;				//设备信息
	std::string FirmwareVersion;	//固件版本
	std::string SerialNumber;		//序列号
	std::string HardwareId;			//硬件标识
	std::string Name;				//设备名称
} Info;

//设备能力集
typedef struct
{
	struct {
		XAddr xaddr;
		BOOL RTPMulticast;
		BOOL RTP_TCP;
		BOOL RTP_RTSP_TCP;
		BOOL Support;
	}Media;
	struct {
		XAddr xaddr;
		BOOL        Support;
	}PTZ;
	struct {
		XAddr xaddr;
		BOOL        Support;
	}Events;
	struct {
		XAddr xaddr;
		BOOL        Support;
	}Mmessage;
} Capabilities;

//暂时不解析GetScopes，太复杂，目前没需求
typedef struct {

}Scopes;

struct _VideoSource {
	int  width, height, x, y;

	int  use_count;
	std::string token;
	std::string stream_name;
	std::string source_token;
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
} ;

typedef struct
{
	float min;
	float max;
} range;

struct _PTZConfig
{
	int  use_count;
	int  def_timeout;

	std::string name;
	std::string token;
	std::string nodeToken;

	struct
	{
		int pan_tilt_x;
		int pan_tilt_y;
		int zoom;
	} def_speed;

	range pantilt_x;
	range pantilt_y;
	range zoom;
} ;

struct ConfigurationOptions
{
	int used;

	range absolute_pantilt_x;
	range absolute_pantilt_y;
	range absolute_zoom;

	range relative_pantilt_x;
	range relative_pantilt_y;
	range relative_zoom;

	range continuous_pantilt_x;
	range continuous_pantilt_y;
	range continuous_zoom;

	range pantilt_speed;
	range zoom_speed;
	range timeout;
};

//配置信息
struct ProfileInfo {
	shared_ptr<_VideoSource> VideoSource;
	shared_ptr<_VideoEncoder> VideoEncoder;
	shared_ptr<_PTZConfig> PTZConfig;


	std::string name;
	std::string token;
	std::string stream_URL;
	std::string snap_URL;
	std::string eventSubscription;
	bool fixed;
};

typedef struct {
	std::vector<ProfileInfo> infos;
}Profiles;

typedef struct {

}NetworkInterfaces;

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
} ;



}
}
}



#endif //__ONVIFCLIENT_H__
