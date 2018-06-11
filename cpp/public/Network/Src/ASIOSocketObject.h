#ifndef __ASIOSOCKET_OBJCET_H__
#define __ASIOSOCKET_OBJCET_H__
#include "ASIOServerPool.h"
#include "Network/Socket.h"

#ifdef WIN32
typedef int  socklen_t;
#endif

namespace Public{
namespace Network{

///发送基类指针、普通的消息发送结构体
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
	///设置发送的结果，并检测是否已经发送结束
	virtual bool setSendResultAndCheckIsOver(int sendlen)
	{
		sendCallback(sock,buffer,sendlen);
		
		return true;
	}
	///获取需要发送的数据指针
	virtual const char* getSendBuffer() {return buffer;}
	///获取需要发送的数据长度
	virtual uint32_t getSendBufferLen() {return bufferLen;}
	
	const NetAddr& getSendAddr() {return sendpoint;}
protected:	
	const char* 					buffer;
	uint32_t 						bufferLen;
	NetAddr							sendpoint;
	Socket::SendedCallback			sendCallback;
	Socket*							sock;
};

///可以多次发送的发送指针
class RepeatSendInternal:public SendInternal
{
public:
	RepeatSendInternal(Socket* _sock,const char* addr,uint32_t size,const Socket::SendedCallback& sended,bool _newbuffer = false)
	:SendInternal(_sock,addr,size,sended,_newbuffer){sendBuffer = buffer;sendBufferLen = bufferLen;}
	virtual ~RepeatSendInternal(){}
	///设置发送的结果，并检测是否已经发送结束
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
	///获取需要发送的数据指针
	virtual const char* getSendBuffer() {return sendBuffer;}
	///获取需要发送的数据长度
	virtual uint32_t getSendBufferLen() {return sendBufferLen;}
private:
	const char*						sendBuffer;
	uint32_t 						sendBufferLen;
};



///数据接收基类指针
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

///TCP的数据接收信息
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

///UDP的数据接收信息
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
	void waitAllOtherCallbackThreadUsedEnd()////等待所有回调线程使用结束
	{
		uint32_t currThreadId = Thread::getCurrentThreadID();
		while(1)
		{
			{
				Guard locker(mutex);
				if(usedCallbackThreadNum == 0 || (usedCallbackThreadNum == 1 && usedCallbackThreadId == currThreadId))
				{
					///如果没有线程使用，或者是自己线程关闭自己，可以关闭
					break;
				}
			}
			Thread::sleep(10);
		}
	}
	bool _callbackThreadUsedStart()///记录当前回调线程使用信息
	{
		uint32_t currThreadId = Thread::getCurrentThreadID();
		usedCallbackThreadId += currThreadId;
		++usedCallbackThreadNum;

		return true;
	}
	bool callbackThreadUsedEnd() ///当前回调线程使用结束
	{
		Guard locker(mutex);
		
		uint32_t currThreadId = Thread::getCurrentThreadID();
		usedCallbackThreadId -= currThreadId;
		--usedCallbackThreadNum;

		return true;
	}
protected:
	Mutex											mutex;
	bool											mustQuit;				///是不是已经被关闭
	uint64_t										usedCallbackThreadId;	//正在使用回调的线程ID和
	Base::AtomicCount								usedCallbackThreadNum;	//正在使用回调的线程数
};

///asio socket操作的对象，该对象基类
class ISocketObject:public UserThreadInfo
{
public:
	ISocketObject(Socket* sock);
	~ISocketObject();

	virtual int getHandle() = 0;

	void setSocketPtr(Socket* _sock) {sock = _sock;}

	virtual void socketDisconnect(const char* error){}

	///断开socket
	///true表示可以正常关闭该对象，false表示需要另外线程来释放该对象
	virtual bool destory(); 

	///启动接收
	bool postReceive(boost::shared_ptr<RecvInternal>& internal);

	///启动发送
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
	
	boost::shared_ptr<RecvInternal>					currRecvInternal;	///当前正在接收的节点
	std::list<boost::shared_ptr<SendInternal> >		sendInternalList;	///需要发送的节点列表
	boost::shared_ptr<SendInternal>					currSendInternal;	///当前正在发送的节点

	NetType											type;				///socket类型
	NetStatus										status;				///socket状态
};

}
}


#endif //__ASIOSOCKET_OBJCET_H__
