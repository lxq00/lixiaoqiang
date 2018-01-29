#ifndef __NETWORK_UDP_H__
#define __NETWORK_UDP_H__

//
//  Copyright (c)1998-2012,  Public Technology
//  All Rights Reserved.
//
//	Description:
//	$Id: udp.h 216 2015-12-15 11:33:55Z  $
//
#include "Network/Socket.h"

using namespace std;
namespace Public{
namespace Network{
	
class NETWORK_API UDP:public Socket
{
	struct UDPInternalPointer;
	UDP(const UDP&);
	UDP();
public:	
	static Public::Base::shared_ptr<Socket> create(const  Public::Base::shared_ptr<AsyncIOWorker>& worker);
	virtual ~UDP();
	
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

	///【异步】投递UDP数据接收事件 
	///param[in]		buf				接收到的数据存储地址
	///param[in]		len				接收到的数据存储空间最大值
	///param[in]		received		成功接受到数据后的回调，不能为NULL
	///retun		 true 成功、false 失败 、返回投递异步消息命令结果
	///注：
	///	1:只有UDP，并且Bind后才支持/
	///	2:received不能为空
	virtual bool async_recvfrom(char *buf , uint32_t len,const RecvFromCallback& received);


	///【同步】UDP接收
	///param[in]		buf				接收到的数据存储地址
	///param[in]		len				接收到的数据存储空间最大值
	///retun		返回实际接收数据的长度
	///注：
	///	1:只有UDP，并且Bind后才支持/
	///	2:received不能为空
	/// 3:该接口的调用时间跟setSocketTimeout/nonBlocking两个函数决定
	virtual int recvfrom(char *buf , uint32_t len ,NetAddr& other);

	///【异步】投递UDP数据发送事件
	///param[in]		buf				发送数据缓冲地址，该地址空间内容发送过程中不能被修改删除，直到sended调用后才能操作
	///param[in]		len				发送数据数据最大值
	///param[in]		other			数据发送的目的
	///param[in]		sended			数据发送成后的异步通知，不能为NULL
	///retun		返回投递发送数据命令结果
	///注：
	///  1:只有UDP才支持
	virtual bool async_sendto(const char * buf, uint32_t len,const NetAddr& other,const SendedCallback& sended);

	///【同步】UDP发送
	///param[in]		buf				发送数据缓冲地址
	///param[in]		len				发送数据数据最大值
	///param[in]		other			数据发送的目的
	///retun		返回实际发送数据的长度
	///注：
	/// 1:只有UDP才支持
	///	2:该接口的调用时间跟setSocketTimeout/nonBlocking两个函数决定
	virtual int sendto(const char * buf, uint32_t len,const NetAddr& other);


	///获取Socket句柄
	///return 句柄	、当socket创建失败 -1
	virtual int getHandle() const ;

	///获取Socket网络类型
	///param in		
	///return 网络类型
	virtual NetType getNetType() const;

	///获取Socket自身的地址
	///param in		
	///return 自身bind的地址、未bind为空
	virtual NetAddr getMyAddr() const;
private:
	UDPInternalPointer* udpinternal;
};


};
};




#endif //__NETWORK_SOCKET_H__
