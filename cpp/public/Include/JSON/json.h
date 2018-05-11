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
	JSONValue();
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

	std::string asString() const;
	int asInt() const;
	float asFloat() const;
	long long asInt64() const;
	bool asBool() const;
	uint32_t asUint32() const;
	uint64_t asUint64() const;

	/// Number of values in array or object
	uint32_t size() const;

	/// \brief Return true if empty array, empty object, or null;
	/// otherwise, false.
	bool empty() const;

	/// Remove all object members and array elements.
	/// \pre type() is arrayValue, objectValue, or nullValue
	/// \post type() is unchanged
	void clear();

	/// Access an object value by name, create a null member if it does not exist.
	JSONValue operator[](const std::string &key);
	
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
