#ifndef __ASIOSOCKET_OBJCET_H__
#define __ASIOSOCKET_OBJCET_H__
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
	SendInternal(Socket* _sock,const char* addr,uint32_t size,const Socket::SendedCallback& sended,const NetAddr& point = NetAddr())
	:buffer(addr),bufferLen(size),sendpoint(point),sendCallback(sended),sock(_sock)
	{
	}
	virtual ~SendInternal()
	{
	}
	///���÷��͵Ľ����������Ƿ��Ѿ����ͽ���
	virtual bool setSendResultAndCheckIsOver(int sendlen)
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
	Socket*							sock;
};

///���Զ�η��͵ķ���ָ��
class RepeatSendInternal:public SendInternal
{
public:
	RepeatSendInternal(Socket* _sock,const char* addr,uint32_t size,const Socket::SendedCallback& sended,bool _newbuffer = false)
	:SendInternal(_sock,addr,size,sended,_newbuffer){sendBuffer = buffer;sendBufferLen = bufferLen;}
	virtual ~RepeatSendInternal(){}
	///���÷��͵Ľ����������Ƿ��Ѿ����ͽ���
	virtual bool setSendResultAndCheckIsOver(int sendlen)
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
	RecvInternal(Socket* _sock,char* addr,uint32_t len):recvBufferAlloc(NULL),recvBuffer((char*)addr),recvBufferLen(len),sock((Socket*)_sock){}
	RecvInternal(Socket* _sock,uint32_t len) :recvBufferAlloc(NULL), recvBuffer(NULL), recvBufferLen(len), sock((Socket*)_sock) 
	{
		recvBufferAlloc = new (std::nothrow)char[len];
		recvBuffer = recvBufferAlloc;
	}
	virtual ~RecvInternal()
	{
		SAFE_DELETEARRAY(recvBufferAlloc);
	}

	virtual void setRecvResult(int recvLen) {}
	char* getRecvBuffer() const {return recvBuffer;}
	uint32_t getRecvBufferLen() const {return recvBufferLen;}
	boost::asio::ip::udp::endpoint& getRecvPoint() {return recvpoint;}
protected:
	char*							recvBufferAlloc;
	char*							recvBuffer;
	uint32_t 						recvBufferLen;
	Socket*							sock;
	boost::asio::ip::udp::endpoint	recvpoint;
};

///TCP�����ݽ�����Ϣ
class TCPRecvInternal:public RecvInternal
{
public:
	TCPRecvInternal(Socket* _sock,char* addr,uint32_t len,const Socket::ReceivedCallback& recved):RecvInternal(_sock,addr,len),recvCallback(recved){}
	TCPRecvInternal(Socket* _sock, uint32_t len, const Socket::ReceivedCallback& recved) :RecvInternal(_sock, len), recvCallback(recved) {}
	~TCPRecvInternal(){}
	virtual void setRecvResult(int recvLen)
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
	udpRecvInternal(Socket* _sock,char* addr,uint32_t len,const Socket::RecvFromCallback& recved):RecvInternal(_sock,addr,len),recvCallback(recved){}
	udpRecvInternal(Socket* _sock,uint32_t len, const Socket::RecvFromCallback& recved):RecvInternal(_sock, len), recvCallback(recved) {}
	~udpRecvInternal(){}
	virtual void setRecvResult(int recvLen)
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

///asio socket�����Ķ��󣬸ö������
class ISocketObject:public UserThreadInfo
{
public:
	ISocketObject(Socket* sock);
	~ISocketObject();

	virtual int getHandle() = 0;

	void setSocketPtr(Socket* _sock) {sock = _sock;}

	virtual void socketDisconnect(const char* error){}

	///�Ͽ�socket
	///true��ʾ���������رոö���false��ʾ��Ҫ�����߳����ͷŸö���
	virtual bool destory(); 

	///��������
	bool postReceive(boost::shared_ptr<RecvInternal>& internal);

	///��������
	bool beginSend(boost::shared_ptr<SendInternal>& internal);
	
	NetStatus getStatus() const {return status;}

	void setStatus(NetStatus _status){status = _status;}

	bool setSocketTimeout(uint32_t recvTimeout,uint32_t sendTimeout);
	bool getSocketTimeout(uint32_t& recvTimeout,uint32_t& sendTimeout);
protected:
	virtual void _recvCallbackPtr(const boost::system::error_code& er, std::size_t length);
	virtual void _sendCallbackPtr(const boost::system::error_code& er, std::size_t length);
private:
	virtual bool doStartSend(boost::shared_ptr<SendInternal>& internal) = 0;
	virtual bool doPostRecv(boost::shared_ptr<RecvInternal>& internal) = 0;
	void doStartSendData();
protected:
	Socket*											sock;
	
	boost::shared_ptr<RecvInternal>					currRecvInternal;	///��ǰ���ڽ��յĽڵ�
	std::list<boost::shared_ptr<SendInternal> >		sendInternalList;	///��Ҫ���͵Ľڵ��б�
	boost::shared_ptr<SendInternal>					currSendInternal;	///��ǰ���ڷ��͵Ľڵ�

	NetType											type;				///socket����
	NetStatus										status;				///socket״̬
};

}
}


#endif //__ASIOSOCKET_OBJCET_H__
