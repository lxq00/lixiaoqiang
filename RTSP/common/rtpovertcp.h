#pragma once
#include "rtp.h"
#include "RTSPProtocol.h"
#include "rtspHeaderSdp.h"
#include "rtcp.h"

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
			std::string firstheartbeatData = firstRtpOverTcpRTCPHeartBeat();
			protocol->sendContrlData(firstheartbeatData.c_str(), firstheartbeatData.length());
			firsthearbeta = false;
		}
		else if (!firsthearbeta && protocol)
		{
			std::string normaltheartbeatData = normalRtpOverTcpRTCPHeartBeat();
			protocol->sendContrlData(normaltheartbeatData.c_str(), normaltheartbeatData.length());
		}
	}
private:
	RTSPProtocol*			 protocol;
	uint16_t				 rtpsn;
	String					 framedata;

	bool					 firsthearbeta;
	shared_ptr<Timer>		 timer;
};