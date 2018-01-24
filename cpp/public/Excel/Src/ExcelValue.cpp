#include "Excel/Excel.h"

namespace Xunmei {
namespace Excel {

struct XM_Excel::Value::ValueInternal
{
	XM_Excel::Value::Type  type;
	shared_ptr<URI::Value>	val;
};

XM_Excel::Value::Value() 
{
	internal = new ValueInternal;
	internal->val = new URI::Value();
}
XM_Excel::Value::Value(const char* val)
{
	internal = new ValueInternal;
	internal->type = Value::Type_String;
	internal->val = new URI::Value(val);
}

XM_Excel::Value::Value(const std::string& val)
{
	internal = new ValueInternal;
	internal->type = Value::Type_String;
	internal->val = new URI::Value(val);
}

XM_Excel::Value::Value(int val)
{
	internal = new ValueInternal;
	internal->type = Value::Type_Number;
	internal->val = new URI::Value(val);
}

XM_Excel::Value::Value(unsigned int val)
{
	internal = new ValueInternal;
	internal->type = Value::Type_Number;
	internal->val = new URI::Value(val);
}

XM_Excel::Value::Value(long long val)
{
	internal = new ValueInternal;
	internal->type = Value::Type_Number;
	internal->val = new URI::Value(val);
}

XM_Excel::Value::Value(unsigned long long val)
{
	internal = new ValueInternal;
	internal->type = Value::Type_Number;
	internal->val = new URI::Value((long long)val);
}

XM_Excel::Value::Value(float val)
{
	internal = new ValueInternal;
	internal->type = Value::Type_Double;
	internal->val = new URI::Value(val);
}

XM_Excel::Value::Value(double val)
{
	internal = new ValueInternal;
	internal->type = Value::Type_Double;
	internal->val = new URI::Value((float)val);
}

XM_Excel::Value::Value(bool val)
{
	internal = new ValueInternal;
	internal->type = Value::Type_Bool;
	internal->val = new URI::Value((bool)val);
}

XM_Excel::Value::Type XM_Excel::Value::type() const
{
	return internal->type;
}

XM_Excel::Value::Value(const Value& val)
{
	internal = new ValueInternal;	
	internal->type = val.internal->type;
	internal->val = new URI::Value(*val.internal->val.get());
}

XM_Excel::Value::~Value() 
{
	SAFE_DELETE(internal);
}

std::string XM_Excel::Value::toString() const
{
	return internal->val != NULL ? internal->val->readString() : "";
}

int XM_Excel::Value::toInt32() const
{
	return internal->val != NULL ? internal->val->readInt() : 0;
}

unsigned int XM_Excel::Value::toUint32() const
{
	return internal->val != NULL ? internal->val->readUint32() : 0;
}

unsigned long long XM_Excel::Value::toUint64() const
{
	return internal->val != NULL ? internal->val->readUint64() : 0;
}

long long XM_Excel::Value::toInt64() const
{
	return internal->val != NULL ? internal->val->readInt64() : 0;
}

float XM_Excel::Value::toFloat() const
{
	return internal->val != NULL ? internal->val->readFloat() : 0;
}

double XM_Excel::Value::toDouble() const
{
	return internal->val != NULL ? internal->val->readFloat() : 0;
}

bool XM_Excel::Value::toBool() const
{
	return internal->val != NULL ? (((internal->val.get()) > 0) ? true : false) : 0;
}

bool XM_Excel::Value::isEmpty() const
{
	return internal->val != NULL ? internal->val->isEmpty() : false;
}

bool XM_Excel::Value::operator == (const Value& val) const
{
	return ((internal == val.internal) || (internal < val.internal || internal > val.internal));
}

XM_Excel::Value& XM_Excel::Value::operator = (const Value& val)
{
	internal = new ValueInternal;
	internal->type = val.internal->type;
	internal->val = new URI::Value(*val.internal->val.get());

	return*this;
}

}
}