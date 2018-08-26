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

std::string toErrorString(int errCode)
{
	char buffer[128] = { 0 };
#ifdef WIN32
	{
		LPVOID lpMsgBuf;
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, errCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&lpMsgBuf, 0, NULL);
		snprintf_x(buffer, 127, "´íÎóÂë:%d:%s", errCode, lpMsgBuf == NULL ? "error" : lpMsgBuf);

		if (lpMsgBuf != NULL) {
			LocalFree(lpMsgBuf);
		}
	}
#else
	{
		char * mesg = strerror(errCode);
		snprintf_x(buffer, 127, "´íÎóÂë:%d:%s", errCode, mesg == NULL ? "error" : mesg);
	}
#endif

	return buffer;
}

struct OperationResult::OperationResultInternal
{
	bool result;
	std::string errmsg;
};
OperationResult::OperationResult(const OperationResult& result)
{
	internal = new OperationResultInternal;
	internal->result = result.internal->result;
	internal->errmsg = result.internal->errmsg;
}
OperationResult::OperationResult()
{
	internal = new OperationResultInternal;
	internal->result = true;
	internal->result = "success";
}
OperationResult::OperationResult(const std::string& _errmsg)
{
	internal = new OperationResultInternal;
	internal->result = false;
	internal->errmsg = _errmsg;
}
OperationResult::OperationResult(const char* _errmsg)
{
	internal = new OperationResultInternal;
	internal->result = false;
	internal->errmsg = _errmsg;
}
OperationResult::OperationResult(int errCode)
{
	internal = new OperationResultInternal;
	internal->result = false;
	internal->errmsg = toErrorString(errCode);
}
OperationResult::OperationResult(bool result)
{
	internal = new OperationResultInternal;
	internal->result = result;
	if (internal->result)
	{
		internal->result = "success";
	}
	else
	{
#ifdef WIN32
		internal->errmsg = toErrorString(GetLastError());
#else
		internal->errmsg = toErrorString(errno);
#endif
	}
}
OperationResult::~OperationResult()
{
	SAFE_DELETE(internal);
}

OperationResult& OperationResult::operator=(const OperationResult& result)
{
	internal->result = result.internal->result;
	internal->errmsg = result.internal->errmsg;

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

bool OperationResult::result() const
{
	return internal->result;
}
std::string OperationResult::errorMessage() const
{
	return internal->errmsg;
}

}
}