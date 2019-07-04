#pragma once

#include "Base/Base.h"
#include "Network/Network.h"
#include <boost/variant.hpp>
#include "redisstring.h"
using namespace Public::Base;
using namespace Public::Network;


class RedisValue {
public:
	struct StatusTag {
		StatusTag(){}
		StatusTag(bool succ,const std::string& msg):success(succ),errmsg(msg){}
		
		bool success;
		std::string errmsg;

		inline bool operator == (const StatusTag &t) const {
			return success == t.success && errmsg == t.errmsg;
		}
	};
	RedisValue(): value(NullTag()){}
	RedisValue(RedisValue &&other): value(std::move(other.value)){}
	RedisValue(long long i): value(i){}
	RedisValue(const StatusTag& err):value(err){}
	RedisValue(bool success,const std::string& errmsg):value(StatusTag(success,errmsg)){}
	RedisValue(const char *s,int len) : value(RedisString(std::string(s,len))) {}
	RedisValue(const std::string &s): value(RedisString(s)){}
	RedisValue(const RedisString &s):value(s){}
	RedisValue(const std::vector<RedisValue>& array): value(std::move(array)){}

	RedisValue(const RedisValue &) = default;
	RedisValue& operator = (const RedisValue &) = default;
	RedisValue& operator = (RedisValue &&) = default;

	// Return the value as a std::string if
	// type is a byte string; otherwise returns an empty std::string.
	RedisString toString() const
	{
		if (isInt()) return Value(toInt()).readString();

		static const RedisString emptstr;
		if (!isString()) return emptstr;

		return getString();
	}

	// Return the value as a std::vector<RedisValue> if
	// type is an int; otherwise returns 0.
	int64_t toInt() const
	{
		if (isString()) return Value(toString()).readInt64();

		return castTo<int64_t>();
	}

	const StatusTag& toStatus()const
	{
		static const StatusTag emtptystatus;

		if (!isStatus()) return emtptystatus;

		return getStatus();
	}

	// Return the value as an array if type is an array;
	// otherwise returns an empty array.
	const std::vector<RedisValue>& toArray() const
	{
		static const std::vector<RedisValue> emtpyarray;

		if (!isArray()) return emtpyarray;

		return getArray();
	}

	// Return true if value not a error
     bool isStatus() const
	 {
		 return typeEq<StatusTag>();
	 }

	// Return true if this is a null.
	bool isNull() const
	{
		return typeEq<NullTag>();
	}
	// Return true if type is an int
	bool isInt() const
	{
		return typeEq<int64_t>();
	}
	// Return true if type is an array
	bool isArray() const
	{
		return typeEq< std::vector<RedisValue> >();
	}
	// Return true if type is a string/byte array. Alias for isByteArray().
	bool isString() const
	{
		return typeEq<RedisString>();
	}

	bool operator == (const RedisValue &rhs) const
	{
		return value == rhs.value;
	}
	bool operator != (const RedisValue &rhs) const
	{
		return !(value == rhs.value);
	}

	std::vector<RedisValue> &getArray()
	{
		assert(isArray());
		return boost::get<std::vector<RedisValue>>(value);
	}

	const std::vector<RedisValue> &getArray() const
	{
		assert(isArray());
		return boost::get<std::vector<RedisValue>>(value);
	}

	StatusTag& getStatus()
	{
		assert(isStatus());

		return boost::get<StatusTag>(value);
	}

	const StatusTag& getStatus() const
	{
		assert(isStatus());

		return boost::get<StatusTag>(value);
	}
	RedisString& getString()
	{
		assert(isString());
		return boost::get<RedisString>(value);
	}
	const RedisString& getString()const
	{
		assert(isString());

		return boost::get<RedisString>(value);
	}
protected:
	template<typename T>
	T castTo() const
	{
		if (value.type() == typeid(T))
			return boost::get<T>(value);
		else
			return T();
	}

	template<typename T>
	bool typeEq() const
	{
		if (value.type() == typeid(T))
			return true;
		else
			return false;
	}

private:
	struct NullTag {
		inline bool operator == (const NullTag &) const {
			return true;
		}
	};


	boost::variant<NullTag, StatusTag, int64_t, RedisString, std::vector<RedisValue> > value;
};


typedef Function2<void, void*, const RedisValue&> CmdResultCallback;