#include "Base/Expression.h"
#include "Base/Base.h"
#include "Expression/muParser.h"
namespace Public {
namespace Base {

using namespace Public::Base;

bool Expression::evalBool(const std::string& expression)
{
	mu::Parser parser;
	parser.SetExpr(expression);

	return parser.Eval() == 0 ? false : true;
}

std::string Expression::evalVal(const std::string& expression)
{
	mu::Parser parser;
	parser.SetExpr(expression);

	char value[64];
	snprintf_x(value,63,"%f",parser.Eval());

	return value;
}


}
}
