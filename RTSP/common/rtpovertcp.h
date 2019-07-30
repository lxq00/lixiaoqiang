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

			datacallback(true, ntohl(header->ts), data + sizeof(RTPHEADER), len - sizeof(RTPHEADER),header->m);
		}
	}
	void sendData(bool isvideo, uint32_t timestmap, const char* buffer, uint32_t bufferlen, bool mark)
	{
		if (buffer == NULL || bufferlen <= 0) return;

		while (bufferlen > 0)
		{
			uint32_t cansendlen = min(MAXRTPPACKETLEN, bufferlen);

			RTPHEADER header;
			memset(&header, 0, sizeof(header));
			header.v = 2;
			header.ts = htonl(timestmap);
			header.seq = htons(rtpsn++);
			header.pt = isvideo ? rtspmedia.media.stStreamVideo.nPayLoad : rtspmedia.media.stStreamAudio.nPayLoad;
			header.m = bufferlen == cansendlen ? mark : false;
			header.ssrc = htonl(isvideo ? rtspmedia.videoTransport.ssrc : rtspmedia.audioTransport.ssrc);

			protocol->sendMedia(isvideo, (const char*)& header, sizeof(RTPHEADER), buffer, cansendlen);

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