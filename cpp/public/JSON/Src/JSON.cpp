#include "JSON/JSON.h"
#include "jsoncpp/json.h"
#include <fstream>

using namespace Public::Base;
namespace Public {
namespace JSON {

struct JSONValue::JSONInternal
{
	Json::Value value;

	URI::Value converToURIValue()
	{
		if (value.isString()) return URI::Value(value.asString());
		else if (value.isInt()) return URI::Value(value.asInt());
		else if (value.isBool()) return URI::Value(value.asBool());
		else if (value.isDouble()) return URI::Value(value.asDouble());
		else if(value.isUInt()) return URI::Value(value.asUInt());
		else return URI::Value();
	}
};
JSONValue::JSONValue(const std::string& val, Type Type)
{
	internal = new JSONInternal;
	if (Type == Type_Int) internal->value = Json::Value(URI::Value(val).readInt());
	else if(Type == Type_Double) internal->value = Json::Value(URI::Value(val).readFloat());
	else if (Type == Type_Bool) internal->value = Json::Value(URI::Value(val).readBool());
	else if (Type == Type_String) internal->value = Json::Value(val);
}
JSONValue::JSONValue()
{
	internal = new JSONInternal;
}
JSONValue::JSONValue(const std::string& val)
{
	internal = new JSONInternal;
	internal->value = Json::Value(val);
}
JSONValue::JSONValue(char val)
{
	internal = new JSONInternal;
	internal->value = Json::Value(URI::Value(val).readString());
}
JSONValue::JSONValue(const char* val)
{
	internal = new JSONInternal;
	if(val != NULL) internal->value = Json::Value(val);
}
JSONValue::JSONValue(const unsigned char* val)
{
	internal = new JSONInternal;
	if (val != NULL) internal->value = Json::Value(val);
}
JSONValue::JSONValue(int val)
{
	internal = new JSONInternal;
	if (val != NULL) internal->value = Json::Value(val);
}
JSONValue::JSONValue(double val)
{
	internal = new JSONInternal;
	if (val != NULL) internal->value = Json::Value(val);
}
JSONValue::JSONValue(bool val)
{
	internal = new JSONInternal;
	if (val != NULL) internal->value = Json::Value(val);
}
JSONValue::JSONValue(long long val)
{
	internal = new JSONInternal;
	internal->value = Json::Value(URI::Value(val).readString());
}
JSONValue::JSONValue(uint32_t val)
{
	internal = new JSONInternal;
	if (val != NULL) internal->value = Json::Value(val);
}
JSONValue::JSONValue(uint64_t val)
{
	internal = new JSONInternal;
	internal->value = Json::Value(URI::Value(val).readString());
}
JSONValue::JSONValue(const JSONValue& val)
{
	internal = new JSONInternal;
	internal->value = val.internal->value;
}
JSONValue::~JSONValue()
{
	delete internal;
}

JSONValue& JSONValue::operator = (const JSONValue& val)
{
	internal->value = val.internal->value;

	return *this;
}


bool JSONValue::isBool() const { return internal->value.isBool(); }
bool JSONValue::isInt() const { return internal->value.isInt(); }
bool JSONValue::isDouble() const { return internal->value.isDouble(); }
bool JSONValue::isString() const { return internal->value.isString(); }
bool JSONValue::isArray() const { return internal->value.isArray(); }
bool JSONValue::isObject() const { return internal->value.isObject(); }

JSONValue::Type JSONValue::type() const
{
	if (internal->value.empty()) return Type_Empty;
	else if (internal->value.isString()) return Type_String;
	else if (internal->value.isInt() || internal->value.isUInt()) return Type_Int;
	else if (internal->value.isBool()) return Type_Bool;
	else if (internal->value.isDouble()) return Type_Double;
	else if (internal->value.isArray()) return Type_Array;
	else return Type_Object;
}

std::string JSONValue::asString() const { return internal->converToURIValue().readString(); }
int JSONValue::asInt() const { return internal->converToURIValue().readInt(); }
float JSONValue::asFloat() const { return internal->converToURIValue().readFloat(); }
long long JSONValue::asInt64() const { return internal->converToURIValue().readInt64(); }
bool JSONValue::asBool() const { return internal->converToURIValue().readBool(); }
uint32_t JSONValue::asUint32() const { return internal->converToURIValue().readUint32(); }
uint64_t JSONValue::asUint64() const {	return internal->converToURIValue().readUint64();}


uint32_t JSONValue::size() const { return internal->value.size(); }

bool JSONValue::empty() const { return internal->value.empty(); }

void JSONValue::clear() { internal->value.clear(); }

JSONValue JSONValue::operator[](const std::string &key) const
{
	JSONValue value;
	value.internal->value = internal->value[key];

	return value;
}

std::string JSONValue::toStyledString() const { return internal->value.toStyledString(); }
bool JSONValue::parseFromFile(const std::string& filename)
{
	std::ifstream t(filename);

	Json::Reader reader;
	bool ret = reader.parse(t, internal->value);
	t.close();

	return ret;
}
bool JSONValue::parse(const std::string& buffer)
{
	Json::Reader reader;
	return reader.parse(buffer, internal->value);
}

std::vector<std::string> JSONValue::Members() const { return internal->value.getMemberNames(); }

} // namespace Base
} // namespace Public

