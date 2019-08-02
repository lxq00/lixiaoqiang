#pragma once
#include "rtpSession.h"
#include "rtcp.h"

class rtpOverTcpSesssion :public RTPSession
{
public:
	typedef Function3<void, const shared_ptr<STREAM_TRANS_INFO>&, const char*, uint32_t > SendContrlDataCallback;
	typedef Function4<void, const shared_ptr<STREAM_TRANS_INFO>&, const RTPHEADER&, const char*, uint32_t > SendMediaDataCallback;
public:
	rtpOverTcpSesssion(const shared_ptr<STREAM_TRANS_INFO>& _transport, 
		const SendMediaDataCallback& _sendmediacallback,const SendContrlDataCallback& _sendcontrlcallback,
		const MediaDataCallback& _datacallback, const ContorlDataCallback& _contorlcallback)
		:RTPSession(_transport, _datacallback, _contorlcallback),firsthearbeta(true)
		,sendmediacallback(_sendmediacallback),sendcontorlcallback(_sendcontrlcallback)
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
		sendcontorlcallback(transportinfo, buffer, bufferlen);
	}
	void sendMediaData(const shared_ptr<STREAM_TRANS_INFO>& transportinfo, uint32_t timestmap, const char*  buffer, uint32_t bufferlen, bool mark)
	{
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
			
			sendmediacallback(transportinfo, rtpheader,buffer,bufferlen);

			buffer += cansendlen;
			bufferlen -= cansendlen;
		}
		
	}
	bool onPoolHeartbeat()
	{
		std::string hearbeatadata = firsthearbeta ? firstRtpOverTcpRTCPHeartBeat() : normalRtpOverTcpRTCPHeartBeat();
		firsthearbeta = false;

		sendcontorlcallback(transportinfo, hearbeatadata.c_str(), hearbeatadata.length());	

		return true;
	}
private:
	bool					 firsthearbeta;

	SendMediaDataCallback	sendmediacallback;
	SendContrlDataCallback	sendcontorlcallback;
};