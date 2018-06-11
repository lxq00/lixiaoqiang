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
	///���ڹر�Ӧ�ã�����Ӧ�Ĺرջص�����
	waitAllOtherCallbackThreadUsedEnd();

	return true;
}

	///��������
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
		///����ʧ�ܣ����߸ýڵ� ʧ����
		_recvCallbackPtr(boost::system::error_code(),-1);

		return false;
	}

	return true;
}

	///��������
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
		if(mustQuit || status != NetStatus_connected)///���socket�Ѿ����رգ�ֹͣ
		{
			return;
		}
		///��ȡ��ǰ�������յĽڵ�
		needDoRecvInternal = currRecvInternal;
		currRecvInternal = boost::shared_ptr<RecvInternal>();

		///��¼��ǰ�ص��Ĵ�����Ϣ
		_callbackThreadUsedStart();
	}
	if(er && type != NetType_Udp)
	{
		///����ʧ�ܣ�tcp���ǶϿ����ӣ�udpҲ����������Ͽ���
		socketDisconnect(er.message().c_str());
	}
	else if(!er && needDoRecvInternal != NULL)
	{
		needDoRecvInternal->setRecvResult(length);
	}	
	
	///�����ǰ�ص��ļ�¼��Ϣ
	callbackThreadUsedEnd();
}
void ISocketObject::_sendCallbackPtr(const boost::system::error_code& er, std::size_t length)
{
	boost::shared_ptr<SendInternal> needDoSendInternal;
	{
		Guard locker(mutex);
		if(mustQuit || status != NetStatus_connected)///���socket�Ѿ����رգ�ֹͣ
		{
			return;
		}
		///��ȡ��ǰ�������յĽڵ�
		needDoSendInternal = currSendInternal;

		///��¼��ǰ�ص��Ĵ�����Ϣ
		_callbackThreadUsedStart();
	}
	bool sendover = false;
	if(needDoSendInternal != NULL)
	{
		sendover = needDoSendInternal->setSendResultAndCheckIsOver(!er ? length : 0);
	}	

	{
		///�����ǰ�ص��ļ�¼��Ϣ
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
			///��ǰ���˷�������
			return;
		}
		if(sendInternalList.size() <= 0)
		{
			///��ǰû���ݿ��Է���
			return;
		}
		needSendData = sendInternalList.front();
		currSendInternal = needSendData;
	}
	
	bool ret = doStartSend(needSendData);
	if(!ret)
	{
		///���������������ʧ��,�����ǰ����
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

