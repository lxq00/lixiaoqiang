#include "Base/Value.h"
#include "Base/String.h"
#include <sstream>

namespace Public{
namespace Base{

struct Value::ValueInternal
{
	Type		type;
	union {
		uint64_t	_int;
		double		_float;
		bool		_bool;
		char*		_str;
	}val;

	ValueInternal() :type(Type_Empty) { val._int = 0; }
};
Value::Value()
{
	internal = new ValueInternal();
	internal->type = Type_Empty;
}
Value::Value(const std::string& val, Value::Type type)
{
	internal = new ValueInternal();
	internal->type = type;
	switch (type)
	{
		case Type_Char:
		case Type_Int32:
		case Type_Int64:
			sscanf(val.c_str(), "%llu", &internal->val._int);
			break;		
		case Type_String:
			internal->val._str = new char[val.length() + 1];
			strcpy(internal->val._str, val.c_str());
			break;
		case Type_Double:
			sscanf(val.c_str(), "%lf", &internal->val._float);
			break;
		case Type_Bool:
			internal->val._bool = strcasecmp(val.c_str(), "true") == 0;
			break;
	}
}
Value::Value(const char* val)
{
	internal = new ValueInternal();
	if (val != NULL)
	{
		internal->val._str = new char[strlen(val) + 1];
		strcpy(internal->val._str, val);
		internal->type = Type_String;
	}
}
Value::Value(const std::string& val)
{
	internal = new ValueInternal();
	{
		internal->val._str = new char[val.length() + 1];
		strcpy(internal->val._str, val.c_str());
		internal->type = Type_String;
	}
}

Value::Value(const unsigned char* val)
{
	internal = new ValueInternal();
	if (val != NULL)
	{
		internal->val._str = new char[strlen((const char*)val) + 1];
		strcpy(internal->val._str, (const char*)val);
		internal->type = Type_String;
	}
}
Value::Value(char val)
{
	internal = new ValueInternal();
	internal->type = Type_Char;
	internal->val._int = (uint64_t)val;
}
Value::Value(int val)
{
	internal = new ValueInternal();
	internal->type = Type_Int32;
	internal->val._int = (uint64_t)val;
}
Value::Value(double val)
{
	internal = new ValueInternal();
	internal->type = Type_Double;
	internal->val._float = (double)val;
}
Value::Value(long long val)
{
	internal = new ValueInternal();
	internal->type = Type_Int64;
	internal->val._int = (uint64_t)val;
}
Value::Value(uint32_t val)
{
	internal = new ValueInternal();
	internal->type = Type_Int32;
	internal->val._int = (uint64_t)val;
}
Value::Value(uint64_t val)
{
	internal = new ValueInternal();
	internal->type = Type_Int64;
	internal->val._int = (uint64_t)val;
}
Value::Value(bool val)
{
	internal = new ValueInternal();
	internal->type = Type_Bool;
	internal->val._bool = val;
}

Value::Value(const Value& val)
{
	internal = new ValueInternal();
	internal->type = val.internal->type;
	internal->val = val.internal->val;
	if (internal->type == Type_String)
	{
		internal->val._str = new char[strlen(val.internal->val._str) + 1];
		strcpy(internal->val._str, val.internal->val._str);
	}
}

Value::~Value()
{
	if (internal->type == Type_String)
		delete[]internal->val._str;
}

Value& Value::operator = (const Value& val)
{
	if (internal->type == Type_String)
		delete[]internal->val._str;

	internal->type = val.internal->type;
	internal->val = val.internal->val;
	if (internal->type == Type_String)
	{
		internal->val._str = new char[strlen(val.internal->val._str) + 1];
		strcpy(internal->val._str, val.internal->val._str);
	}

	return *this;
}

Value::Type Value::type() const
{
	return internal->type;
}

Value::operator std::string() const { return readString(); }
Value::operator uint64_t() const { return readUint64(); }
Value::operator uint32_t() const { return readUint32(); }
Value::operator bool() const { return readBool(); }
Value::operator double() const { return readFloat(); }
Value::operator float() const { return readFloat(); }

std::string Value::readString(const std::string& fmt) const
{
	switch (internal->type)
	{
	case Type_Char:
	{
		char buffer[32] = { 0 };
		snprintf(buffer, 31, fmt == "" ? "%c" : fmt.c_str(), (char)internal->val._int);
		return buffer;
	}
	case Type_Int32:
	{
		char buffer[32] = { 0 };
		snprintf(buffer, 31, fmt == "" ? "%d" : fmt.c_str(), (int)internal->val._int);
		return buffer;
	}
	case Type_Int64:
	{
		char buffer[32] = { 0 };
		snprintf(buffer, 31, fmt == "" ? "%lld" : fmt.c_str(), (int64_t)internal->val._int);
		return buffer;
	}
	case Type_String:
		return internal->val._str;
	case Type_Double:
	{
		char buffer[32] = { 0 };
		snprintf(buffer, 31, fmt == "" ? "%lf" : fmt.c_str(), internal->val._float);
		return buffer;
	}
	case Type_Bool:
		return internal->val._bool ? "true" : "false";
	}

	return "";
}
int Value::readInt(const std::string& fmt) const
{
	switch (internal->type)
	{
	case Type_Char:
	case Type_Int32:
	case Type_Int64:
		return (int)internal->val._int;
	case Type_String:
	{
		int val = 0;
		sscanf(internal->val._str, fmt == "" ? "%d" : fmt.c_str(),&val);
		return val;
	}
	case Type_Double:
		return (int)internal->val._float;
	case Type_Bool:
		return internal->val._bool ? 1 : 0;
	}

	return 0;
}
float Value::readFloat(const std::string& fmt) const
{
	switch (internal->type)
	{
	case Type_Char:
	case Type_Int32:
	case Type_Int64:
		return (float)internal->val._int;
	case Type_String:
	{
		float val = 0;
		sscanf(internal->val._str, fmt == "" ? "%f" : fmt.c_str(), &val);
		return val;
	}
	case Type_Double:
		return (float)internal->val._float;
	case Type_Bool:
		return internal->val._bool ? (float)1.0 : (float)0.0;
	}

	return (float)0.0;
}

long long Value::readInt64(const std::string& fmt) const
{
	switch (internal->type)
	{
	case Type_Char:
	case Type_Int32:
	case Type_Int64:
		return (long long)internal->val._int;
	case Type_String:
	{
		long long val = 0;
		sscanf(internal->val._str, fmt == "" ? "%lld" : fmt.c_str(), &val);
		return val;
	}
	case Type_Double:
		return (long long)internal->val._float;
	case Type_Bool:
		return internal->val._bool ? 1 : 0;
	}

	return 0;
}

uint32_t Value::readUint32(const std::string& fmt) const
{
	switch (internal->type)
	{
	case Type_Char:
	case Type_Int32:
	case Type_Int64:
		return (uint32_t)internal->val._int;
	case Type_String:
	{
		uint32_t val = 0;
		sscanf(internal->val._str, fmt == "" ? "%u" : fmt.c_str(), &val);
		return val;
	}
	case Type_Double:
		return (uint32_t)internal->val._float;
	case Type_Bool:
		return internal->val._bool ? 1 : 0;
	}

	return 0;
}

uint64_t Value::readUint64(const std::string& fmt) const
{
	switch (internal->type)
	{
	case Type_Char:
	case Type_Int32:
	case Type_Int64:
		return (uint64_t)internal->val._int;
	case Type_String:
	{
		uint64_t val = 0;
		sscanf(internal->val._str, fmt == "" ? "%llu" : fmt.c_str(), &val);
		return val;
	}
	case Type_Double:
		return (uint64_t)internal->val._float;
	case Type_Bool:
		return internal->val._bool ? 1 : 0;
	}

	return 0;
}
bool Value::readBool() const
{
	switch (internal->type)
	{
	case Type_Char:
	case Type_Int32:
	case Type_Int64:
		return internal->val._int != 0;
	case Type_String:
		return strcasecmp(internal->val._str, "true") == 0;
	case Type_Double:
		return (int)internal->val._float != 0;
	case Type_Bool:
		return internal->val._bool;
	}

	return false;
}
bool Value::empty() const
{
	return internal->type == Type_Empty;
}

}
}


