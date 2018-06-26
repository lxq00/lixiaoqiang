#ifndef __ASIOSOCKET_OBJCET_DEFINE_H__
#define __ASIOSOCKET_OBJCET_DEFINE_H__
#include "ASIOServerPool.h"
#include "Network/Socket.h"

#ifdef WIN32
typedef int  socklen_t;
#endif

namespace Public{
namespace Network{

///���ͻ���ָ�롢��ͨ����Ϣ���ͽṹ��
class SendInternal
{
public:
	SendInternal(const char* addr,uint32_t size,const Socket::SendedCallback& sended,const NetAddr& point = NetAddr())
	:buffer(addr),bufferLen(size),sendpoint(point),sendCallback(sended)
	{
	}
	virtual ~SendInternal()
	{
	}
	///���÷��͵Ľ����������Ƿ��Ѿ����ͽ���
	virtual bool setSendResultAndCheckIsOver(const shared_ptr<Socket>& sock,int sendlen)
	{
		sendCallback(sock,buffer,sendlen);
		
		return true;
	}
	///��ȡ��Ҫ���͵�����ָ��
	virtual const char* getSendBuffer() {return buffer;}
	///��ȡ��Ҫ���͵����ݳ���
	virtual uint32_t getSendBufferLen() {return bufferLen;}
	
	const NetAddr& getSendAddr() {return sendpoint;}
protected:	
	const char* 					buffer;
	uint32_t 						bufferLen;
	NetAddr							sendpoint;
	Socket::SendedCallback			sendCallback;
};

///���Զ�η��͵ķ���ָ��
class RepeatSendInternal:public SendInternal
{
public:
	RepeatSendInternal(const char* addr,uint32_t size,const Socket::SendedCallback& sended,bool _newbuffer = false)
		:SendInternal(addr,size,sended,_newbuffer){sendBuffer = buffer;sendBufferLen = bufferLen;}
	virtual ~RepeatSendInternal(){}
	///���÷��͵Ľ����������Ƿ��Ѿ����ͽ���
	virtual bool setSendResultAndCheckIsOver(const shared_ptr<Socket>& sock, int sendlen)
	{
		if(sendlen > 0)
		{
			sendBuffer += sendlen;
			sendBufferLen -= sendlen;
		}
		if(sendlen <= 0 || sendBufferLen == 0)
		{
			
			sendCallback(sock,buffer,bufferLen - sendBufferLen);
			
			return true;
		}
		
		return false;
	}
	///��ȡ��Ҫ���͵�����ָ��
	virtual const char* getSendBuffer() {return sendBuffer;}
	///��ȡ��Ҫ���͵����ݳ���
	virtual uint32_t getSendBufferLen() {return sendBufferLen;}
private:
	const char*						sendBuffer;
	uint32_t 						sendBufferLen;
};



///���ݽ��ջ���ָ��
class RecvInternal
{
public:
	RecvInternal(char* addr,uint32_t len):recvBufferAlloc(NULL),recvBuffer((char*)addr),recvBufferLen(len){}
	RecvInternal(uint32_t len) :recvBufferAlloc(NULL), recvBuffer(NULL), recvBufferLen(len)
	{
		recvBufferAlloc = new (std::nothrow)char[len + 100];
		recvBuffer = recvBufferAlloc;
	}
	virtual ~RecvInternal()
	{
		SAFE_DELETEARRAY(recvBufferAlloc);
	}

	virtual void setRecvResult(const shared_ptr<Socket>& sock, int recvLen) {}
	char* getRecvBuffer() const {return recvBuffer;}
	uint32_t getRecvBufferLen() const {return recvBufferLen;}
	boost::asio::ip::udp::endpoint& getRecvPoint() {return recvpoint;}
protected:
	char*							recvBufferAlloc;
	char*							recvBuffer;
	uint32_t 						recvBufferLen;
	boost::asio::ip::udp::endpoint	recvpoint;
};

///TCP�����ݽ�����Ϣ
class TCPRecvInternal:public RecvInternal
{
public:
	TCPRecvInternal(char* addr,uint32_t len,const Socket::ReceivedCallback& recved):RecvInternal(addr,len),recvCallback(recved){}
	TCPRecvInternal(uint32_t len, const Socket::ReceivedCallback& recved) :RecvInternal(len), recvCallback(recved) {}
	~TCPRecvInternal(){}
	virtual void setRecvResult(const shared_ptr<Socket>& sock, int recvLen)
	{
		recvCallback(sock,recvBuffer,recvLen);
	}
private:
	Socket::ReceivedCallback		recvCallback;
};

///UDP�����ݽ�����Ϣ
class udpRecvInternal:public RecvInternal
{
public:
	udpRecvInternal(char* addr,uint32_t len,const Socket::RecvFromCallback& recved):RecvInternal(addr,len),recvCallback(recved){}
	udpRecvInternal(uint32_t len, const Socket::RecvFromCallback& recved):RecvInternal(len), recvCallback(recved) {}
	~udpRecvInternal(){}
	virtual void setRecvResult(const shared_ptr<Socket>& sock, int recvLen)
	{
		NetAddr recvaddr(recvpoint.address().to_string(),recvpoint.port());
		recvCallback(sock,recvBuffer,recvLen,recvaddr);
	}
private:
	Socket::RecvFromCallback		recvCallback;
};

class UserThreadInfo
{
protected:
	UserThreadInfo():mustQuit(false),usedCallbackThreadId(0),usedCallbackThreadNum(0){}
	~UserThreadInfo(){}
	void quit() 
	{
		Guard locker(mutex);
		mustQuit = true;
	}
	void waitAllOtherCallbackThreadUsedEnd()////�ȴ����лص��߳�ʹ�ý���
	{
		uint32_t currThreadId = Thread::getCurrentThreadID();
		while(1)
		{
			{
				Guard locker(mutex);
				if(usedCallbackThreadNum == 0 || (usedCallbackThreadNum == 1 && usedCallbackThreadId == currThreadId))
				{
					///���û���߳�ʹ�ã��������Լ��̹߳ر��Լ������Թر�
					break;
				}
			}
			Thread::sleep(10);
		}
	}
	bool _callbackThreadUsedStart()///��¼��ǰ�ص��߳�ʹ����Ϣ
	{
		uint32_t currThreadId = Thread::getCurrentThreadID();
		usedCallbackThreadId += currThreadId;
		++usedCallbackThreadNum;

		return true;
	}
	bool callbackThreadUsedEnd() ///��ǰ�ص��߳�ʹ�ý���
	{
		Guard locker(mutex);
		
		uint32_t currThreadId = Thread::getCurrentThreadID();
		usedCallbackThreadId -= currThreadId;
		--usedCallbackThreadNum;

		return true;
	}
protected:
	Mutex											mutex;
	bool											mustQuit;				///�ǲ����Ѿ����ر�
	uint64_t										usedCallbackThreadId;	//����ʹ�ûص����߳�ID��
	Base::AtomicCount								usedCallbackThreadNum;	//����ʹ�ûص����߳���
};

}
}


#endif //__ASIOSOCKET_OBJCET_H__
