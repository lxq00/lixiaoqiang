#pragma once

#include "Base/Base.h"
#include "Network/Network.h"
#include "redisvalue.h"
#include <stack>
using namespace Public::Base;
using namespace Public::Network;

#define crlf "\r\n"

class RedisParser
{
public:
	RedisParser() : bulkSize(0)
	{
		buf.reserve(64);
	}
	~RedisParser() {}

	bool input(const char *ptr, size_t size)
	{
		size_t position = 0;
		State state = Start;

		if (!states.empty())
		{
			state = states.top();
			states.pop();
		}

		while (position < size)
		{
			char c = ptr[position++];
			switch (state)
			{
			case StartArray:
			case Start:
				buf.clear();
				switch (c)
				{
				case stringReply:
					state = String;
					break;
				case errorReply:
					state = ErrorString;
					break;
				case integerReply:
					state = Integer;
					break;
				case bulkReply:
					state = BulkSize;
					bulkSize = 0;
					break;
				case arrayReply:
					state = ArraySize;
					break;
				default:
					logdebug("RedisParser::input error");
					return false;
				}
				break;
			case String:
				if (c == '\r')
				{
					state = StringLF;
				}
				else if (isChar(c) && !isControl(c))
				{
					buf.push_back(c);
				}
				else
				{
					logdebug("RedisParser::input error");
					return false;
				}
				break;
			case ErrorString:
				if (c == '\r')
				{
					state = ErrorLF;
				}
				else if (isChar(c) && !isControl(c))
				{
					buf.push_back(c);
				}
				else
				{
					logdebug("RedisParser::input error");
					return false;
				}
				break;
			case BulkSize:
				if (c == '\r')
				{
					if (buf.empty())
					{
						logdebug("RedisParser::input error");
						return false;
					}
					else
					{
						state = BulkSizeLF;
					}
				}
				else if (isdigit(c) || c == '-')
				{
					buf.push_back(c);
				}
				else
				{
					logdebug("RedisParser::input error");
					return false;
				}
				break;
			case StringLF:
				if (c == '\n')
				{
					state = Start;
					redisValue = RedisValue(buf);
				}
				else
				{
					logdebug("RedisParser::input error");
					return false;
				}
				break;
			case ErrorLF:
				if (c == '\n')
				{
					state = Start;
					redisValue = RedisValue(false,buf);
				}
				else
				{
					logdebug("RedisParser::input error");
					return false;
				}
				break;
			case BulkSizeLF:
				if (c == '\n')
				{
					bulkSize = bufToLong(buf.data(), buf.size());
					buf.clear();

					if (bulkSize == -1)
					{
						state = Start;
						redisValue = RedisValue(); // Nil
					}
					else if (bulkSize == 0)
					{
						state = BulkCR;
					}
					else if (bulkSize < 0)
					{
						logdebug("RedisParser::input error");
						return false;
					}
					else
					{
						buf.reserve(bulkSize);

						long int available = size - position;
						long int canRead = min(bulkSize, available);

						if (canRead > 0)
						{
							buf.assign(ptr + position, ptr + position + canRead);
							position += canRead;
							bulkSize -= canRead;
						}


						if (bulkSize > 0)
						{
							state = Bulk;
						}
						else
						{
							state = BulkCR;
						}
					}
				}
				else
				{
					logdebug("RedisParser::input error");
					return false;
				}
				break;
			case Bulk: {
				//assert( bulkSize > 0 );

				long int available = size - position + 1;
				long int canRead = min(available, bulkSize);

				buf.insert(buf.end(), ptr + position - 1, ptr + position - 1 + canRead);
				bulkSize -= canRead;
				position += canRead - 1;

				if (bulkSize == 0)
				{
					state = BulkCR;
				}
				break;
			}
			case BulkCR:
				if (c == '\r')
				{
					state = BulkLF;
				}
				else
				{
					logdebug("RedisParser::input error");
					return false;
				}
				break;
			case BulkLF:
				if (c == '\n')
				{
					state = Start;
					redisValue = RedisValue(buf);
				}
				else
				{
					logdebug("RedisParser::input error");
					return false;
				}
				break;
			case ArraySize:
				if (c == '\r')
				{
					if (buf.empty())
					{
						logdebug("RedisParser::input error");
						return false;
					}
					else
					{
						state = ArraySizeLF;
					}
				}
				else if (isdigit(c) || c == '-')
				{
					buf.push_back(c);
				}
				else
				{
					logdebug("RedisParser::input error");
					return false;
				}
				break;
			case ArraySizeLF:
				if (c == '\n')
				{
					int64_t arraySize = bufToLong(buf.data(), buf.size());
					std::vector<RedisValue> array;

					if (arraySize == -1)
					{
						state = Start;
						redisValue = RedisValue();  // Nil value
					}
					else if (arraySize == 0)
					{
						state = Start;
						redisValue = RedisValue(std::move(array));  // Empty array
					}
					else if (arraySize < 0)
					{
						logdebug("RedisParser::input error");
						return false;
					}
					else
					{
						array.reserve((int)arraySize);
						arraySizes.push((long)arraySize);
						arrayValues.push(std::move(array));

						state = StartArray;
					}
				}
				else
				{
					logdebug("RedisParser::input error");
					return false;
				}
				break;
			case Integer:
				if (c == '\r')
				{
					if (buf.empty())
					{
						logdebug("RedisParser::input error");
						return false;
					}
					else
					{
						state = IntegerLF;
					}
				}
				else if (isdigit(c) || c == '-')
				{
					buf.push_back(c);
				}
				else
				{
					logdebug("RedisParser::input error");
					return false;
				}
				break;
			case IntegerLF:
				if (c == '\n')
				{
					int64_t value = bufToLong(buf.data(), buf.size());

					buf.clear();
					redisValue = RedisValue(value);
					state = Start;
				}
				else
				{
					logdebug("RedisParser::input error");
					return false;
				}
				break;
			default:
				logdebug("RedisParser::input error");
				return false;
			}


			if (state == Start)
			{
				if (!arraySizes.empty())
				{
					//assert(arraySizes.size() > 0);
					arrayValues.top().getArray().push_back(redisValue);

					while (!arraySizes.empty() && --arraySizes.top() == 0)
					{
						arraySizes.pop();
						redisValue = std::move(arrayValues.top());
						arrayValues.pop();

						if (!arraySizes.empty())
							arrayValues.top().getArray().push_back(redisValue);
					}
				}


				if (arraySizes.empty())
				{
					valueList.push_back(std::move(redisValue));
				}
			}
		}

		if (arraySizes.empty() && state == Start)
		{
		}
		else
		{
			states.push(state);
		}

		return true;
	}


	shared_ptr<RedisValue> result()
	{
		if (valueList.size() <= 0) return shared_ptr<RedisValue>();

		shared_ptr<RedisValue> val = make_shared<RedisValue>(std::move(valueList.front()));
		valueList.pop_front();

		return val;
	}
protected:
	inline bool isChar(int c)
	{
		return c >= 0 && c <= 127;
	}

	inline bool isControl(int c)
	{
		return (c >= 0 && c <= 31) || (c == 127);
	}

	long int bufToLong(const char *str, size_t size)
	{
		long int value = 0;
		bool sign = false;

		if (str == nullptr || size == 0)
		{
			return 0;
		}

		if (*str == '-')
		{
			sign = true;
			++str;
			--size;

			if (size == 0) {
				return 0;
			}
		}

		for (const char *end = str + size; str != end; ++str)
		{
			char c = *str;

			// char must be valid, already checked in the parser
			//assert(c >= '0' && c <= '9');

			value = value * 10;
			value += c - '0';
		}

		return sign ? -value : value;
	}

private:
	enum State {
		Start = 0,
		StartArray = 1,

		String = 2,
		StringLF = 3,

		ErrorString = 4,
		ErrorLF = 5,

		Integer = 6,
		IntegerLF = 7,

		BulkSize = 8,
		BulkSizeLF = 9,
		Bulk = 10,
		BulkCR = 11,
		BulkLF = 12,

		ArraySize = 13,
		ArraySizeLF = 14,
	};

	std::stack<State> states;

	long int bulkSize;
	std::string buf;

	RedisValue redisValue;

	// temporary variables
	std::stack<long int> arraySizes;
	std::stack<RedisValue> arrayValues;

	static const char stringReply = '+';
	static const char errorReply = '-';
	static const char integerReply = ':';
	static const char bulkReply = '$';
	static const char arrayReply = '*';

	std::list<RedisValue> valueList;
};

class RedisBuilder
{
public:
	typedef Function1<void, const std::string&> BuildCallback;
public:
	static void build(const RedisValue &items,BuildCallback& callback)
	{
		if (items.isArray()) buildArray(items,callback);
		else if (items.isString()) buildString(items, callback);
		else if (items.isInt()) buildInt(items, callback);
		else if (items.isStatus()) buildStatus(items, callback);
	}
private:
	static void buildArray(const RedisValue &items,BuildCallback& callback)
	{
		const std::vector<RedisValue>& array = items.getArray();

		{
			std::string buffer;
			buffer = "*" + Value(array.size()).readString() + crlf;

			callback(buffer);
		}
		
		for (uint32_t i = 0; i < array.size(); i++) 
		{
			RedisBuilder().build(array[i], callback);
		}
	}
	static void buildString(const RedisValue &items, BuildCallback& callback)
	{
		std::string buffer;
		const RedisString& stringtmp = items.getString();
		buffer = "$" + Value(stringtmp.length()).readString() + crlf;
		buffer += (std::string)stringtmp;
		buffer += crlf;

		callback(buffer);
	}
	static void buildInt(const RedisValue &items, BuildCallback& callback)
	{
		std::string buffer;
		buffer = ":" + Value(items.toInt()).readString() + crlf;

		callback(buffer);
	}
	static void buildStatus(const RedisValue &items, BuildCallback& callback)
	{
		std::string buffer;

		const RedisValue::StatusTag& tag = items.getStatus();
		if (!tag.success)
		{
			buffer = "-ERR " + tag.errmsg + crlf;
		}
		else
		{
			buffer = std::string("+") + (tag.errmsg == ""  ? "OK":tag.errmsg) + crlf;
		}

		callback(buffer);
	}
};