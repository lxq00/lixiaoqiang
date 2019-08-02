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
	}
	~rtpOverTcp()
	{
	}
	void rtpovertcpCallback(uint32_t channel, const RTPHEADER& rtpheader, const char*  buffer, uint32_t bufferlen)
	{
		//contorl data
		if (channel % 2 != 0)
		{
			int a = 0;
		}
		else
		{
			datacallback(true, ntohl(rtpheader.ts), buffer, bufferlen , rtpheader.m);
		}
	}
	void sendData(bool isvideo, uint32_t timestmap, const char*  buffer, uint32_t bufferlen, bool mark)
	{
	//	while (bufferlen > 0)
		{
			uint32_t cansendlen = bufferlen;// min(MAXRTPPACKETLEN, bufferlen);

			RTPHEADER rtpheader;
			memset(&rtpheader, 0, sizeof(RTPHEADER));
			rtpheader.v = RTP_VERSION;
			rtpheader.ts = htonl(timestmap);
			rtpheader.seq = htons(rtpsn++);
			rtpheader.pt = isvideo ? rtspmedia.media.stStreamVideo.nPayLoad : rtspmedia.media.stStreamAudio.nPayLoad;
			rtpheader.m = bufferlen == cansendlen ? mark : false;
			rtpheader.ssrc = htonl(isvideo ? rtspmedia.videoTransport.ssrc : rtspmedia.audioTransport.ssrc);
			
			protocol->sendMedia(isvideo, rtpheader,buffer,bufferlen);

			buffer += cansendlen;
			bufferlen -= cansendlen;
		}
		
	}
	void onPoolHeartbeat()
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

	bool					 firsthearbeta;
	
};