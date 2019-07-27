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



//ý����Ϣ�ṹ����
struct MEDIA_INFO
{
	int nStreamCount;			//������

	bool bHasVideo;				//�Ƿ�����Ƶ��
	bool bHasAudio;				//�Ƿ�����Ƶ��

	std::string szRange;		//������(���Ϊʵʱ����ʽһ��Ϊ: 0- ���� now-)
	std::string szControl;		//������Ϣ

	STREAM_INFO stStreamVideo;	//��Ƶ����Ϣ
	STREAM_INFO stStreamAudio;	//��Ƶ����Ϣ
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






//���Ŷ�λʱ�䷽ʽ
#define  NTP_TIME	0
#define  CLOCK_TIME	1
#define  SMPTE_TIME	2

//֡��Ϣ�ṹ����
typedef struct _FRAME_INFO
{
	int nFrameType;		//֡���� 0:I֡, 1:p֡, 2��Ƶ֡, 3:sps
	long nFrameNum;		//֡�� ��1��ʼ
	long lFrameTimestamp;//֡RTPʱ���
	int nFrameRate;		//֡��
	int nFramesize;		//֡��С(�����ݳ���һ��)
	int nFrameWidth;	//ͼ����
	int nFrameHeight;	//ͼ��߶�
	int nYear;			//֡����ʱ���� (֡������ɵı���ʱ��)
	int nMonth;			//֡����ʱ���� (֡������ɵı���ʱ��)
	int nDay;			//֡����ʱ���� (֡������ɵı���ʱ��)
	int nHour;			//֡����ʱ���� (֡������ɵı���ʱ��)
	int nMinute;		//֡����ʱ���� (֡������ɵı���ʱ��)
	int nSecond;		//֡����ʱ���� (֡������ɵı���ʱ��)

}FRAME_INFO, *LPFRAME_INFO;

//����Ϣ�ṹ����
typedef struct _STREAM_INFO
{
	int  nPayLoad;				//��������
	int	 nWidth;				//ͼ����(ֻ����Ƶ��Ч)
	int  nHight;				//ͼ��߶�(ֻ����Ƶ��Ч)
	int  nSampRate;				//������
	int  nBandwidth;			//����(�е�ý����Ϣ����û������)
	int  nChannles;				//ͨ����(ֻ����Ƶ��Ч)

	double	 fFramRate;			//֡��(һ��ֻ��������Ƶ������Ϣ)

	char szProtocol[8];		//����Э��(һ��ΪRTP)
	char szMediaName[128];		//ý������
	char szTrackID[256];		//track id ������������
	char szCodec[10];			//���뷽ʽ
	char szSpsPps[512];		//sps��Ϣ(һ��ΪBase64�ı��봮,���ڳ�ʼ��������,һ��ֻ��������Ƶ������Ϣ)

} STREAM_INFO, *LPSTREAM_INFO;




/*��־��¼����*/
typedef enum 
{
	LOG_LEVEL_NONE		= 0x00, //����¼�κ���־
	LOG_LEVEL_NORMAL	= 0x01, //��¼������־
	LOG_LEVEL_WARNING	= 0x02, //��¼����;�����־
	LOG_LEVEL_ERROR		= 0x03, //ֻ��¼������־
}LOG_LEVEL;

/*Ѷ��CODE_ID����*/
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

/*RTP payload ����*/
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
	//0x13 ����
	//0x14 δ����
	//0x15 δ����
	//0x16 δ����
	//0x17 δ����
	AUDIO_G726_40_8000_1 = 0x64,//��̬ 100
	AUDIO_G726_32_8000_1 = 0x65,//��̬ 101
	AUDIO_G726_24_8000_1 = 0x66,//��̬ 102
	AUDIO_G726_16_8000_1 = 0x67,//��̬ 103
	AUDIO_G729D_8000_1	 = 0x68,//��̬ 104
	AUDIO_G729E_8000_1	 = 0x69,//��̬ 105
	AUDIO_GSM_EFR_8000_1 = 0x6a,//��̬ 106
	AUDIO_L8_var_1		 = 0x6b,//��̬ 107
	AUDIO_VDVI_var_1	 = 0x6c,//��̬ 108

}AUDIO_FORMAT;

typedef enum 
{
	VIDEO_JPEG_90000	= 0x1a,
	VIDEO_MPV_90000		= 0x1f,
	VIDEO_MP2T_90000	= 0x20,
	VIDEO_H261_90000	= 0x21,
	VIDEO_H263_90000	= 0x22,
	//��̬
	VIDEO_H264_90000	= 0x60,

}VIDEO_FORMAT;

typedef struct _MEDIA_PARAMETER
{
	bool bHasVideo;//�Ƿ�����Ƶ
	int  nVideoRtpPayLoad;//��ƵRTP��������
	int  nVideoXMDecodID;//Ѷ��DECODER ID
	int  nVideoSampRate;//��Ƶ������
	int  nVideoWidth;
	int  nVideoHight;
	float  fVideoFrameRate;

	bool bHasAudio;//�Ƿ�����Ƶ
	int  nAudioRtpPayLoad;//��ƵRTP��������
	int  nAudioXMDecodID;//Ѷ��DECODER ID
	int  nAudioSampRate;//��Ƶ������
	int  nAudioChannels;//��Ƶ������
	
}MEDIA_PARAMETER, *LPMEDIA_PARAMETER;

//�����䷽ʽ
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
