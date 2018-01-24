//
//  Copyright (c)1998-2012, Chongqing Xunmei Technology
//  All Rights Reserved.
//
//	Description:
//	$Id: tcpserver.h 216 2015-12-15 11:33:55Z lixiaoqiang $
//
#ifndef __NETWORK_TCPSERVER_H__
#define __NETWORK_TCPSERVER_H__

#include "Network/Socket.h"

namespace Xunmei{
namespace Network{

class NETWORK_API TCPServer:public Socket
{
	struct TCPServerInternalPointer;
	TCPServer(const TCPServer&);
public:
	TCPServer(IOWorker& worker,const NetAddr& addr = NetAddr());

	virtual ~TCPServer();

	///�Ͽ�socket���ӣ�ֹͣsocket�ڲ��������ر�socket�����
	///UDP/TCP����ʹ�øýӿ��ͷ���Դ���ر�socket
	virtual bool disconnect();

	///�󶨴�����Ϣ
	///param[in]		addr		��Ҫ�󶨵Ķ˿�
	///param[in]		reusedAddr	�˿��Ƿ�������Ҫ�ظ���
	///return		true �ɹ���false ʧ�� 
	///ע����������ʹ�øú������жϴ����Ƿ�ռ�ã��˿��ж��ƽ�ʹ��host::checkPortIsNotUsed�ӿ�
	virtual bool bind(const NetAddr& addr,bool reusedAddr = true);

	///��ȡsocket���ͽ��ܳ�ʱʱ��
	///param[in]		recvTimeout		���ճ�ʱ ��λ������
	///param[in]		sendTimeout		���ͳ�ʱ ��λ������
	///retun		 true �ɹ���false ʧ�� 
	virtual bool getSocketTimeout(uint32_t& recvTimeout,uint32_t& sendTimeout) const;

	///����socket���ͽ��ܳ�ʱʱ��
	///param[in]		recvTimeout		���ճ�ʱ ��λ������
	///param[in]		sendTimeout		���ͳ�ʱ ��λ������
	///retun		 true �ɹ���false ʧ�� 
	virtual bool setSocketTimeout(uint32_t recvTimeout,uint32_t sendTimeout);


	///����socket�������Ƕ���ģʽ
	///param[in]		nonblock		true ����ģʽ  false �Ƕ���ģʽ
	///return		true �ɹ���false ʧ�� 
	virtual bool nonBlocking(bool nonblock);


	///���첽��������������
	///param[in]		callback		accept������socket֪ͨ�ص�������ΪNULL
	///retun		 true �ɹ���false ʧ�� 
	///ע��
	/// 1:ֻ��tcpserver��֧��
	///	2:�첽accept��accept�����Ľ��ͨ��callback����
	/// 3:��accept��������ͬʱʹ��
	virtual bool async_accept(const AcceptedCallback& callback);


	///��ͬ����ͬ��accept����socket
	///param[in]		��
	///return		�����²�����socket���󣬵�NULLʱ��ʾʧ�ܣ�����ֵ��Ҫ�ⲿ�ͷ���Դ
	///ע��
	/// 1:ֻ��tcpserver��֧��
	///	2:��startListen����ͬʱʹ��
	///	3:�ýӿڵĵ���ʱ���setSocketTimeout/nonBlocking������������
	virtual Socket* accept();

	///��ȡSocket���
	///return ���	����socket����ʧ�� -1
	virtual int getHandle() const ;

	///��ȡSocket����״̬
	///param in		
	///return ����״̬��TCPServerĬ��NetStatus_notconnected��UDPĬ��NetStatus_connected
	virtual NetStatus getStatus() const;

	///��ȡSocket��������
	///param in		
	///return ��������
	virtual NetType getNetType() const;

	///��ȡSocket����ĵ�ַ
	///param in		
	///return ����bind�ĵ�ַ��δbindΪ��
	virtual NetAddr getMyAddr() const;
private:
	TCPServerInternalPointer* tcpserverinternal;
};


};
};




#endif //__NETWORK_TCPSERVER_H__

