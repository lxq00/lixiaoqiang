#pragma once
#include "Base/IntTypes.h"
#include "Base/Defs.h"

namespace Public {
namespace Base {

enum Operation_ErrorCode
{
	Operation_Fail = -1,
	Operation_Success = 0,
	Operation_Error_UnSupport,					//不支持

	Operation_Error_Password = 100,				//账号密码错误
	Operation_Error_OldPassword,				//旧密码错误
	Operation_Error_Port,						//端口错误
	Operation_Error_Authen,						//认证失败

	Operation_Error_NetworkErr = 200,			//网络错误
	Operation_Error_NetworkTimeOut,				//网络超时

	Operation_Error_ParseParam = 300,			//解析参数错误
	Operation_Error_ParseObject,				//解析对象错误
	Operation_Error_ParseFile,					//解析文件错误
	Operation_Error_ParseUrl,					//解析url错误
	Operation_Error_Request,					//请求错误
	Operation_Error_Param,						//参数错误
	Operation_Error_DBEroor,					//数据库错误
	Operation_Error_SetCfg,						//设置配置错误
	Operation_Error_GetCfg,						//获取配置错误
	Operation_Error_QueryFail,					//查询失败
	Operation_Error_QueryTimeOut,				//查询超时
	Operation_Error_ControlFail,				//控制失败
	Operation_Error_ControlTimeOut,				//控制超时
	Operation_Error_RepeatOPeration,			//重复操作

	Operation_Error_InitFail = 400,				//初始化失败
	Operation_Error_ResourceUninit,				//资源未初始化
	Operation_Error_ResourceCreateFail,			//资源创建失败
	Operation_Error_ResourceNotExist,			//资源不存在
	Operation_Error_ResourceExisted,			//资源已存在
	Operation_Error_DeviceNotExist,				//设备不存在
	Operation_Error_DeviceExisted,				//设备已存在
	Operation_Error_DeviceOffOnline,			//设备离线
	Operation_Error_ChannelNotExist,			//通道不存在
	Operation_Error_ChannelExisted,				//通道已存在
	Operation_Error_ChannedOffOnline,			//通道离线
	Operation_Error_ServiceNotExist,			//服务不存在
	Operation_Error_ServiceExisted,				//服务已存在
	Operation_Error_ServiceOffOnline,			//服务离线
	Operation_Error_FileExisted,				//文件已存在
	Operation_Error_FileNotExist,				//文件不存在
	Operation_Error_EnterpriseExisted,			//企业已存在
	Operation_Error_EnterpriseNotExist,			//企业不存在
	Operation_Error_PointExisted,				//点位已存在
	Operation_Error_PointNotExist,				//点位不存在

	Operation_Error_InvalidRecord = 500,		//无效的录像
	Operation_Error_InvalidService,				//无效的服务
	Operation_Error_InvalidDevice,				//无效的设备

	Operation_Error_Send = 600,					//发送数据失败
	Operation_Error_Recive,						//接收数据失败
	Operation_Error_OpenFile,					//打开文件失败
	Operation_Error_WriteFile,					//写入文件失败
	Operation_Error_ReadFile,					//读取文件失败

	Operation_Error_DiskErr = 700,				//磁盘异常错误
	Operation_Error_DiskFull,					//磁盘满


	Operation_Error_StreamType	= 1000,			//流类型错误
	Operation_Error_StreamCmd,					//播放控制命令错误

};

class BASE_API OperationResult
{
public:
	OperationResult(const OperationResult& result);
	OperationResult();
	//系统错误码
	OperationResult(Operation_ErrorCode errorCode, const std::string& errmsg = "");
	//如果result == false，将会从getlasterror获取

	~OperationResult();

	OperationResult& operator=(const OperationResult& result);
	operator bool() const;
	bool operator !() const;
	operator std::string() const;

	std::string errorMessage() const;
	Operation_ErrorCode errorCode() const;
private:
	struct OperationResultInternal;
	OperationResultInternal* internal;
};

}
}