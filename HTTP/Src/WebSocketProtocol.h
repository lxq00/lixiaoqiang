#pragma once

#include "Base/Base.h"
#include "Network/Network.h"
#include "HTTP/WebSocket.h"
using namespace Public::Base;
using namespace Public::Network;

namespace Public {
namespace HTTP {

class WebSocketProtocol
{
	struct FrameInfo
	{
		int			playloadlen;
		bool		mask;
		char		maskkey[4];
		std::string	data;
		int			datatype;

		FrameInfo():playloadlen(0),mask(false), datatype(0)
		{
			maskkey[0] = maskkey[1] = maskkey[2] = maskkey[3] = 0;
		}
		~FrameInfo()
		{
		}
	};

	struct SendItemInfo
	{
		std::string		needsenddata;
		uint32_t		havesendlen;

		SendItemInfo():havesendlen(0){}
	};
public:
	WebSocketProtocol(bool client):isClient(client), errorCode(408)
	{
		headerParseFinish = false;
		recvBuffer = new char[MAXRECVBUFFERLEN + 10];
		recvBufferLen = 0;
	}
	virtual ~WebSocketProtocol() 
	{
		sock = NULL;
		SAFE_DELETE(recvBuffer);
	}
	void close()
	{
		weak_ptr<Socket>socktmp = sock;
		if (sock != NULL) sock->disconnect();
		sock = NULL;
		while (socktmp.lock() != NULL) Thread::sleep(10);
	}
	bool buildAndSend(const std::string& data, WebSocketDataType type)
	{
		std::string protocolstr;
		char maskkey[4] = { 0 };
		char header[32] = { 0 };
		int headerlen = 0;

		//build websocket header
		{	
			header[headerlen] |= 1 << 7;	//fin
			header[headerlen] |= (type == WebSocketDataType_Txt ? 1 : 2) & 0x0f;
			headerlen += 1;
			
			header[headerlen] |= (isClient ? 1 : 0) << 7;	//mask
			header[headerlen] |= (data.length() <= 125 ? data.length() : (
				(data.length() > 125 && data.length() <= 0xffff) ? 126 : 127
				)) & 0x7f;	//playload len
			headerlen += 1;

			//ext playload len
			if (data.length() > 125 && data.length() <= 0xffff)
			{
				*(unsigned short*)(header+headerlen) = htons((unsigned short)data.length());
				headerlen += 2;
			}
			else if (data.length() > 0xffff)
			{
				*(unsigned long*)(header+headerlen + 4) = htonl(data.length());
				headerlen += 8;
			}
			//client mask key
			if (isClient)
			{
				for (uint32_t i = 0; i < 4; i++)
				{
					maskkey[i] = (((long)this) >> (i * 8))&0xff;
					while (maskkey[i] == 0) 
					{
						maskkey[i] = ((char)Time::getCurrentMilliSecond())&0xff;
					}
				}

				header[headerlen + 0] = maskkey[0];
				header[headerlen + 1] = maskkey[1];
				header[headerlen + 2] = maskkey[2];
				header[headerlen + 3] = maskkey[3];
				headerlen += 4;
			}
		}

		protocolstr = std::string(header, headerlen) + data;

		//mask data
		if (isClient)
		{
			char* datatmp = (char*)protocolstr.c_str();
			datatmp += headerlen;

			for (uint32_t i = 0; i < data.length(); i++)
			{
				uint32_t j = i % 4;
				datatmp[i] = datatmp[i] ^ maskkey[j];
			}
		}


		addDataAndCheckSend(protocolstr.c_str(),protocolstr.length());

		return true;
	}
	bool setSocketAndStart(const shared_ptr<Socket>& _sock)
	{
		sock = _sock;
		sock->setDisconnectCallback(Socket::DisconnectedCallback(&WebSocketProtocol::socketDisConnectcallback, this));
		sock->async_recv(recvBuffer, MAXRECVBUFFERLEN, Socket::ReceivedCallback(&WebSocketProtocol::socketRecvCallback, this));
	
		return true;
	}
	shared_ptr<Socket> getsocket() { return sock; }
	bool ready() { return headerParseFinish; }
private:
	const char* parseProtocol(const char* buffer, int len)
	{
		while (len > 0)
		{
			if (frame == NULL)
			{
				frame = make_shared<FrameInfo>();

				const char* tmp = parseWebSocketHeard(buffer, len, frame);
				if (tmp == NULL) break;

				len = buffer + len - tmp;
				buffer = tmp;
			}

			int needdatalen = min(len, (int)(frame->playloadlen - frame->data.length()));
			if (needdatalen > 0)
			{
				frame->data += std::string(buffer, needdatalen);
				buffer += needdatalen;
				len -= needdatalen;
			}

			if (frame->playloadlen == frame->data.length())
			{
				if (frame->datatype == 1 || frame->datatype == 2)
				{
					if (frame->mask)
					{
						char* buffertmp = (char*)frame->data.c_str();
						uint32_t datalen = frame->data.length();
						for (uint32_t i = 0; i < datalen; i++)
						{
							uint32_t j = i % 4;
							buffertmp[i] = buffertmp[i] ^ frame->maskkey[j];
						}
					}
					
					dataCallback(frame->data, frame->datatype == 1 ? WebSocketDataType_Txt: WebSocketDataType_Bin);
				}
				frame = NULL;
			}
		}

		return buffer;
	}
	const char* parseWebSocketHeard(const char* buffer, int len,shared_ptr<FrameInfo> frame)
	{
		int needlen = 2;
		if (len < needlen) return NULL;

		int fine = buffer[0] >> 7;
		frame->datatype = buffer[0] & 0xf;
		frame->mask = (uint8_t)buffer[1] >> 7;
		frame->playloadlen = buffer[1] & 0x7f;
		if (frame->playloadlen == 126)
		{
			if (len < needlen + 2) return NULL;
			
			frame->playloadlen = ntohs(*(unsigned short*)(buffer + needlen));
			needlen += 2;
		}
		else if (frame->playloadlen == 127)
		{
			if (len < needlen + 8) return NULL;

			frame->playloadlen = ntohl(*(unsigned long*)(buffer + needlen + 4));
			needlen += 8;
		}

		if (frame->mask)
		{
			if (len < needlen + 4) return NULL;

			frame->maskkey[0] = buffer[needlen + 0];
			frame->maskkey[1] = buffer[needlen + 1];
			frame->maskkey[2] = buffer[needlen + 2];
			frame->maskkey[3] = buffer[needlen + 3];
			needlen += 4;
		}

		return buffer += needlen;
	}
	virtual void dataCallback(const std::string& data, WebSocketDataType type) = 0;
	virtual void socketDisConnectcallback(const weak_ptr<Socket>& /*connectSock*/, const std::string&) = 0;
	virtual void headerParseReady() {}
	void socketRecvCallback(const weak_ptr<Socket>& /*sock*/, const char* buffer, int len)
	{
		if (len <= 0) return;

		recvBufferLen += len;
		uint32_t lentmp = recvBufferLen;
		const char* buftmp = recvBuffer;

		if (!headerParseFinish && lentmp > 0)
		{
			const char* tmp = parseHeader(buftmp, lentmp);

			headerParseReady();

			lentmp = buftmp + lentmp - tmp;
			buftmp = tmp;
		}
		if (headerParseFinish && lentmp > 0)
		{
			const char* tmp = parseProtocol(buftmp, lentmp);
			lentmp = buftmp + lentmp - tmp;
			buftmp = tmp;
		}

		if (lentmp > 0 && lentmp != recvBufferLen)
		{
			memmove(recvBuffer, buftmp, lentmp);
		}
		recvBufferLen = lentmp;

		shared_ptr<Socket> tmp = sock;
		if (tmp != NULL)
		{
			tmp->async_recv(recvBuffer + recvBufferLen, MAXRECVBUFFERLEN - recvBufferLen,
				Socket::ReceivedCallback(&WebSocketProtocol::socketRecvCallback, this));
		}
	}
	const char* parseHeader(const char* buffer, int bufferlen)
	{
		int pos = String::indexOf(buffer, bufferlen, HTTPHEADEREND);
		if (pos == -1) return buffer;

		const char* endheaderbuf = buffer + pos + strlen(HTTPHEADEREND);

		const char* firsttmp = parseHeaderFlag(buffer, endheaderbuf - buffer);
		if (firsttmp == NULL) return buffer;

		parseHeaderContent(firsttmp, endheaderbuf - firsttmp);

		headerParseFinish = true;

		return endheaderbuf;
	}
	const char* parseHeaderFlag(const char* buffer, int len)
	{
		int pos = String::indexOf(buffer, len, HTTPHREADERLINEEND);
		if (pos == -1) return NULL;

		std::vector<std::string> tmp = String::split(buffer, pos, " ");
		
		//不是客户端解析的是客户端返回的数据
		if (!isClient)
		{	
			if (tmp.size() != 3) return NULL;

			url = tmp[1];
		}
		else
		{
			//解析服务端返回的数据
			if (tmp.size() < 2) return NULL;

			errorCode = Value(tmp[1]).readInt();
		}

		return buffer + pos + strlen(HTTPHREADERLINEEND);
	}
	bool parseHeaderContent(const char* buffer, int len)
	{
		std::vector<std::string> values = String::split(buffer, len, HTTPHREADERLINEEND);
		for (uint32_t i = 0; i < values.size(); i++)
		{
			std::string strtmp = values[i];
			const char* tmpstr = strtmp.c_str();
			const char* tmp = strchr(tmpstr, ':');
			if (tmp != NULL)
			{
				headers[String::strip(std::string(tmpstr, tmp - tmpstr))] = String::strip(tmp + 1);
			}
		}

		return true;
	}
	void dataSendCallback(const weak_ptr<Socket>& ,const char*,int len)
	{
		shared_ptr<Socket> tmp = sock;
		if (tmp == NULL) return;

		//发送失败，socket异常后，退出发送
		if (len < 0) return;

		Guard locker(mutex);
		{
			if (sendlist.size() <= 0) return;

			shared_ptr<SendItemInfo> sendtmp = sendlist.front();
			//数据出错
			if (sendtmp->havesendlen + len > sendtmp->needsenddata.length()) return;

			sendtmp->havesendlen += len;
			if (sendtmp->havesendlen == sendtmp->needsenddata.length())
			{
				sendlist.pop_front();
			}
		}
		
		{
			if (sendlist.size() <= 0) return;

			shared_ptr<SendItemInfo> sendtmp = sendlist.front();
			const char* sendbuffertmp = sendtmp->needsenddata.c_str() + sendtmp->havesendlen;
			uint32_t  sendbufferlen = sendtmp->needsenddata.length() - sendtmp->havesendlen;

			tmp->async_send(sendbuffertmp, sendbufferlen, Socket::SendedCallback(&WebSocketProtocol::dataSendCallback, this));
		}
	}
protected:
	void addDataAndCheckSend(const char* buf, int len)
	{
		shared_ptr<Socket> socktmp = sock;

		if (socktmp == NULL) return;

		Guard locker(mutex);

		shared_ptr<SendItemInfo> sendinfo = make_shared<SendItemInfo>();
		sendinfo->needsenddata = std::string(buf, len);

		sendlist.push_back(sendinfo);

		//只有当前加入的一条数据，启动发送
		if (sendlist.size() == 1)
		{
			shared_ptr<SendItemInfo> sendtmp = sendlist.front();
			const char* sendbuffertmp = sendtmp->needsenddata.c_str();
			uint32_t  sendbufferlen = sendtmp->needsenddata.length();

			socktmp->async_send(sendbuffertmp, sendbufferlen, Socket::SendedCallback(&WebSocketProtocol::dataSendCallback, this));
		}
	}
private:
	shared_ptr<Socket>				sock;
	shared_ptr<FrameInfo>			frame;
	char*							recvBuffer;
	int								recvBufferLen;
	bool							headerParseFinish;
	bool							isClient;

	Mutex							mutex;
	std::list<shared_ptr<SendItemInfo> > sendlist;
public:
	std::map<std::string, Value>	headers;
	uint32_t						errorCode;
	URL								url;
};


}
}

