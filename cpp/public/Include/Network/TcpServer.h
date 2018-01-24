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

	///断开socket连接，停止socket内部工作，关闭socket句柄等
	///UDP/TCP都可使用该接口释放资源，关闭socket
	virtual bool disconnect();

	///绑定串口信息
	///param[in]		addr		需要绑定的端口
	///param[in]		reusedAddr	端口是否允许需要重复绑定
	///return		true 成功、false 失败 
	///注：不不建议使用该函数来判断串口是否被占用，端口判断推进使用host::checkPortIsNotUsed接口
	virtual bool bind(const NetAddr& addr,bool reusedAddr = true);

	///获取socket发送接受超时时间
	///param[in]		recvTimeout		接收超时 单位：毫秒
	///param[in]		sendTimeout		发送超时 单位：毫秒
	///retun		 true 成功、false 失败 
	virtual bool getSocketTimeout(uint32_t& recvTimeout,uint32_t& sendTimeout) const;

	///设置socket发送接受超时时间
	///param[in]		recvTimeout		接收超时 单位：毫秒
	///param[in]		sendTimeout		发送超时 单位：毫秒
	///retun		 true 成功、false 失败 
	virtual bool setSocketTimeout(uint32_t recvTimeout,uint32_t sendTimeout);


	///设置socket堵塞、非堵塞模式
	///param[in]		nonblock		true 堵塞模式  false 非赌赛模式
	///return		true 成功、false 失败 
	virtual bool nonBlocking(bool nonblock);


	///【异步】启动监听服务
	///param[in]		callback		accept产生的socket通知回调，不能为NULL
	///retun		 true 成功、false 失败 
	///注：
	/// 1:只有tcpserver才支持
	///	2:异步accept，accept产生的结果通过callback返回
	/// 3:与accept函数不能同时使用
	virtual bool async_accept(const AcceptedCallback& callback);


	///【同步】同步accept产生socket
	///param[in]		无
	///return		返回新产生的socket对象，当NULL时表示失败，返回值需要外部释放资源
	///注：
	/// 1:只有tcpserver才支持
	///	2:与startListen不能同时使用
	///	3:该接口的调用时间跟setSocketTimeout/nonBlocking两个函数决定
	virtual Socket* accept();

	///获取Socket句柄
	///return 句柄	、当socket创建失败 -1
	virtual int getHandle() const ;

	///获取Socket连接状态
	///param in		
	///return 连接状态、TCPServer默认NetStatus_notconnected、UDP默认NetStatus_connected
	virtual NetStatus getStatus() const;

	///获取Socket网络类型
	///param in		
	///return 网络类型
	virtual NetType getNetType() const;

	///获取Socket自身的地址
	///param in		
	///return 自身bind的地址、未bind为空
	virtual NetAddr getMyAddr() const;
private:
	TCPServerInternalPointer* tcpserverinternal;
};


};
};




#endif //__NETWORK_TCPSERVER_H__

