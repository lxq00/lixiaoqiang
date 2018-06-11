#ifndef __SIPStackERROR_DEFINE_H__
#define __SIPStackERROR_DEFINE_H__
#include <string>
using namespace std;
namespace Public{
namespace SIPStack{

enum SIPStackErrorCode
{
	SIPStackError_Processing = 100,
	SIPStackError_Ringing = 180,
	SIPStackError_OK	= 200,
	SIPStackError_ServerError = 608,
	SIPStackError_NotRegister = 604,
	SIPStackError_DevNotFind = 404,
	SIPStackError_UserError = 606,
	SIPStackError_PwdError = 600,
	SIPStackError_Timeout   = 481,
	SIPStackError_ProcolError = 400,
	SIPStackError_NoSuport = 440,
	SIPStackError_DecoderError = 485,
	SIPStackError_AllocChannelError = 486,
	SIPStackError_InviteDeviceError = 487,
	SIPStackError_AddSessionError = 488,
	SIPStackError_ClientBye = 489,
	SIPStackError_NoIdleMrs = 490,
	SIPStackError_UnAuthenticate = 401,
	SIPStackError_SvriceUnavailable = 503,
	SIPStackError_AlarmNotGuard = 601,
	SIPStackError_DeviceNotRegister = 602,
};

struct SIPStackError
{
private:

	class SIPStackErrorInfo
	{
	public:
		struct ErrorInfo
		{
			SIPStackErrorCode error;
			const char*  erorrInfo;
		};
		static std::string getErrorInfo(SIPStackErrorCode error)
		{
			static ErrorInfo infos[] =
			{
				{ SIPStackError_Processing	, "Processing" },
				{ SIPStackError_Ringing , "Ringing"	},
				{ SIPStackError_OK			, "No Error!" },
				{ SIPStackError_ServerError	, "Server Error!" },
				{ SIPStackError_NotRegister	, "Have No Registr!" },
				{ SIPStackError_DevNotFind	, "Device Not Find!" },
				{ SIPStackError_UserError		, "User Error!" },
				{ SIPStackError_PwdError		, "Passwd Error!" },
				{ SIPStackError_Timeout		, "Timeout!" },
				{ SIPStackError_ProcolError	, "Protocol Error!" },
				{ SIPStackError_NoSuport		, "Not Suport!" },
				{ SIPStackError_DecoderError	, "Devocder Error!" },
				{ SIPStackError_AllocChannelError,"Alloc Channel Error!" },
				{ SIPStackError_AddSessionError, "Add Session Error!" },
				{ SIPStackError_ClientBye		, "Client Bye!" },
				{ SIPStackError_NoIdleMrs		, "Media Server Error!" },
				{ SIPStackError_UnAuthenticate, "No Aunthenticate!" },
				{ SIPStackError_AlarmNotGuard , "Alarm Not Guard!" },
				{ SIPStackError_SvriceUnavailable, "Service Unavailable" },
				{ SIPStackError_DeviceNotRegister, "Device Not Register!" },
			};

			for (unsigned int i = 0; i < sizeof(infos) / sizeof(ErrorInfo); i++)
			{
				if (infos[i].error == error)
				{
					return infos[i].erorrInfo;
				}
			}

			return "Unkown Error!";
		}
	};
public:
	SIPStackError() 
	{
		errorCode = SIPStackError_OK;
	}
	SIPStackError(SIPStackErrorCode code,const std::string& info = "")
	{
		errorCode = code;
		errorInfo = std::string("<") + info + std::string(">");;
		if (errorCode != SIPStackError_OK && errorInfo == "")
		{
			errorInfo = std::string("<") + SIPStackErrorInfo::getErrorInfo(code) + std::string(">");
		}
	}

	SIPStackErrorCode getErrorCode() const
	{
		return errorCode;
	}
	std::string getErrorInfo() const
	{
		return errorInfo;
	}
private:
	SIPStackErrorCode errorCode;
	std::string errorInfo;
};

};
};

#endif //__SIPStackERROR_H__

