//
//  Copyright (c)1998-2012, Public Technology
//  All Rights Reserved.
//
//	Description:
//	$Id: MemPool.h 255 2014-01-23 03:28:32Z  $
//
#ifndef __PUBLIC_JSON_H__
#define __PUBLIC_JSON_H__
#include "Base/Base.h"
#include "JSON/Defs.h"
using namespace Public::Base;
namespace Public {
namespace JSON {

class JSON_API JSONValue
{
public:
	typedef enum
	{
		Type_Empty,
		Type_String,
		Type_Int,
		Type_Double,
		Type_Bool,
		Type_Array,
		Type_Object,
	}Type;
public:
	JSONValue();
	JSONValue(const std::string& val,Type Type);
	JSONValue(const std::string& val);
	JSONValue(char val);
	JSONValue(const char* val);
	JSONValue(const unsigned char* val);
	JSONValue(int val);
	JSONValue(double val);
	JSONValue(bool val);
	JSONValue(long long val);
	JSONValue(uint32_t val);
	JSONValue(uint64_t val);
	JSONValue(const JSONValue& val);
	~JSONValue();

	JSONValue& operator = (const JSONValue& val);

	bool isBool() const;
	bool isInt() const;
	bool isDouble() const;
	bool isString() const;
	bool isArray() const;
	bool isObject() const;

	Type type() const;

	std::string asString() const;
	int asInt() const;
	float asFloat() const;
	long long asInt64() const;
	bool asBool() const;
	uint32_t asUint32() const;
	uint64_t asUint64() const;

	uint32_t size() const;

	bool empty() const;

	void clear();

	JSONValue operator[](const std::string &key) const;
	
	std::string toStyledString() const;

	bool parseFromFile(const std::string& filename);
	bool parse(const std::string& buffer);

	std::vector<std::string> Members() const;
private:
	struct JSONInternal;
	JSONInternal *internal;
};

} // namespace Base
} // namespace Public


#endif //__PUBLIC_JSON_H__
