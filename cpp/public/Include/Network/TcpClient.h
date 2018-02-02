//
//  Copyright (c)1998-2012,  Public Technology
//  All Rights Reserved.
//
//	Description:
//	$Id: tcpclient.h 216 2015-12-15 11:33:55Z  $
//
#ifndef __NETWORK_TCPCLIENT_H__
#define __NETWORK_TCPCLIENT_H__

#include "Network/Socket.h"

namespace Public{
namespace Network{


class NETWORK_API TCPClient:public Socket
{
	struct TCPClientInternal;
	TCPClient(const TCPClient&);
	TCPClient();
public:
	static Public::Base::shared_ptr<Socket> create(const Public::Base::shared_ptr<AsyncIOWorker>& worker);
	virtual ~TCPClient();

	///断开socket连接，停止socket内部工作，关闭socket句柄等
	///UDP/TCP都可使用该接口释放资源，关闭socket
	virtual bool disconnect();

	///绑定串口信息
	///param[in]		addr		需要绑定的端口
	///param[in]		reusedAddr	端口是否允许需要重复绑定
	///return		true 成功、false 失败 
	///注：不不建议使用该函数来判断串口是否被占用，端口判断推进使用host::checkPortIsNotUsed接口
	virtual bool bind(const NetAddr& addr,bool reusedAddr = true);

	///设置socket发送接受超时时间
	///param[in]		recvTimeout		接收超时 单位：毫秒,-1不设置
	///param[in]		sendTimeout		发送超时 单位：毫秒,-1不设置
	///retun		 true 成功、false 失败 
	virtual bool setSocketTimeout(uint32_t recvTimeout = -1, uint32_t sendTimeout = -1);

	///获取socket发送接受超时时间
	///param[in]		recvTimeout		接收超时 单位：毫秒
	///param[in]		sendTimeout		发送超时 单位：毫秒
	///retun		 true 成功、false 失败 
	///注：异步IO不支持
	virtual bool getSocketTimeout(uint32_t& recvTimeout, uint32_t& sendTimeout) const;


	///获取socket缓冲区大小
	///param[in]		readSize		读的缓冲区大小
	///param[in]		sendSize		发送缓冲区大小
	///retun		 true 成功、false 失败 
	virtual bool getSocketBuffer(uint32_t& recvSize,uint32_t& sendSize) const;

	///设置socket缓冲区大小
	///param[in]		readSize		读的缓冲区大小
	///param[in]		sendSize		发送缓冲区大小
	///retun		 true 成功、false 失败 
	virtual bool setSocketBuffer(uint32_t recvSize = -1,uint32_t sendSize = -1);

	///【异步】启动TCP连接
	///param[in]		addr			第三方的地址
	///param[in]		callback		connect成功后的回调，不能为空
	///retun		 true 成功、false 失败  返回值为异步投递消息结果
	///注：
	/// 1:只有tcpclient才支持
	virtual bool async_connect(const NetAddr& addr,const ConnectedCallback& callback);

	///【同步】TCP连接
	///param[in]		addr			第三方的地址
	///retun		 true 成功、false 失败  返回值为连接的结果
	///注：
	/// 1:只有tcpclient才支持
	virtual bool connect(const NetAddr& addr);

	///设置TCP断开回调通知
	///param[in]		disconnected	断开回调通知
	///retun		 true 成功、false 失败 
	///注：
	/// 1:只有TCP才支持
	virtual bool setDisconnectCallback(const DisconnectedCallback& disconnected);

	///【异步】投递TCP接收数据事件
	///param[in]		buf				接收到的数据存储地址
	///param[in]		len				接收到的数据存储空间最大值
	///param[in]		received		成功接受到数据后的回调，不能为NULL
	///retun		 true 成功、false 失败 ，返回投递消息结果
	///注：
	///	1:只有连接成功后的TCP才支持
	virtual bool async_recv(char *buf , uint32_t len,const ReceivedCallback& received);

	///【同步】TCP接收
	///param[in]		buf				接收到的数据存储地址
	///param[in]		len				接收到的数据存储空间最大值
	///retun		 返回直接接受到消息的长度
	///注：
	///	1:只有连接成功后的TCP才支持
	/// 2:该接口的调用时间跟setSocketTimeout/nonBlocking两个函数决定
	virtual int recv(char *buf , uint32_t len);

	///【异步】投递TCP数据发送事件
	///param[in]		buf				发送数据缓冲地址，该地址空间内容发送过程中不能被修改删除，直到sended调用后才能操作
	///param[in]		len				发送数据数据最大值
	///param[in]		sended			数据发送成后的异步通知，不能为NULL
	///retun		 true 成功、false 失败 ，返回投递消息结果
	///注：
	///  1:只有连接成功后的TCP才支持
	virtual bool async_send(const char * buf, uint32_t len,const SendedCallback& sended);

	///【同步】TCP发送
	///param[in]		buf				发送数据缓冲地址
	///param[in]		len				发送数据数据最大值
	///retun		返回数据直接发送的长度
	///注：
	///  1:只有连接成功后的TCP才支持
	///  2:该接口的调用时间跟setSocketTimeout/nonBlocking两个函数决定
	virtual int send(const char * buf, uint32_t len);
	
	///获取Socket句柄
	///return 句柄	、当socket创建失败 -1
	virtual int getHandle() const;

	///获取Socket网络类型
	///param in		
	///return 网络类型
	virtual NetType getNetType() const;

	///获取Socket自身的地址
	///param in		
	///return 自身bind的地址、未bind为空
	virtual NetAddr getMyAddr() const;

	///获取Socket对方的地址
	///param in		
	///return TCPConnection使用
	virtual NetAddr getOhterAddr() const;
private:
	TCPClientInternal* internal;
};


};
};




#endif //__NETWORK_SOCKET_H__

