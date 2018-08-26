
#pragma once

#ifndef _RTSP_CLIENT_ERROR_CODE_H_
#define _RTSP_CLIENT_ERROR_CODE_H_

//error code*******************************************************
#define RTSP_ERR_NO_ERROR					(0)   //没有错误
#define RTSP_ERR_UNKOWN						(-1)  //未知错误
#define RTSP_ERR_CONNECT_FAILED				(-2)  //连接服务器失败
#define RTSP_ERR_MAX_TASK					(-3)  //超过最大连接数
#define RTSP_ERR_NOT_CONNECTED				(-4)  //尚未连接服务器
#define RTSP_ERR_INVLID_HANDLE				(-5)  //任务句柄无效
#define RTSP_ERR_INVLID_PARAM				(-6)  //非法参数
#define RTSP_ERR_URL_CHECK_FAILED			(-7)  //URL检查无法通过
#define RTSP_ERR_OPTION_FAILED				(-8)  //OPTION 命令失败
#define RTSP_ERR_DISCRIBE_FAILED			(-9)  //DISCRIBE 命令失败
#define RTSP_ERR_SETUP_FAILED				(-10) //SETUP 命令失败
#define RTSP_ERR_PLAY_FAILED				(-11) //PLAY 命令失败
#define RTSP_ERR_GET_PARAMETER_FAILED		(-12) //GET_PARAMETER 命令错误
#define RTSP_ERR_STARTRECV_FAILED			(-13) //开启数据接收失败
#define RTSP_ERR_RECV_TYPE_ERROR			(-14) //数据接收方式错误
#define RTSP_ERR_USER_INFO_ERROR			(-15) //用户验证数据为空

#define RTSP_ERR_UNINIT						(-100) //SDK未初始化
#define RTSP_ERR_UNSUPPORT					(-101) //目前不支持
#define RTSP_ERR_CREATECLIENT_FAILED		(-102) //创建RTSP客户端实例失败

typedef enum {
	RTSPStatus_CMDError,
	RTSPStatus_Timeout,
	RTSPStatus_DisConnect,
	RTSPStatus_ConnectSuccess, 
	RTSPStatus_Eof
}RTSPStatus_t;


#endif
