#ifndef REDISCLIENT_REDISPARSER_H
#define REDISCLIENT_REDISPARSER_H

#include <stack>
#include <vector>
#include <utility>

#include "redisvalue.h"

namespace redisclient {

class RedisParser
{
public:
     RedisParser();
	 ~RedisParser();

	 bool input(const char *ptr, size_t size);

	 shared_ptr<RedisValue> result();
protected:
    inline bool isChar(int c)
    {
        return c >= 0 && c <= 127;
    }

    inline bool isControl(int c)
    {
        return (c >= 0 && c <= 31) || (c == 127);
    }

     long int bufToLong(const char *str, size_t size);

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
	std::vector<char> buf;

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
	static std::string makeCommand(const std::deque<Value> &items);
private:
	static void bufferAppend(std::string &vec, char buf);
	static void bufferAppend(std::string &vec, const char* buf);
	static void bufferAppend(std::string &vec, const std::string &val);
};

}



#endif // REDISCLIENT_REDISPARSER_H
