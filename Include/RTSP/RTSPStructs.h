#pragma once
#include "Base/Base.h"



// Transport: RTP/AVP/TCP;interleaved=0-1
// Transport: RTP/AVP;unicast;client_port=4588-4589;server_port=6256-6257
// Transport: RTP/AVP;multicast;destination=224.2.0.1;port=3456-3457;ttl=16
struct TRANSPORT_INFO
{
	enum {
		TRANSPORT_NONE = 0,
		TRANSPORT_RTP_UDP = 1,
		TRANSPORT_RTP_TCP,
		TRANSPORT_RAW,
	} transport; // RTSP_TRANSPORT_xxx
	enum {
		MULTICAST_NONE = 0,
		MULTICAST_UNICAST = 1,
		MULTICAST_MULTICAST,
	} multicast; // 0-unicast/1-multicast, default multicast
	std::string destination; // IPv4/IPv6
	std::string source; // IPv4/IPv6
	int layer; // rtsp setup response only
	enum {
		MODE_NONE,
		MODE_PLAY = 1,
		MODE_RECORD
	} mode; // PLAY/RECORD, default PLAY, rtsp setup response only
	int append; // use with RECORD mode only, rtsp setup response only
	int interleaved1, interleaved2; // rtsp setup response only
	union rtsp_header_transport_rtp_u
	{
		struct rtsp_header_transport_multicast_t
		{
			int ttl; // multicast only
			unsigned short port1, port2; // multicast only
		} m;

		struct rtsp_header_transport_unicast_t
		{
			unsigned short client_port1, client_port2; // unicast RTP/RTCP port pair, RTP only
			unsigned short server_port1, server_port2; // unicast RTP/RTCP port pair, RTP only
			int ssrc; // RTP only(synchronization source (SSRC) identifier) 4-bytes
		} u;
	} rtp;
};



//媒体信息结构定义
struct MEDIA_INFO
{
	int nStreamCount;			//流个数

	bool bHasVideo;				//是否有视频流
	bool bHasAudio;				//是否有音频流

	std::string szRange;		//流长度(如果为实时流格式一般为: 0- 或者 now-)
	std::string szControl;		//控制信息

	STREAM_INFO stStreamVideo;	//视频流信息
	STREAM_INFO stStreamAudio;	//音频流信息
};




enum ERTSP_RANGE_TIME {
	RTSP_RANGE_SMPTE = 1, // relative to the start of the clip 
	RTSP_RANGE_SMPTE_30 = RTSP_RANGE_SMPTE,
	RTSP_RANGE_SMPTE_25,
	RTSP_RANGE_NPT,  // relative to the beginning of the presentation
	RTSP_RANGE_CLOCK, // absolute time, ISO 8601 timestamps, UTC(GMT)
};

enum ERTSP_RANGE_TIME_VALUE {
	RTSP_RANGE_TIME_NORMAL = 1,
	RTSP_RANGE_TIME_NOW, // npt now
	RTSP_RANGE_TIME_NOVALUE, // npt don't set from value: -[npt-time]
};

struct RANGE_INFO
{
	enum ERTSP_RANGE_TIME type;
	enum ERTSP_RANGE_TIME_VALUE from_value;
	enum ERTSP_RANGE_TIME_VALUE to_value;

	uint64_t from; // ms
	uint64_t to; // ms

	uint64_t time; // range time parameter(in ms), 0 if no value
};






//播放定位时间方式
#define  NTP_TIME	0
#define  CLOCK_TIME	1
#define  SMPTE_TIME	2

//帧信息结构定义
typedef struct _FRAME_INFO
{
	int nFrameType;		//帧类型 0:I帧, 1:p帧, 2音频帧, 3:sps
	long nFrameNum;		//帧号 从1开始
	long lFrameTimestamp;//帧RTP时间戳
	int nFrameRate;		//帧率
	int nFramesize;		//帧大小(与数据长度一致)
	int nFrameWidth;	//图像宽度
	int nFrameHeight;	//图像高度
	int nYear;			//帧生成时间年 (帧分析完成的本地时间)
	int nMonth;			//帧生成时间年 (帧分析完成的本地时间)
	int nDay;			//帧生成时间年 (帧分析完成的本地时间)
	int nHour;			//帧生成时间年 (帧分析完成的本地时间)
	int nMinute;		//帧生成时间年 (帧分析完成的本地时间)
	int nSecond;		//帧生成时间年 (帧分析完成的本地时间)

}FRAME_INFO, *LPFRAME_INFO;

//流信息结构定义
typedef struct _STREAM_INFO
{
	int  nPayLoad;				//荷载类型
	int	 nWidth;				//图像宽度(只有视频有效)
	int  nHight;				//图像高度(只有视频有效)
	int  nSampRate;				//采样率
	int  nBandwidth;			//带宽(有的媒体信息可能没有描述)
	int  nChannles;				//通道数(只有音频有效)

	double	 fFramRate;			//帧率(一般只存在于视频描述信息)

	char szProtocol[8];		//传输协议(一般为RTP)
	char szMediaName[128];		//媒体名称
	char szTrackID[256];		//track id 用于请求命令
	char szCodec[10];			//编码方式
	char szSpsPps[512];		//sps信息(一般为Base64的编码串,用于初始化解码器,一般只存在于视频描述信息)

} STREAM_INFO, *LPSTREAM_INFO;




/*日志记录级别*/
typedef enum 
{
	LOG_LEVEL_NONE		= 0x00, //不记录任何日志
	LOG_LEVEL_NORMAL	= 0x01, //记录所有日志
	LOG_LEVEL_WARNING	= 0x02, //记录错误和警告日志
	LOG_LEVEL_ERROR		= 0x03, //只记录错误日志
}LOG_LEVEL;

/*讯美CODE_ID定义*/
typedef enum
{
	XM_PLAYER_CODEID_H263 = 0x00,
	XM_PLAYER_CODEID_H264 = 0x01,
	XM_PLAYER_CODEID_H265 = 0x02,
	XM_PLAYER_CODEID_Mpeg4 = 0x03,
	XM_PLAYER_CODEID_JPEG = 0x04,
	XM_PLAYER_CODEID_H261 = 0x05,
	XM_PLAYER_CODEID_MP2T = 0x06,
	XM_PLAYER_CODEID_MPV = 0x07,
	XM_PLAYER_CODEID_MJEPG = 0x10,
	XM_PLAYER_CODEID_G711A = 0x20,
	XM_PLAYER_CODEID_G711Mu = 0x21,
	XM_PLAYER_CODEID_ADPCM = 0x22,
	XM_PLAYER_CODEID_PCM = 0x23,
	XM_PLAYER_CODEID_G726 = 0x24,
	XM_PLAYER_CODEID_MPEG2 = 0x25,
	XM_PLAYER_CODEID_AMR = 0x26,
	XM_PLAYER_CODEID_AAC = 0x27,
	XM_PLAYER_CODEID_MP3 = 0x28,
	XM_PLAYER_CODEID_OGG = 0x29,
	XM_PLAYER_CODEID_PSStream = 0x30,
	XM_PLAYER_CODEID_Hik = 0x40,
	XM_PLAYER_CODEID_DaHua = 0x41,
	XM_PLAYER_CODEID_XunMei = 0x42,
	XM_PLAYER_CODEID_BlueStar7K = 0x43,
	XM_PLAYER_CODEID_Bosch = 0x44,
	XM_PLAYER_CODEID_GSM = 0x50,
	XM_PLAYER_CODEID_DVI4 = 0x51,
	XM_PLAYER_CODEID_LPC = 0x52,
	XM_PLAYER_CODEID_L16 = 0x53,
	XM_PLAYER_CODEID_QCELP = 0x54,
	XM_PLAYER_CODEID_CN = 0x55,
	XM_PLAYER_CODEID_MPA = 0x56,
	XM_PLAYER_CODEID_G728 = 0x57,
	XM_PLAYER_CODEID_G729 = 0x58,
	XM_PLAYER_CODEID_G729D = 0x59,
	XM_PLAYER_CODEID_G729E = 0x5a,
	XM_PLAYER_CODEID_GSMESR = 0x5b,
	XM_PLAYER_CODEID_L8 = 0x5c,
	XM_PLAYER_CODEID_G723 = 0x5d,
	XM_PLAYER_CODEID_G722 = 0x5e,
	XM_PLAYER_CODEID_1016 = 0x5f,
	XM_PLAYER_CODEID_G721 = 0x60,
}AV_CODEID;

/*RTP payload 定义*/
typedef enum 
{
	AUDIO_PCMU_8000_1	= 0x00,
	AUDIO_1016_8000_1	= 0x01,
	AUDIO_G721_8000_1	= 0x02,
	AUDIO_GSM_8000_1	= 0x03,
	AUDIO_G723_8000_1	= 0x04,
	AUDIO_DVI4_8000_1	= 0x05,
	AUDIO_DVI4_16000_1	= 0x06,
	AUDIO_LPC_8000_1	= 0x07,
	AUDIO_PCMA_8000_1	= 0x08,
	AUDIO_G722_8000_1	= 0x09,
	AUDIO_L16_44100_2	= 0x0a,
	AUDIO_L16_44100_1	= 0x0b,
	AUDIO_QCELP_8000_1	= 0x0c,
	AUDIO_CN_8000_1		= 0x0d,
	AUDIO_MPA_90000_1	= 0x0e,
	AUDIO_G728_8000_1	= 0x0f,
	AUDIO_DVI4_11025_1	= 0x10,
	AUDIO_DVI4_22050_1	= 0x11,
	AUDIO_G729_8000_1	= 0x12,
	//0x13 保留
	//0x14 未定义
	//0x15 未定义
	//0x16 未定义
	//0x17 未定义
	AUDIO_G726_40_8000_1 = 0x64,//动态 100
	AUDIO_G726_32_8000_1 = 0x65,//动态 101
	AUDIO_G726_24_8000_1 = 0x66,//动态 102
	AUDIO_G726_16_8000_1 = 0x67,//动态 103
	AUDIO_G729D_8000_1	 = 0x68,//动态 104
	AUDIO_G729E_8000_1	 = 0x69,//动态 105
	AUDIO_GSM_EFR_8000_1 = 0x6a,//动态 106
	AUDIO_L8_var_1		 = 0x6b,//动态 107
	AUDIO_VDVI_var_1	 = 0x6c,//动态 108

}AUDIO_FORMAT;

typedef enum 
{
	VIDEO_JPEG_90000	= 0x1a,
	VIDEO_MPV_90000		= 0x1f,
	VIDEO_MP2T_90000	= 0x20,
	VIDEO_H261_90000	= 0x21,
	VIDEO_H263_90000	= 0x22,
	//动态
	VIDEO_H264_90000	= 0x60,

}VIDEO_FORMAT;

typedef struct _MEDIA_PARAMETER
{
	bool bHasVideo;//是否有视频
	int  nVideoRtpPayLoad;//视频RTP荷载类型
	int  nVideoXMDecodID;//讯美DECODER ID
	int  nVideoSampRate;//视频采样率
	int  nVideoWidth;
	int  nVideoHight;
	float  fVideoFrameRate;

	bool bHasAudio;//是否有音频
	int  nAudioRtpPayLoad;//音频RTP荷载类型
	int  nAudioXMDecodID;//讯美DECODER ID
	int  nAudioSampRate;//音频采样率
	int  nAudioChannels;//音频声道数
	
}MEDIA_PARAMETER, *LPMEDIA_PARAMETER;

//流传输方式
typedef enum
{
	RTP_RECV_TYPE_TCP,
	RTP_RECV_TYPE_UDP
}RTP_RECV_Type;

#define AUDIO_MPEG4_GENERIC "mpeg4-generic"
#define AUDIO_G726 "G726"
#define AUDIO_L16 "L16"
#define VIDEO_H264 "H264"

#endif
