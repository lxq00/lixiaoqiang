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
	void rtpovertcpCallback(uint32_t channel, const RTSPBuffer& data)
	{
		//contorl data
		if (channel % 2 != 0)
		{
			int a = 0;
		}
		else
		{
			if (data.size() < sizeof(RTPHEADER))
			{
				assert(0);
				return;
			}
			RTPHEADER* header = (RTPHEADER*)data.buffer();

			datacallback(true, ntohl(header->ts), RTSPBuffer(data, data.buffer() + sizeof(RTPHEADER), data.size() - sizeof(RTPHEADER)),header->m);
		}
	}
	void sendData(bool isvideo, uint32_t timestmap, const RTSPBuffer& data, bool mark)
	{
		const char* buffer = data.buffer();
		uint32_t bufferlen = data.size();
	
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

			std::string rtpheaderstr = std::string((const char*)& header, sizeof(RTPHEADER));

			protocol->sendMedia(isvideo, rtpheaderstr, RTSPBuffer(data, buffer, cansendlen));

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