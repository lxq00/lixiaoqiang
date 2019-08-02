#pragma once
#include "HTTP/HTTPParse.h"
using namespace Public::HTTP;
using namespace Public::RTSP;

#define MAXPARSERTSPBUFFERLEN	1024*1024
#define RTPOVERTCPMAGIC		'$'
#define RTSPCMDFLAG			"RTSP/1.0"

#define RTSPCONENTTYPESDP	"application/sdp"

class RTSPProtocol:public HTTPParse
{
public:
	struct SendItem
	{
		String				buffer;
		uint32_t			havesendlen;



		SendItem() :havesendlen(0) {}
	};	
	struct TCPInterleaved
	{
		int dataChannel;
		int contrlChannel;
	};
public:
	typedef Function1<void, const shared_ptr<RTSPCommandInfo>&> CommandCallback;
	typedef Function3<void, const shared_ptr<STREAM_TRANS_INFO>&,const RTPHEADER&,const std::vector<CircleBuffer::BufferInfo>&> MediaDataCallback;
	typedef Function3<void, const shared_ptr<STREAM_TRANS_INFO>&,const char*, uint32_t> ContorlDataCallback;
	typedef Function0<void> DisconnectCallback;
public:
	RTSPProtocol(const shared_ptr<Socket>& sock, const CommandCallback& cmdcallback, const DisconnectCallback& disconnectCallback,bool server)
		:HTTPParse(server),m_recvBuffer(MAXPARSERTSPBUFFERLEN)
	{
		m_sock = sock;
		m_cmdcallback = cmdcallback;
		
		m_disconnect = disconnectCallback;
		m_prevalivetime = Time::getCurrentMilliSecond();
		m_bodylen = 0;
		m_haveFindHeaderStart = false;

		m_sock->setSocketBuffer(1024 * 1024 * 4, 1024 * 1024);
		m_sock->setDisconnectCallback(Socket::DisconnectedCallback(&RTSPProtocol::onSocketDisconnectCallback, this));
		m_sock->async_recv(m_recvBuffer.getProductionAddr(), m_recvBuffer.getProductionLength(), Socket::ReceivedCallback(&RTSPProtocol::onSocketRecvCallback, this));
	}
	~RTSPProtocol()
	{
		m_sock->disconnect();
		m_sock = NULL;
	}

	void setRTPOverTcpCallback(const MediaDataCallback& datacallback,const ContorlDataCallback& contorlcallback)
	{
		m_mediadatacallback = datacallback;
		m_contorldatacallback = contorlcallback;
	}

	uint64_t prevalivetime() const { return m_prevalivetime; }
	uint32_t sendListSize()
	{
		uint32_t listsize = 0;

		Guard locker(m_mutex);
		for (std::list<shared_ptr<SendItem> >::iterator iter = m_sendList.begin(); iter != m_sendList.end(); iter++)
		{
			listsize += (*iter)->buffer.length();
		}

		return listsize;
	}

	void sendProtocolData(const std::string& cmdstr)
	{
		if (cmdstr.length() == 0) return;

		logdebug("\r\n%s",cmdstr.c_str());

		Guard locker(m_mutex);
		_addAndCheckSendData(String(cmdstr));
	}
	void sendMedia(const shared_ptr<STREAM_TRANS_INFO>& mediainfo, const RTPHEADER& rtpheader,const char*  buffer, uint32_t bufferlen)
	{
		INTERLEAVEDFRAME frame;
		frame.magic = RTPOVERTCPMAGIC;
		frame.channel = mediainfo->transportinfo.rtp.t.dataChannel;
		frame.rtp_len = htons((uint16_t)(bufferlen + sizeof(RTPHEADER)));

		String sendbuffer;
		uint32_t sendbufferlen = sizeof(INTERLEAVEDFRAME) + sizeof(RTPHEADER) + bufferlen;

		char* rtspheaderptr = sendbuffer.alloc(sendbufferlen);
		memcpy(rtspheaderptr, &frame, sizeof(INTERLEAVEDFRAME));
		memcpy(rtspheaderptr + sizeof(INTERLEAVEDFRAME), &rtpheader, sizeof(RTPHEADER));
		memcpy(rtspheaderptr + sizeof(INTERLEAVEDFRAME) + sizeof(RTPHEADER), buffer, bufferlen);
		sendbuffer.resize(sendbufferlen);

		Guard locker(m_mutex);
		_addAndCheckSendData(sendbuffer);
	}
	void sendContrlData(const shared_ptr<STREAM_TRANS_INFO>& mediainfo, const char* buffer, uint32_t len)
	{
		INTERLEAVEDFRAME frame;
		frame.magic = RTPOVERTCPMAGIC;
		frame.channel = mediainfo->transportinfo.rtp.t.contorlChannel;
		frame.rtp_len = htons(len);

		String contorldata = std::string((char*)&frame, sizeof(frame)) + std::string(buffer, len);


		Guard locker(m_mutex);
		_addAndCheckSendData(contorldata);
	}
	void setMediaTransportInfo(const shared_ptr<MEDIA_INFO>& _rtspmedia)
	{
		m_rtspmedia = _rtspmedia;
	}
private:
	void _sendAndCheckBuffer(const char* buffer = NULL, int len = 0)
	{
		bool needSendData = false;
		//第一次需要发送数据
		if (buffer == NULL && m_sendList.size() == 1 && m_sock != NULL)
		{
			needSendData = true;
		}
		else if(buffer)
		{
			if (len < 0 || m_sendList.size() <= 0)
			{
				return;
			}

			{
				shared_ptr<SendItem>& item = m_sendList.front();
				if (buffer != item->buffer.c_str() + item->havesendlen)
				{
					return;
				}
				if (item->havesendlen + len > item->buffer.length())
				{
					return;
				}
				item->havesendlen += len;

				if (item->havesendlen == item->buffer.length())
				{
					m_sendList.pop_front();
				}
			}

			needSendData = true;
		}

		if (!needSendData) return;

		shared_ptr<Socket> tmp = m_sock;
		if (tmp == NULL || m_sendList.size() <= 0) return;

		shared_ptr<SendItem>& item = m_sendList.front();

		const char* sendbuffer = item->buffer.c_str();
		uint32_t sendbufferlen = item->buffer.length();

		m_prevalivetime = Time::getCurrentMilliSecond();

		m_sock->async_send(sendbuffer + item->havesendlen, sendbufferlen - item->havesendlen, Socket::SendedCallback(&RTSPProtocol::onSocketSendCallback, this));
	}
	void _addAndCheckSendData(const String& buffer)
	{
		shared_ptr<SendItem> item = make_shared<SendItem>();
		item->buffer = buffer;

		m_sendList.push_back(item);

		_sendAndCheckBuffer();
	}
	void onSocketSendCallback(const weak_ptr<Socket>& sock, const char* buffer, int len)
	{
		if (len <= 0) return;

		Guard locker(m_mutex);

		_sendAndCheckBuffer(buffer,len);
	}
	void onSocketDisconnectCallback(const weak_ptr<Socket>&, const std::string&)
	{
		m_disconnect();
	}
	void onSocketRecvCallback(const weak_ptr<Socket>& sock, const char* buffer, int len)
	{
		m_prevalivetime = Time::getCurrentMilliSecond();
		if (len <= 0)
		{
			assert(0);
			return;
		}

		if(buffer != m_recvBuffer.getProductionAddr())
		{
			assert(0);
		}

		if ((uint32_t)len > m_recvBuffer.getProductionLength())
		{
			assert(0);
		}

		m_recvBuffer.setProductionLength(len);
		
		while (m_recvBuffer.dataLenght() > 0)
		{
			if (m_cmdinfo != NULL)
			{
				if (m_bodylen > m_cmdinfo->body.length())
				{
					//数据不够
					if (m_recvBuffer.dataLenght() < m_bodylen - m_cmdinfo->body.length()) break;

					uint32_t needlen = m_bodylen - m_cmdinfo->body.length();

					std::vector<CircleBuffer::BufferInfo> info;
					if (!m_recvBuffer.consumeBuffer(0 ,info, needlen)) break;

					for (size_t i = 0; i < info.size(); i++)
					{
						m_cmdinfo->body += std::string(info[i].bufferAddr, info[i].bufferLen);
					}
				}
				
				{
					m_cmdcallback(m_cmdinfo);
					m_cmdinfo = NULL;
					m_bodylen = 0;
					m_haveFindHeaderStart = false;
				}
				
			}
			else if (m_cmdinfo == NULL && m_haveFindHeaderStart)
			{
				std::string usedstr;
				shared_ptr<HTTPParse::Header> header = HTTPParse::parse(m_recvBuffer,&usedstr);

				if (header != NULL)
				{
					logdebug("\r\n%s", usedstr.c_str());

					m_cmdinfo = make_shared<RTSPCommandInfo>();
					m_cmdinfo->method = header->method;
					m_cmdinfo->url = header->url.href();
					m_cmdinfo->verinfo.protocol = header->verinfo.protocol;
					m_cmdinfo->verinfo.version = header->verinfo.version;
					m_cmdinfo->statuscode = header->statuscode;
					m_cmdinfo->statusmsg = header->statusmsg;
					m_cmdinfo->headers = std::move(header->headers);

					m_cmdinfo->cseq = m_cmdinfo->header("CSeq").readInt();

					m_bodylen = m_cmdinfo->header(Content_Length).readInt();

					if (m_bodylen == 0)
					{
						m_cmdcallback(m_cmdinfo);
						m_cmdinfo = NULL;
						m_bodylen = 0;
						m_haveFindHeaderStart = false;
					}
				}
				//没结束，说明数据不够
				else
				{
					break;
				}
			}
			else if (m_recvBuffer.readChar(0) == RTPOVERTCPMAGIC)
			{
				if (m_recvBuffer.dataLenght() < sizeof(INTERLEAVEDFRAME)) break;

				INTERLEAVEDFRAME frame;
				if(!m_recvBuffer.readBuffer(0,&frame,sizeof(INTERLEAVEDFRAME))) break;

				uint32_t datalen = ntohs(frame.rtp_len);
				
				if (datalen <= 0 || datalen >= 0xffff)
				{
					//跳过数据
					m_recvBuffer.setConsumeLength(1);
					continue;
				}

				if (datalen + sizeof(INTERLEAVEDFRAME) > m_recvBuffer.dataLenght())
				{
					break;
				}

				if (m_rtspmedia)
				{
					for (std::list<shared_ptr<STREAM_TRANS_INFO> >::iterator iter = m_rtspmedia->infos.begin(); iter != m_rtspmedia->infos.end(); iter++)
					{
						shared_ptr<STREAM_TRANS_INFO> transportinfo = *iter;
						if(transportinfo->transportinfo.transport != TRANSPORT_INFO::TRANSPORT_RTP_TCP) continue;

						if (frame.channel == transportinfo->transportinfo.rtp.t.dataChannel)
						{
							RTPHEADER rtpheader;
							if (!m_recvBuffer.readBuffer(sizeof(INTERLEAVEDFRAME), &rtpheader, sizeof(RTPHEADER))) break;
							
							if (rtpheader.v != RTP_VERSION) break;

							std::vector<CircleBuffer::BufferInfo> info;
							if (!m_recvBuffer.readBuffer(sizeof(INTERLEAVEDFRAME) + sizeof(RTPHEADER), info, datalen - sizeof(RTPHEADER))) break;

							m_mediadatacallback(transportinfo,rtpheader, info);
							break;
						}
						else if (frame.channel == transportinfo->transportinfo.rtp.t.contorlChannel)
						{
							std::vector<CircleBuffer::BufferInfo> info;
							if (!m_recvBuffer.readBuffer(sizeof(INTERLEAVEDFRAME), info, datalen)) break;

							std::string buffertmp;
							const char* buffertmpaddr = NULL;

							if (info.size() == 1) buffertmpaddr = info[0].bufferAddr;
							else
							{
								for (size_t i = 0; i < info.size(); i++)
								{
									buffertmp += std::string(info[i].bufferAddr, info[i].bufferLen);
								}
								buffertmpaddr = buffertmp.c_str();
							}

							m_contorldatacallback(transportinfo, buffertmpaddr, datalen);
							break;
						}
					}
				}
				m_recvBuffer.setConsumeLength(datalen + sizeof(INTERLEAVEDFRAME));
			}
			else if(!m_haveFindHeaderStart)
			{
				uint32_t ret = checkIsRTSPCommandStart(m_recvBuffer);
				if (ret == 0)
				{
					m_recvBuffer.setConsumeLength(1);
					continue;
				}
				else if (ret == 1)
				{
					break;
				}
				else
				{
					m_haveFindHeaderStart = true;
				}
			}
			else
			{
				assert(0);
				int a = 0;
			}
		}
		
		shared_ptr<Socket> socktmp = sock.lock();
		if (socktmp != NULL)
		{
			socktmp->async_recv(m_recvBuffer.getProductionAddr(), m_recvBuffer.getProductionLength(), Socket::ReceivedCallback(&RTSPProtocol::onSocketRecvCallback, this));
		}
	}
	// return 0 not cmonnand,return 1 not sure need wait,return 2 is command
	uint32_t checkIsRTSPCommandStart(CircleBuffer& buffer)
	{
		static std::string rtspcmdflag = "rtsp";

		uint32_t datalen = buffer.dataLenght();
		uint32_t readpos = 0;

		char readbufferarray[5] = { 0, };
		while (readpos < datalen)
		{
			char ch = buffer.readChar(readpos);

			//b不是可现实字符，不是
			if (!isCanShowChar(ch)) return 0;

			readbufferarray[0] = readbufferarray[1];
			readbufferarray[1] = readbufferarray[2];
			readbufferarray[2] = readbufferarray[3];
			readbufferarray[3] = ch;
			readbufferarray[4] = 0;

			if (readpos >= 4)
			{
				if (strncasecmp(readbufferarray, rtspcmdflag.c_str(), 4) == 0) return 2;
			}

			readpos++;
		}
		return 1;
	}
	//判断是否是显示字符
	bool isCanShowChar(char ch)
	{
		return ((ch >= 0x20 && ch < 0x7f) || ch == '\r' || ch == '\n');
	}
public:
	shared_ptr<Socket>			m_sock;
private:
	CommandCallback				m_cmdcallback;
	DisconnectCallback			m_disconnect;

	MediaDataCallback			m_mediadatacallback;
	ContorlDataCallback			m_contorldatacallback;

	CircleBuffer				m_recvBuffer;

	Mutex						m_mutex;
	std::list<shared_ptr<SendItem> > m_sendList;

	uint64_t					m_prevalivetime;


	bool						m_haveFindHeaderStart;
	shared_ptr<RTSPCommandInfo>	m_cmdinfo;
	uint32_t					m_bodylen;
	
	shared_ptr<MEDIA_INFO>		m_rtspmedia;
};


