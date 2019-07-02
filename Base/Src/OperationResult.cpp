#ifdef WIN32
#include <Windows.h>
#else
#include <errno.h> 
#endif

#include "Base/OperationResult.h"
#include "Base/String.h"
#include "Base/BaseTemplate.h"
namespace Public {
namespace Base {

//std::string toErrorString(int errCode)
//{
//	char buffer[128] = { 0 };
//#ifdef WIN32
//	{
//		LPVOID lpMsgBuf;
//		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, errCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&lpMsgBuf, 0, NULL);
//		snprintf_x(buffer, 127, "错误码:%d:%s", errCode, lpMsgBuf == NULL ? "error" : lpMsgBuf);
//
//		if (lpMsgBuf != NULL) {
//			LocalFree(lpMsgBuf);
//		}
//	}
//#else
//	{
//		char * mesg = strerror(errCode);
//		snprintf_x(buffer, 127, "错误码:%d:%s", errCode, mesg == NULL ? "error" : mesg);
//	}
//#endif
//
//	return buffer;
//}
typedef struct {
	Operation_ErrorCode errcode;
	const char *errmsg;
} errorString;

static const errorString faults[] = {
	{Operation_Fail,					"失败"},
	{Operation_Success,					"成功"},
	{Operation_Error_UnSupport,			"不支持"},
	{Operation_Error_Password,			"账号密码错误"},
	{Operation_Error_OldPassword,		"旧密码错误"},
	{Operation_Error_Port,				"端口错误"},
	{Operation_Error_Authen,			"认证失败"},
	{Operation_Error_NetworkErr,		"网络错误"},
	{Operation_Error_NetworkTimeOut,	"网络超时"},
	{Operation_Error_ParseParam,		"解析参数错误"},
	{Operation_Error_ParseObject,		"解析对象错误"},
	{Operation_Error_ParseFile,			"解析文件错误"},
	{Operation_Error_ParseUrl,			"解析url错误"},
	{Operation_Error_Request,			"请求错误"},
	{Operation_Error_Param,				"参数错误"},
	{Operation_Error_DBEroor,			"数据库错误"},
	{Operation_Error_SetCfg,			"设置配置错误"},
	{Operation_Error_GetCfg,			"获取配置错误"},
	{Operation_Error_QueryFail,			"查询失败"},
	{Operation_Error_QueryTimeOut,		"查询超时"},
	{Operation_Error_ControlFail,		"控制失败"},
	{Operation_Error_ControlTimeOut,	"控制超时"},
	{Operation_Error_RepeatOPeration,	"重复操作"},
	{Operation_Error_InitFail,			"初始化失败"},
	{Operation_Error_ResourceUninit,	"资源未初始化"},
	{Operation_Error_ResourceCreateFail,"资源创建失败"},
	{Operation_Error_ResourceNotExist,	"资源不存在"},
	{Operation_Error_ResourceExisted,	"资源已存在"},
	{Operation_Error_DeviceNotExist,	"设备不存在"},
	{Operation_Error_DeviceExisted,		"设备已存在"},
	{Operation_Error_DeviceOffOnline,	"设备离线"},
	{Operation_Error_ChannelNotExist,	"通道不存在"},
	{Operation_Error_ChannelExisted,	"通道已存在"},
	{Operation_Error_ChannedOffOnline,  "通道离线"},
	{Operation_Error_ServiceNotExist,	"服务不存在"},
	{Operation_Error_ServiceExisted,	"服务已存在"},
	{Operation_Error_ServiceOffOnline,  "服务离线"},
	{Operation_Error_FileExisted,		"文件已存在"},
	{Operation_Error_FileNotExist,		"文件不存在"},
	{Operation_Error_EnterpriseExisted, "企业已存在"},
	{Operation_Error_EnterpriseNotExist,"企业不存在"},
	{Operation_Error_PointExisted,		"点位已存在"},
	{Operation_Error_PointNotExist,		"点位不存在"},
	{Operation_Error_InvalidRecord,		"无效的录像"},
	{Operation_Error_InvalidService,	"无效的服务"},
	{Operation_Error_InvalidDevice,		"无效的设备"},
	{Operation_Error_Send,				"发送数据失败"},
	{Operation_Error_Recive,			"接收数据失败"},
	{Operation_Error_OpenFile,			"打开文件失败"},
	{Operation_Error_WriteFile,			"写入文件失败"},
	{Operation_Error_ReadFile,			"读取文件失败"},
	{Operation_Error_DiskErr,			"磁盘异常错误"},
	{Operation_Error_DiskFull,			"磁盘满"},
	{Operation_Error_StreamType,		"流类型错误"},
	{Operation_Error_StreamCmd,			"播放控制命令错误"}

};

std::string toErrorString(Operation_ErrorCode errCode)
{
	for (unsigned int i = 0; i < sizeof(faults) / sizeof(faults[0]); i++)
	{
		if (faults[i].errcode == errCode)
		{
			return faults[i].errmsg;
		}
	}

	return "未知错误";
}

struct OperationResult::OperationResultInternal
{
	bool result;
	std::string errmsg;
	Operation_ErrorCode errCode;
};
OperationResult::OperationResult(const OperationResult& result)
{
	internal = new OperationResultInternal;
	internal->result = result.internal->result;
	internal->errmsg = result.internal->errmsg;
	internal->errCode = result.internal->errCode;
}
OperationResult::OperationResult()
{
	internal = new OperationResultInternal;
	internal->result = true;
	internal->result = "success";
	internal->errCode = Operation_Success;
}

OperationResult::OperationResult(Operation_ErrorCode errCode, const std::string& errmsg)
{
	internal = new OperationResultInternal;
	internal->result = errCode == Operation_Success ? true : false;
	internal->errCode = errCode;
	internal->errmsg = errmsg == "" ? toErrorString(errCode) : errmsg;
}

OperationResult::~OperationResult()
{
	SAFE_DELETE(internal);
}

OperationResult& OperationResult::operator=(const OperationResult& result)
{
	internal->result = result.internal->result;
	internal->errmsg = result.internal->errmsg;
	internal->errCode = result.internal->errCode;

	return *this;
}
OperationResult::operator bool() const
{
	return internal->result;
}
bool OperationResult::operator !() const
{
	return !internal->result;
}
OperationResult::operator std::string() const
{
	return internal->errmsg;
}

std::string OperationResult::errorMessage() const
{
	return internal->errmsg;
}

Operation_ErrorCode OperationResult::errorCode() const
{
	return internal->errCode;
}

}
}