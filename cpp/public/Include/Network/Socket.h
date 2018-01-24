//
//  Copyright (c)1998-2012, Chongqing Xunmei Technology
//  All Rights Reserved.
//
//	Description:
//	$Id: Socket.h 216 2015-12-15 11:33:55Z lixiaoqiang $
//

#ifndef __NETWORK_SOCKET_H__
#define __NETWORK_SOCKET_H__

#include "Base/Base.h"
#include "Network/Defs.h"
#include "Network/NetAddr.h"

using namespace Xunmei::Base;

namespace Xunmei{
namespace Network{

#define INVALIDHANDLE -1

///socket工作需要的worker集合，主要在这定义当前socket工作需要的线程数信息
class NETWORK_API IOWorker
{
public:
	class NETWORK_API ThreadNum
	{
	public:
		///指明工作线程的的个数 当num == 0 时 使用 num = 1
		ThreadNum(uint32_t num);
		///cpuCorePerThread 为一个cpucore上运行几个线程，总数为 cpuCorePerThread*cpucore
		///minThread 最少的线程数,maxThread最大的线程数
		///当计算出来线程数为0，使用1个线程
		ThreadNum(uint32_t cpuCorePerThread,uint32_t minThread,uint32_t maxThread);
		~ThreadNum();

		uint32_t getNum() const;
	private:
		uint32_t num;
	};
public:
	IOWorker(const ThreadNum& num);
	~IOWorker();

public:
	class IOWorkerInternal;
	IOWorkerInternal* internal;
};


/// Net 连接的状态
enum NetStatus{
	NetStatus_connected = 0, 		//	已经连接上
	NetStatus_notconnected = -1, 	// 	未连接
	NetStatus_error = -2			//  出错
};

enum NetType{
	NetType_TcpServer = 0,			//Accept的TCP
	NetType_TcpClient,				//Connect的TCP
	NetType_TcpConnection,			//连接成功的TCP、Accept产生的TCP连接、或者Connect成功后变成TCP连接
	NetType_Udp,					//udp
};

class NETWORK_API Socket
{
public:
	/// socket监听时有accept事件回调方法，第一个参数表示监听socket自身，第二个表示新构造的socket.需要外部释放
	/// 回调定义参考：void acceptCallbackFunc(Socket* oldSock,Socket* newSock);
	typedef Function2<void, Socket*, Socket*> AcceptedCallback;

	/// socket异步连接事件回调方法,第一个参数表示连接socket自身
	///回调定义参考：void connectCallbackFunc(Socket* connectSock);
	typedef Function1<void, Socket*> ConnectedCallback;

	/// socket异步断开事件回调方法,int 第一个参数表示连接socket自身，第二个表示描述断开错误描述，客户端，服务端均使用该事件
	/// 当socket第一次连接成功后，如果10秒无数据，会主动断开连接，错误，防止 tcp恶意连接，只针对accept产生的socket
	///回调定义参考：void disconnectCallbackFunc(Socket* connectionSock,const char* disconectErrorInfo);
	typedef Function2<void, Socket*,const char*> DisconnectedCallback;

	// socket发生received事件回调方法，第一个参数表示连接socket自身，第二个表示实际接收的socket数据长度.
	///回调定义参考：void recvCallbackFunc(Socket* sock,const char* recvBuffer,int recvlen);
	typedef Function3<void, Socket*,const char*, int> ReceivedCallback;

	// socket发生send事件回调方法，第一个参数表示连接socket自身，第二个表示实际发送的socket数据长度.
	///回调定义参考：void sendCallbackFunc(Socket* sock,const char* sendBuffer,int sendlen);
	typedef Function3<void, Socket*,const char*, int> SendedCallback;
	
	/// socket发送udp的recvfrom事件回调方法，第一个参数表示socket自身，第二三个参数表示接收的数据信息地址和长度，第四个参数表示数据发送方ip
	///回调定义参考：void recvfromCallbackFunc(Socket* sock,const char* recvBuffer, int recvlen ,const NetAddr& otheraddr);
	typedef Function4<void,Socket*,const char*, int,const NetAddr&> RecvFromCallback;
public:
	Socket(){}
	virtual ~Socket(){}

	///断开socket连接，停止socket内部工作，关闭socket句柄等
	///UDP/TCP都可使用该接口释放资源，关闭socket
	virtual bool disconnect(){return false;}

	///绑定串口信息
	///param[in]		addr		需要绑定的端口
	///param[in]		reusedAddr	端口是否允许需要重复绑定
	///return		true 成功、false 失败 
	///注：不不建议使用该函数来判断串口是否被占用，端口判断推进使用host::checkPortIsNotUsed接口
	virtual bool bind(const NetAddr& addr,bool reusedAddr = true){return false;}
	
	///获取socket缓冲区大小
	///param[in]		readSize		读的缓冲区大小
	///param[in]		sendSize		发送缓冲区大小
	///retun		 true 成功、false 失败 
	virtual bool getSocketBuffer(uint32_t& recvSize,uint32_t& sendSize) const{return false;}
	
	///设置socket缓冲区大小
	///param[in]		readSize		读的缓冲区大小
	///param[in]		sendSize		发送缓冲区大小
	///retun		 true 成功、false 失败 
	virtual bool setSocketBuffer(uint32_t recvSize,uint32_t sendSize){return false;}

	///获取socket发送接受超时时间
	///param[in]		recvTimeout		接收超时 单位：毫秒
	///param[in]		sendTimeout		发送超时 单位：毫秒
	///retun		 true 成功、false 失败 
	virtual bool getSocketTimeout(uint32_t& recvTimeout,uint32_t& sendTimeout) const{return false;}

	///设置socket发送接受超时时间
	///param[in]		recvTimeout		接收超时 单位：毫秒
	///param[in]		sendTimeout		发送超时 单位：毫秒
	///retun		 true 成功、false 失败 
	virtual bool setSocketTimeout(uint32_t recvTimeout,uint32_t sendTimeout) {return false;}


	///设置socket堵塞、非堵塞模式
	///param[in]		nonblock		true 堵塞模式  false 非赌赛模式
	///return		true 成功、false 失败 
	virtual bool nonBlocking(bool nonblock){return false;}


	///【异步】启动监听服务
	///param[in]		callback		accept产生的socket通知回调，不能为NULL
	///retun		 true 成功、false 失败 
	///注：
	/// 1:只有tcpserver才支持
	///	2:异步accept，accept产生的结果通过callback返回
	/// 3:与accept函数不能同时使用
	virtual bool async_accept(const AcceptedCallback& callback){return false;}


	///【同步】同步accept产生socket
	///param[in]		无
	///return		返回新产生的socket对象，当NULL时表示失败，返回值需要外部释放资源
	///注：
	/// 1:只有tcpserver才支持
	///	2:与startListen不能同时使用
	///	3:该接口的调用时间跟setSocketTimeout/nonBlocking两个函数决定
	virtual Socket* accept(){return NULL;}


	///【异步】启动TCP连接
	///param[in]		addr			第三方的地址
	///param[in]		callback		connect成功后的回调，不能为空
	///retun		 true 成功、false 失败  返回值为异步投递消息结果
	///注：
	/// 1:只有tcpclient才支持
	virtual bool async_connect(const NetAddr& addr,const ConnectedCallback& callback){return false;}

	///【同步】TCP连接
	///param[in]		addr			第三方的地址
	///retun		 true 成功、false 失败  返回值为连接的结果
	///注：
	/// 1:只有tcpclient才支持
	virtual bool connect(const NetAddr& addr){return false;}

	///设置TCP断开回调通知
	///param[in]		disconnected	断开回调通知
	///retun		 true 成功、false 失败 
	///注：
	/// 1:只有TCP才支持
	virtual bool setDisconnectCallback(const DisconnectedCallback& disconnected){return false;}

	///【异步】投递TCP接收数据事件
	///param[in]		buf				接收到的数据存储地址
	///param[in]		len				接收到的数据存储空间最大值
	///param[in]		received		成功接受到数据后的回调，不能为NULL
	///retun		 true 成功、false 失败 ，返回投递消息结果
	///注：
	///	1:只有连接成功后的TCP才支持
	virtual bool async_recv(char *buf , uint32_t len,const ReceivedCallback& received){return false;}

	///【同步】TCP接收
	///param[in]		buf				接收到的数据存储地址
	///param[in]		len				接收到的数据存储空间最大值
	///retun		 返回直接接受到消息的长度
	///注：
	///	1:只有连接成功后的TCP才支持
	/// 2:该接口的调用时间跟setSocketTimeout/nonBlocking两个函数决定
	virtual int recv(char *buf , uint32_t len){return false;}

	///【异步】投递TCP数据发送事件
	///param[in]		buf				发送数据缓冲地址，该地址空间内容发送过程中不能被修改删除，直到sended调用后才能操作
	///param[in]		len				发送数据数据最大值
	///param[in]		sended			数据发送成后的异步通知，不能为NULL
	///retun		 true 成功、false 失败 ，返回投递消息结果
	///注：
	///  1:只有连接成功后的TCP才支持
	virtual bool async_send(const char * buf, uint32_t len,const SendedCallback& sended){return false;}

	///【同步】TCP发送
	///param[in]		buf				发送数据缓冲地址
	///param[in]		len				发送数据数据最大值
	///retun		返回数据直接发送的长度
	///注：
	///  1:只有连接成功后的TCP才支持
	///  2:该接口的调用时间跟setSocketTimeout/nonBlocking两个函数决定
	virtual int send(const char * buf, uint32_t len){return -1;}


	///【异步】投递UDP数据接收事件 
	///param[in]		buf				接收到的数据存储地址
	///param[in]		len				接收到的数据存储空间最大值
	///param[in]		received		成功接受到数据后的回调，不能为NULL
	///retun		 true 成功、false 失败 、返回投递异步消息命令结果
	///注：
	///	1:只有UDP，并且Bind后才支持/
	///	2:received不能为空
	virtual bool async_recvfrom(char *buf , uint32_t len,const RecvFromCallback& received){return false;}


	///【同步】UDP接收
	///param[in]		buf				接收到的数据存储地址
	///param[in]		len				接收到的数据存储空间最大值
	///param[in]		other			数据发送的目的
	///retun		返回实际接收数据的长度
	///注：
	///	1:只有UDP，并且Bind后才支持/
	///	2:received不能为空
	/// 3:该接口的调用时间跟setSocketTimeout/nonBlocking两个函数决定
	virtual int recvfrom(char *buf , uint32_t len,NetAddr& other){return false;}

	///【异步】投递UDP数据发送事件
	///param[in]		buf				发送数据缓冲地址，该地址空间内容发送过程中不能被修改删除，直到sended调用后才能操作
	///param[in]		len				发送数据数据最大值
	///param[in]		other			数据发送的目的
	///param[in]		sended			数据发送成后的异步通知，不能为NULL
	///retun		返回投递发送数据命令结果
	///注：
	///  1:只有UDP才支持
	virtual bool async_sendto(const char * buf, uint32_t len,const NetAddr& other,const SendedCallback& sended){return false;}

	///【同步】UDP发送
	///param[in]		buf				发送数据缓冲地址
	///param[in]		len				发送数据数据最大值
	///param[in]		other			数据发送的目的
	///retun		返回实际发送数据的长度
	///注：
	/// 1:只有UDP才支持
	///	2:该接口的调用时间跟setSocketTimeout/nonBlocking两个函数决定
	virtual int sendto(const char * buf, uint32_t len,const NetAddr& other){return -1;}

	
	///获取Socket句柄
	///return 句柄	、当socket创建失败 -1
	virtual int getHandle() const {return INVALIDHANDLE;}

	///获取Socket连接状态
	///param in		
	///return 连接状态、TCPServer默认NetStatus_notconnected、UDP默认NetStatus_connected
	virtual NetStatus getStatus() const{return NetStatus_error;}

	///获取Socket网络类型
	///param in		
	///return 网络类型
	virtual NetType getNetType() const{return NetType_Udp;}

	///获取Socket自身的地址
	///param in		
	///return 自身bind的地址、未bind为空
	virtual NetAddr getMyAddr() const{return NetAddr();}
	
	///获取Socket对方的地址
	///param in		
	///return TCPConnection使用
	virtual NetAddr getOhterAddr() const{return NetAddr();}
};

};
};




#endif //__NETWORK_SOCKET_H__

