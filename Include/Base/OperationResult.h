#pragma once
#include "Base/IntTypes.h"
#include "Base/Defs.h"

namespace Public {
namespace Base {

class BASE_API OperationResult
{
public:
	OperationResult(const OperationResult& result);
	OperationResult();
	//错误描述
	OperationResult(const std::string& _errmsg);
	OperationResult(const char* _errmsg);
	//系统错误码
	OperationResult(int errorno);
	//如果result == false，将会从getlasterror获取
	OperationResult(bool result);


	~OperationResult();

	OperationResult& operator=(const OperationResult& result);
	operator bool() const;
	bool operator !() const;
	operator std::string() const;

	bool result() const;
	std::string errorMessage() const;
private:
	struct OperationResultInternal;
	OperationResultInternal* internal;
};

}
}