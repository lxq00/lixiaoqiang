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


//����Ϣ�ṹ����
struct STREAM_INFO
{
	int  nPayLoad;				//��������
//	int	 nWidth;				//ͼ����(ֻ����Ƶ��Ч)
//	int  nHight;				//ͼ��߶�(ֻ����Ƶ��Ч)
	int  nSampRate;				//������
//	int  nBandwidth;			//����(�е�ý����Ϣ����û������)
//	int  nTrackID;				//ͨ����(ֻ����Ƶ��Ч)

//	double	 fFramRate;			//֡��(һ��ֻ��������Ƶ������Ϣ)

//	std::string szProtocol;		//����Э��(һ��ΪRTP)
//	std::string szMediaName;	//ý������
	std::string szTrackID;		//track id ������������
	std::string szCodec;		//���뷽ʽ
//	std::string szSpsPps;		//sps��Ϣ(һ��ΪBase64�ı��봮,���ڳ�ʼ��������,һ��ֻ��������Ƶ������Ϣ)


	STREAM_INFO():nPayLoad(0),/*nWidth(0),nHight(0),*/nSampRate(0)/*,nBandwidth(0),nTrackID(0),fFramRate(0)*/{}
};

//ý����Ϣ�ṹ����
struct MEDIA_INFO
{
	bool bHasVideo;				//�Ƿ�����Ƶ��
	bool bHasAudio;				//�Ƿ�����Ƶ��

	int		ssrc;				//ssrc
	
	uint32_t startRange;
	uint32_t stopRange;

	STREAM_INFO stStreamVideo;	//��Ƶ����Ϣ
	STREAM_INFO stStreamAudio;	//��Ƶ����Ϣ

	MEDIA_INFO() :bHasVideo(false), bHasAudio(false), ssrc(0), startRange(0),stopRange(0)
	{
		stStreamVideo.nPayLoad = 96;
		stStreamVideo.szCodec = "H264";
		stStreamVideo.nSampRate = 90000;
		stStreamVideo.szTrackID = "trackID=0";

		stStreamAudio.nPayLoad = 97;
		stStreamAudio.szCodec = "G711";
		stStreamAudio.nSampRate = 8000;
		stStreamAudio.szTrackID = "trackID=1";
	}
};

struct RTSP_MEDIA_INFO
{
	MEDIA_INFO	media;
	TRANSPORT_INFO	videoTransport;
	TRANSPORT_INFO	audioTransport;
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


//RTSP������Ϣ
struct RTSP_API RTSPCommandInfo
{
	std::string		method;
	std::string		url;
	struct {
		std::string protocol;
		std::string	version;
	}verinfo;

	int				statuscode;
	std::string		statusmsg;

	std::map<std::string, Value> headers;

	uint32_t		cseq;

	std::string		body;

	RTSPCommandInfo():cseq(0){}
	Value header(const std::string& key) const
	{
		for (std::map<std::string, Value>::const_iterator iter = headers.begin(); iter != headers.end(); iter++)
		{
			if (strcasecmp(key.c_str(), iter->first.c_str()) == 0)
			{
				return iter->second;
			}
		}
		return Value();
	}
};
