#pragma once
#include "Base/Base.h"
#include "RTSP/Defs.h"
#include "HTTP/HTTPParse.h"
using namespace Public::Base;
using namespace Public::HTTP;


// Transport: RTP/AVP/TCP;interleaved=0-1
// Transport: RTP/AVP;unicast;client_port=4588-4589;server_port=6256-6257
// Transport: RTP/AVP;multicast;destination=224.2.0.1;port=3456-3457;ttl=16
struct TRANSPORT_INFO
{
	TRANSPORT_INFO():transport(TRANSPORT_NONE), /*multicast(TRANSPORT_INFO::MULTICAST_UNICAST),*/ ssrc(0)
	{
		rtp.t.dataChannel = 0;
		rtp.t.contorlChannel = 0;
	}

	enum {
		TRANSPORT_NONE = 0,
		TRANSPORT_RTP_UDP = 1,
		TRANSPORT_RTP_TCP,
		TRANSPORT_RAW,
	} transport; // RTSP_TRANSPORT_xxx

	//enum {
	//	MULTICAST_NONE = 0,
	//	MULTICAST_UNICAST = 1,
	//	MULTICAST_MULTICAST,
	//} multicast; // 0-unicast/1-multicast, default multicast

	int ssrc; // RTP only(synchronization source (SSRC) identifier) 4-bytes
	
	union rtsp_header_transport_rtp_u
	{
		//struct rtsp_header_transport_multicast_t
		//{
		//	int ttl; // multicast only
		//	unsigned short port1, port2; // multicast only
		//} m;

		struct rtsp_header_transport_unicast_t
		{
			unsigned short client_port1, client_port2; // unicast RTP/RTCP port pair, RTP only
			unsigned short server_port1, server_port2; // unicast RTP/RTCP port pair, RTP only
		} u;

		struct rtsp_header_transport_tcp_t
		{
			int		dataChannel;
			int		contorlChannel;
		}t;
	} rtp;
};


//流信息结构定义
struct STREAM_INFO
{
	int  nPayLoad;				//荷载类型
	int	 nWidth;				//图像宽度(只有视频有效)
	int  nHight;				//图像高度(只有视频有效)
	int  nSampRate;				//采样率
	int  nBandwidth;			//带宽(有的媒体信息可能没有描述)

	double	 fFramRate;			//帧率(一般只存在于视频描述信息)

//	std::string szProtocol;		//传输协议(一般为RTP)
	std::string szMediaName;	//媒体名称
	std::string szTrackID;		//track id 用于请求命令
	std::string szCodec;		//编码方式

	int profile_level_id;
	std::string szSpsPps;		//sps信息(一般为Base64的编码串,用于初始化解码器,一般只存在于视频描述信息)

	std::string Media_header;


	STREAM_INFO():nPayLoad(0),nWidth(0),nHight(0),nSampRate(0),nBandwidth(0)/*,nTrackID(0)*/,fFramRate(0), profile_level_id(0){}
};
class RTPSession;
struct STREAM_TRANS_INFO
{
	STREAM_INFO				streaminfo;
	TRANSPORT_INFO			transportinfo;
	shared_ptr<RTPSession>	rtpsession;

	STREAM_TRANS_INFO(){}
};

//媒体信息结构定义
struct RTSP_API MEDIA_INFO
{
	std::string		ssrc;				//ssrc
	
	uint32_t startRange;
	uint32_t stopRange;

	std::list< shared_ptr<STREAM_TRANS_INFO> >	infos;
	
	MEDIA_INFO() :startRange(0),stopRange(0){}


	const shared_ptr<STREAM_TRANS_INFO> streamInfo(const std::string& medianame)const
	{
		for (std::list<shared_ptr<STREAM_TRANS_INFO> >::const_iterator iter = infos.begin(); iter != infos.end(); iter++)
		{
			if (strcasecmp((*iter)->streaminfo.szMediaName.c_str(), medianame.c_str()) == 0)
			{
				return *iter;
			}
		}

		return NULL;
	}
	const shared_ptr<STREAM_TRANS_INFO> videoStreamInfo(const std::string& medianame)const
	{
		return streamInfo("video");
	}
	const shared_ptr<STREAM_TRANS_INFO> audioStreamInfo(const std::string& medianame)const
	{
		return streamInfo("audio");
	}
	shared_ptr<STREAM_TRANS_INFO> addVideoStreamInfo()
	{
		shared_ptr<STREAM_TRANS_INFO> videoinfo = addStreamInfo("video");

		{
			videoinfo->streaminfo.nPayLoad = 96;
			videoinfo->streaminfo.szCodec = "H264";
			videoinfo->streaminfo.nSampRate = 90000;
		}
		
		return videoinfo;
	}

	shared_ptr<STREAM_TRANS_INFO> addAudioStreamInfo()
	{
		shared_ptr<STREAM_TRANS_INFO> videoinfo = addStreamInfo("audio");

		{
			videoinfo->streaminfo.nPayLoad = 97;
			videoinfo->streaminfo.szCodec = "G726-16";
			videoinfo->streaminfo.nSampRate = 8000;
		}

		return videoinfo;
	}

	shared_ptr<STREAM_TRANS_INFO> addStreamInfo(const std::string& flag)
	{
		shared_ptr<STREAM_TRANS_INFO> info = make_shared<STREAM_TRANS_INFO>();

		info->streaminfo.szTrackID = "trackID=" + Value(infos.size()).readString();
		info->streaminfo.szMediaName = flag;

		infos.push_back(info);
		


		return info;
	}
	//是否有视频流
	bool hasVideo() const
	{
		return streamInfo("video") != NULL;
	}
	//是否有音频流
	bool hasAudio() const
	{
		return streamInfo("audio") != NULL;
	}

	//清除非音视频的流信息
	void cleanExStreamInfo()
	{
		for (std::list<shared_ptr<STREAM_TRANS_INFO> >::const_iterator iter = infos.begin(); iter != infos.end(); )
		{
			if (strcasecmp((*iter)->streaminfo.szMediaName.c_str(), "video") == 0 || strcasecmp((*iter)->streaminfo.szMediaName.c_str(), "audio") == 0)
			{
				iter++;
			}
			else
			{
				infos.erase(iter++);
			}
		}
	}
};

enum ERTSP_RANGE_TIME {
	RTSP_RANGE_NONE = 0,
	RTSP_RANGE_SMPTE = 1, // relative to the start of the clip 
	RTSP_RANGE_NPT,  // relative to the beginning of the presentation
	RTSP_RANGE_CLOCK, // absolute time, ISO 8601 timestamps, UTC(GMT)
};

struct RANGE_INFO
{
	enum ERTSP_RANGE_TIME type;

	uint64_t from; // ms
	uint64_t to; // ms

	RANGE_INFO():type(RTSP_RANGE_NONE),from(0),to(0){}
};


//RTSP命令信息
struct RTSP_API RTSPCommandInfo:public HTTPHeader
{
	std::string body;

	uint32_t		cseq;

	RTSPCommandInfo():cseq(0){}
};

/*
+---------------+---------------+---------------+---------------+
|0|1|2|3|4|5|6|7|0|1|2|3|4|5|6|7|0|1|2|3|4|5|6|7|0|1|2|3|4|5|6|7|
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|V=2|P|X|   CC  |M|	    PT      |     sequence number 			|
+---------------+---------------+---------------+---------------+
|		                    timestamp	        			    |
+---------------+---------------+---------------+---------------+
|		                       SSRC		        			    |
+---------------+---------------+---------------+---------------+
|		                       CSRC		        			    |
|							  ......							|
+---------------+---------------+---------------+---------------+
*/
typedef struct  _RTPHEADER
{
	unsigned short   cc : 4;
	unsigned short   x : 1;
	unsigned short   p : 1;
	unsigned short   v : 2;
	unsigned short   pt : 7;
	unsigned short   m : 1;

	unsigned short   seq;
	unsigned long    ts;
	unsigned long    ssrc;

}RTPHEADER, *LPRTPHEADER;

#define RTP_VERSION 2

#define MAXRTPPACKETLEN		1440

struct INTERLEAVEDFRAME
{
	unsigned int magic : 8;// $
	unsigned int channel : 8; //0-1
	unsigned int rtp_len : 16;
};

typedef Function0<uint32_t> AllockUdpPortCallback;
