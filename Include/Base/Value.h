#pragma once

#include "Base/IntTypes.h"

namespace Public {
namespace Base {

class BASE_API Value
{
public:
	typedef enum
	{
		Type_Empty = 0,
		Type_String,
		Type_Char,
		Type_Int32,
		Type_Double,
		Type_Bool,
		Type_Int64,
	}Type;
public:
	Value();
	Value(const std::string& val, Type Type);
	Value(const std::string& val);
	Value(char val);
	Value(const char* val);
	Value(const unsigned char* val);
	Value(int val);
	Value(double val);
	Value(bool val);
	Value(uint32_t val);
	Value(uint64_t val);
	Value(int64_t val);
	Value(const Value& val);
	~Value();

	Value& operator = (const Value& val);
	bool operator==(const Value& val) const;

	operator std::string() const;
	operator uint64_t() const;
	operator uint32_t() const;
	operator int64_t() const;
	operator int32_t() const;
	operator bool() const;
	operator double() const;
	operator float() const;

	std::string readString(const std::string& fmt = "") const;
	int readInt(const std::string& fmt = "") const;
	float readFloat(const std::string& fmt = "") const;
	long long readInt64(const std::string& fmt = "") const;
	bool readBool() const;
	uint32_t readUint32(const std::string& fmt = "") const;
	uint64_t readUint64(const std::string& fmt = "") const;
	
	Type type() const;

	bool empty() const;
private:
	struct ValueInternal;
	ValueInternal * internal;
};
}
}