#ifndef __EXPRESSION_H__EVAL_H__
#define __EXPRESSION_H__EVAL_H__
#include "Base/IntTypes.h"
namespace Public {
namespace Base {

///运算表达式执行库
class BASE_API Expression
{
public:
	///执行表达式，返回值为bool,如 (1>=2 || 2 >= 0)
	static bool evalBool(const std::string& expression);

	//执行表达式，返回值,如 1 + 2 * 3
	static std::string evalVal(const std::string& expression);
};

}
}

#endif //__EXPRESSION_H__EVAL_H__
