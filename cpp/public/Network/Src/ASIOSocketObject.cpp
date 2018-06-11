#include "ASIOSocketObject.h"
#include "SocketTcp.h"

namespace Public{
namespace Network{

ISocketObject::ISocketObject(Socket* _sock):sock(_sock),status(NetStatus_notconnected)
{
}

ISocketObject::~ISocketObject(){}

bool ISocketObject::destory()
{
	{
		Guard locker(mutex);

		mustQuit = true;
	}
	///现在关闭应用，做相应的关闭回调处理
	waitAllOtherCallbackThreadUsedEnd();

	return true;
}

	///启动接收
bool ISocketObject::postReceive(boost::shared_ptr<RecvInternal>& internal)
{
	{
		Guard locker(mutex);
		if(internal == NULL ||  mustQuit || currRecvInternal != NULL || status != NetStatus_connected)
		{
			return false;
		}
		currRecvInternal = internal;
		_callbackThreadUsedStart();
	}

	bool ret = doPostRecv(currRecvInternal);

	callbackThreadUsedEnd();

	if(!ret)
	{
		///发送失败，告诉该节点 失败了
		_recvCallbackPtr(boost::system::error_code(),-1);

		return false;
	}

	return true;
}

	///启动发送
bool ISocketObject::beginSend(boost::shared_ptr<SendInternal>& internal)
{
	{
		Guard locker(mutex);
		if(internal == NULL ||  mustQuit || status != NetStatus_connected)
		{
			return false;
		}

		sendInternalList.push_back(internal);

		_callbackThreadUsedStart();
	}

	doStartSendData();

	callbackThreadUsedEnd();

	return true;
}
void ISocketObject::_recvCallbackPtr(const boost::system::error_code& er, std::size_t length)
{
	boost::shared_ptr<RecvInternal> needDoRecvInternal;
	{
		Guard locker(mutex);
		if(mustQuit || status != NetStatus_connected)///如果socket已经被关闭，停止
		{
			return;
		}
		///获取当前出发接收的节点
		needDoRecvInternal = currRecvInternal;
		currRecvInternal = boost::shared_ptr<RecvInternal>();

		///记录当前回调的处理信息
		_callbackThreadUsedStart();
	}
	if(er && type != NetType_Udp)
	{
		///接收失败，tcp都是断开连接，udp也不会进来，断开了
		socketDisconnect(er.message().c_str());
	}
	else if(!er && needDoRecvInternal != NULL)
	{
		needDoRecvInternal->setRecvResult(length);
	}	
	
	///清除当前回调的记录信息
	callbackThreadUsedEnd();
}
void ISocketObject::_sendCallbackPtr(const boost::system::error_code& er, std::size_t length)
{
	boost::shared_ptr<SendInternal> needDoSendInternal;
	{
		Guard locker(mutex);
		if(mustQuit || status != NetStatus_connected)///如果socket已经被关闭，停止
		{
			return;
		}
		///获取当前出发接收的节点
		needDoSendInternal = currSendInternal;

		///记录当前回调的处理信息
		_callbackThreadUsedStart();
	}
	bool sendover = false;
	if(needDoSendInternal != NULL)
	{
		sendover = needDoSendInternal->setSendResultAndCheckIsOver(!er ? length : 0);
	}	

	{
		///清除当前回调的记录信息
		Guard locker(mutex);
		
		if(sendover && sendInternalList.size() > 0)
		{
			boost::shared_ptr<SendInternal> internal = sendInternalList.front();
			if(needDoSendInternal == internal)
			{
				sendInternalList.pop_front();
			}
		}

		currSendInternal = boost::shared_ptr<SendInternal>();
	}

	doStartSendData();

	callbackThreadUsedEnd();
}
void ISocketObject::doStartSendData()
{
	boost::shared_ptr<SendInternal> needSendData;

	{
		Guard locker(mutex);
		if(mustQuit || status != NetStatus_connected)
		{
			return;
		}
		if(currSendInternal != NULL)
		{
			///当前有人发送数据
			return;
		}
		if(sendInternalList.size() <= 0)
		{
			///当前没数据可以发送
			return;
		}
		needSendData = sendInternalList.front();
		currSendInternal = needSendData;
	}
	
	bool ret = doStartSend(needSendData);
	if(!ret)
	{
		///如果数据启动发送失败,清除当前数据
		_sendCallbackPtr(boost::system::error_code(),-1);
	}
}


bool ISocketObject::setSocketTimeout(uint32_t recvTimeout,uint32_t sendTimeout)
{
	if(getHandle() == -1)
	{
		return false;
	}

	int ret = setsockopt(getHandle(),SOL_SOCKET,SO_SNDTIMEO,(const char*)&sendTimeout,sizeof(sendTimeout));
	ret |= setsockopt(getHandle(),SOL_SOCKET,SO_RCVTIMEO,(const char*)&recvTimeout,sizeof(recvTimeout));

	return ret >= 0;
}
bool ISocketObject::getSocketTimeout(uint32_t& recvTimeout,uint32_t& sendTimeout)
{
	if(getHandle() == -1)
	{
		return false;
	}

	socklen_t sendlen = sizeof(sendTimeout),recvlen = sizeof(recvTimeout);

	int ret = getsockopt(getHandle(),SOL_SOCKET,SO_SNDTIMEO,(char*)&sendTimeout,(socklen_t*)&sendlen);
	ret |= getsockopt(getHandle(),SOL_SOCKET,SO_RCVTIMEO,(char*)&recvTimeout,(socklen_t*)&recvlen);

	return ret == 0;
}


}
}

