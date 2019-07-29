#pragma once
#include "rtp.h"
#include "RTSPProtocol.h"

class rtpOverTcp :public rtp
{
public:
	rtpOverTcp(bool _isserver, RTSPProtocol* _protocol, const RTSP_MEDIA_INFO& _rtspmedia, const RTPDataCallback& _datacallback)
		:rtp(_isserver,_rtspmedia,_datacallback),protocol(_protocol), rtpsn(0)
	{
		protocol->setRTPOverTcpCallback(RTSPProtocol::ExternDataCallback(&rtpOverTcp::rtpovertcpCallback, this));
	}
	~rtpOverTcp()
	{

	}
	void rtpovertcpCallback(bool isvideo, const char* data, uint32_t len)
	{
		if (isvideo && !rtspmedia.media.bHasVideo) return;
		if (!isvideo && !rtspmedia.media.bHasAudio) return;

		if (len < sizeof(RTPHEADER))
		{
			assert(0);
			return;
		}
		RTPHEADER* header = (RTPHEADER*)data;

		framedata += std::string(data + sizeof(RTPHEADER), len - sizeof(RTPHEADER));

		if (header->m)
		{
			datacallback(isvideo, ntohl(header->ts), framedata);
			framedata = "";
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
private:
	RTSPProtocol*			 protocol;
	uint16_t				 rtpsn;
	String					 framedata;
};