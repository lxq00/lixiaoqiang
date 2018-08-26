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
	//��������
	OperationResult(const std::string& _errmsg);
	OperationResult(const char* _errmsg);
	//ϵͳ������
	OperationResult(int errorno);
	//���result == false�������getlasterror��ȡ
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