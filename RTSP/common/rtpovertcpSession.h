#pragma once
#include "rtpSession.h"
#include "RTSPProtocol.h"
#include "rtspHeaderSdp.h"
#include "rtcp.h"

class rtpOverTcpSesssion :public RTPSession
{
public:
	rtpOverTcpSesssion(const shared_ptr<RTSPProtocol>& _protocol, const shared_ptr<STREAM_TRANS_INFO>& _transport, const MediaDataCallback& _datacallback, const ContorlDataCallback& _contorlcallback)
		:RTPSession(_transport, _datacallback, _contorlcallback),protocol(_protocol),firsthearbeta(true)
	{}
	~rtpOverTcpSesssion()
	{
	}
	void rtpovertcpContorlCallback(const shared_ptr<STREAM_TRANS_INFO>& mediainfo, const char*  buffer, uint32_t bufferlen)
	{
		contorlcallback(mediainfo, buffer, bufferlen);
	}
	void rtpovertcpMediaCallback(const shared_ptr<STREAM_TRANS_INFO>& mediainfo, const RTPHEADER& rtpheader, const char*  buffer, uint32_t bufferlen)
	{
		datacallback(mediainfo, ntohl(rtpheader.ts), buffer, bufferlen, rtpheader.m);
	}

	void sendContorlData(const shared_ptr<STREAM_TRANS_INFO>& transportinfo, const char*  buffer, uint32_t bufferlen)
	{
		shared_ptr<RTSPProtocol> protocoltmp = protocol.lock();
		if (protocoltmp == NULL) return;

		protocoltmp->sendContrlData(transportinfo, buffer, bufferlen);
	}
	void sendMediaData(const shared_ptr<STREAM_TRANS_INFO>& transportinfo, uint32_t timestmap, const char*  buffer, uint32_t bufferlen, bool mark)
	{
		shared_ptr<RTSPProtocol> protocoltmp = protocol.lock();
		if (protocoltmp == NULL) return;
		
		while (bufferlen > 0)
		{
			uint32_t cansendlen = min(MAXRTPPACKETLEN, bufferlen);

			RTPHEADER rtpheader;
			memset(&rtpheader, 0, sizeof(RTPHEADER));
			rtpheader.v = RTP_VERSION;
			rtpheader.ts = htonl(timestmap);
			rtpheader.seq = htons(transportinfo->rtpsn++);
			rtpheader.pt = transportinfo->streaminfo.nPayLoad;
			rtpheader.m = bufferlen == cansendlen ? mark : false;
			rtpheader.ssrc = htonl(transportinfo->transportinfo.ssrc);
			
			protocoltmp->sendMedia(transportinfo, rtpheader,buffer,bufferlen);

			buffer += cansendlen;
			bufferlen -= cansendlen;
		}
		
	}
	bool onPoolHeartbeat()
	{
		shared_ptr<RTSPProtocol> protocoltmp = protocol.lock();
		if (protocoltmp == NULL) return false;



		std::string hearbeatadata = firsthearbeta ? firstRtpOverTcpRTCPHeartBeat() : normalRtpOverTcpRTCPHeartBeat();
		firsthearbeta = false;

		protocoltmp->sendContrlData(transportinfo, hearbeatadata.c_str(), hearbeatadata.length());	

		return true;
	}
private:
	weak_ptr<RTSPProtocol>	 protocol;

	bool					 firsthearbeta;
	
};