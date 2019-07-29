#pragma once
#include "rtp.h"
#include "RTSPProtocol.h"

static char normaltheartbeatData[] = { 0x81, 0xc9, 0x00, 0x07, 0x27, 0xf7, 0x0a, 0x8d, 0x04, 0xfe, 0x32, 0x09
,0x00, 0xff, 0xff, 0xff, 0x00, 0x01, 0x99, 0x6a, 0x00, 0x00, 0x03, 0x90, 0x0a, 0xda, 0x00, 0x00
,0x00, 0x00, 0x23, 0x91, 0x81, 0xca, 0x00, 0x04, 0x27, 0xf7, 0x0a, 0x8d, 0x01, 0x07, 0x52, 0x41
,0x4e, 0x44, 0x4f, 0x4e, 0x47, 0x00, 0x00, 0x00 };

static char firstheartbeatData[] = { 0x80 ,0xC9 ,0x00 ,0x01 ,0x80 ,0x00 ,0x00 ,0x29 ,0x81 ,0xCA ,0x00 ,0x04
,0x80 ,0x00 ,0x00 ,0x29 ,0x01 ,0x09 ,0x62 ,0x6C  ,0x75 ,0x65 ,0x73 ,0x74 ,0x6F ,0x72 ,0x6D,0x00 };

class rtpOverTcp :public rtp
{
public:
	rtpOverTcp(bool _isserver, RTSPProtocol* _protocol, const RTSP_MEDIA_INFO& _rtspmedia, const RTPDataCallback& _datacallback)
		:rtp(_isserver,_rtspmedia,_datacallback),protocol(_protocol), rtpsn(0), firsthearbeta(true)
	{
		protocol->setRTPOverTcpCallback(RTSPProtocol::ExternDataCallback(&rtpOverTcp::rtpovertcpCallback, this));

		timer = make_shared<Timer>("rtpOverTcp");
		timer->start(Timer::Proc(&rtpOverTcp::onPoolTimerProc, this), 5000, 5000);
	}
	~rtpOverTcp()
	{
		timer = NULL;
	}
	void rtpovertcpCallback(uint32_t channel, const char* data, uint32_t len)
	{
		//contorl data
		if (channel % 2 != 0)
		{
			int a = 0;
		}
		else
		{
			if (len < sizeof(RTPHEADER))
			{
				assert(0);
				return;
			}
			RTPHEADER* header = (RTPHEADER*)data;

			framedata += std::string(data + sizeof(RTPHEADER), len - sizeof(RTPHEADER));

			if (header->m)
			{
				datacallback(true, ntohl(header->ts), framedata);
				framedata = "";
			}
		}
	}
	void sendData(bool isvideo, uint32_t timesmap, const String& data)
	{
		if (isvideo && !rtspmedia.media.bHasVideo) return;
		if (!isvideo && !rtspmedia.media.bHasAudio) return;

		const char* buffer = data.c_str();
		uint32_t bufferlen = data.length();

		while (bufferlen > 0)
		{
			uint32_t cansendlen = min(MAXRTPPACKETLEN, bufferlen);

			RTPHEADER header;
			memset(&header, 0, sizeof(header));
			header.v = 2;
			header.ts = htonl(timesmap);
			header.seq = htons(rtpsn++);
			header.pt = isvideo ? rtspmedia.media.stStreamVideo.nPayLoad : rtspmedia.media.stStreamAudio.nPayLoad;
			header.m = bufferlen == cansendlen;
			header.ssrc = htonl(isvideo ? rtspmedia.videoTransport.ssrc : rtspmedia.audioTransport.ssrc);

			protocol->sendMedia(isvideo, (const char*)& header, sizeof(RTPHEADER), buffer, cansendlen);

			buffer += cansendlen;
			bufferlen -= cansendlen;
		}
	}
	void onPoolTimerProc(unsigned long)
	{
		if (firsthearbeta && protocol)
		{
			protocol->sendContrlData(firstheartbeatData, sizeof(firstheartbeatData));
			firsthearbeta = false;
		}
		else if (!firsthearbeta && protocol)
		{
			protocol->sendContrlData(normaltheartbeatData, sizeof(normaltheartbeatData));
		}
	}
private:
	RTSPProtocol*			 protocol;
	uint16_t				 rtpsn;
	String					 framedata;

	bool					 firsthearbeta;
	shared_ptr<Timer>		 timer;
};