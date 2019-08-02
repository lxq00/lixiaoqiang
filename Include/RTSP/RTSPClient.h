//
//  Copyright (c)1998-2012, Public Technology
//  All Rights Reserved.
//
//	Description:
//	$Id: File.h 252 2013-12-18 04:40:28Z  $
//


#pragma once
#include "Defs.h"
#include "Base/Base.h"
#include "RTSPStructs.h"
#include "RTSPUrl.h"
#include "Network/Network.h"
#include "RTSPBuffer.h"
using namespace Public::Base;
using namespace Public::Network;

namespace Public {
namespace RTSP {

class RTSPClientManager;
class RTSPClientHandler;

class RTSP_API RTSPClient
{
	friend class RTSPClientManager;
	struct RTSPClientInternal;
	RTSPClient(const std::shared_ptr<IOWorker>& work, const shared_ptr<RTSPClientHandler>& handler, const AllockUdpPortCallback& allockport,const RTSPUrl& rtspUrl,const std::string& useragent);
public:
	~RTSPClient();

	/*设置RTP数据接收方式 0:TCP，1:UDP  默认UDP*/
	bool initRTPOverTcpType();
	bool initRTPOverUdpType();

	/*准备数据接收，包括启动数据接收线程，心跳线程*/
	//timeout 连接超时时间，
	//reconnect 是否启用重连
	bool start(uint32_t timeout = 10000);

	bool stop();

	//异步命令，使用RTSPClientHandler->onPlayResponse接收结果
	shared_ptr<RTSPCommandInfo> sendPlayRequest(const RANGE_INFO& range);
	//同步命令，同步返回
	bool sendPlayRequest(const RANGE_INFO& range, uint32_t timeout);

	//异步命令，使用RTSPClientHandler->onPauseResponse接收结果
	shared_ptr<RTSPCommandInfo> sendPauseRequest();
	//同步命令,同步返回
	bool sendPauseRequest(uint32_t timeout);


	//异步命令，使用RTSPClientHandler->onGetparameterResponse接收结果
	shared_ptr<RTSPCommandInfo> sendGetparameterRequest(const std::string& body);
	//同步命令,同步返回
	bool sendGetparameterRequest(const std::string& body, uint32_t timeout);


	//异步命令，使用RTSPClientHandler->onTeradownResponse接收结果
	shared_ptr<RTSPCommandInfo> sendTeradownRequest();
	//同步命令,同步返回
	bool sendTeradownRequest(uint32_t timeout);

	bool sendRTPPackage(bool isvideo, uint32_t timestmap, const char*  buffer, uint32_t bufferlen, bool mark);
private:
	RTSPClientInternal *internal;
};

class RTSP_API RTSPClientHandler
{
public:
	RTSPClientHandler() {}
	virtual ~RTSPClientHandler() {}

	virtual void onConnectResponse(bool success, const std::string& errmsg) {}

	virtual void onDescribeResponse(const shared_ptr<RTSPCommandInfo>& cmdinfo, const MEDIA_INFO& info) {}
	virtual void onSetupResponse(const shared_ptr<RTSPCommandInfo>& cmdinfo, const MEDIA_INFO& info,const TRANSPORT_INFO& transport) {}
	virtual void onPlayResponse(const shared_ptr<RTSPCommandInfo>& cmdinfo) {}
	virtual void onPauseResponse(const shared_ptr<RTSPCommandInfo>& cmdinfo) {}
	virtual void onGetparameterResponse(const shared_ptr<RTSPCommandInfo>& cmdinfo, const std::string& content) {}
	virtual void onTeradownResponse(const shared_ptr<RTSPCommandInfo>& cmdinfo) {}

	virtual void onErrorResponse(const shared_ptr<RTSPCommandInfo>& cmdinfo,int statuscode,const std::string& errmsg) {}

	virtual void onClose(const std::string& errmsg) = 0;
	virtual void onRTPPackageCallback(bool isvideo, uint32_t timestmap, const char*  buffer, uint32_t bufferlen, bool mark) = 0;
};

class RTSP_API RTSPClientManager
{
	struct RTSPClientManagerInternal;
public:
	//userContent 用户描述信息,threadNum 线程数，根据RTSP的用户量决定
	RTSPClientManager(const shared_ptr<IOWorker>& iowrker,const std::string& useragent);
	~RTSPClientManager();

	//rtp 模式地址
	bool initRTPOverUdpType(uint32_t startport = 40000, uint32_t stopport = 41000);

	//创建一个对象
	shared_ptr<RTSPClient> create(const shared_ptr<RTSPClientHandler>& handler, const RTSPUrl& pRtspUrl);
private:
	RTSPClientManagerInternal * internal;
};


}
}
