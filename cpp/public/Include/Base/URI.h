#ifndef __VGSII_URI_H__
#define __VGSII_URI_H__
#include "Base/Defs.h"
#include "Base/IntTypes.h"
#include <string>
#include <list>
using namespace std;
namespace Public{
namespace Base{


class BASE_API URI
{
	struct URIInternal;
public:
	class BASE_API Value
	{
	public:
		typedef enum 
		{
			Type_Empty,
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
		Value(long long val);
		Value(uint32_t val);
		Value(uint64_t val);
		Value(const Value& val);
		~Value();

		Value& operator = (const Value& val);

		std::string readString(const std::string& fmt = "") const;
		int readInt(const std::string& fmt = "") const;
		float readFloat(const std::string& fmt = "") const;
		long long readInt64(const std::string& fmt = "") const;
		bool readBool() const;
		uint32_t readUint32(const std::string& fmt = "") const;
		uint64_t readUint64(const std::string& fmt = "") const;
		Type type() const;

		bool isEmpty() const;
	private:
		struct ValueInternal;
		ValueInternal * internal;
	};
	struct BASE_API URIObject
	{
		std::string	key;
		Value		val;
		//适用于Identifier
		std::string toStringAsIdentifier();
		//适用于ExtParameter和Parameter
		std::string toStringAsParameter();
	};
public:
	//protocol 协议 如：HTTP/FTP/VDR等
	URI(const std::string& protocol,const Value& host,int port = 0);
	URI(const URI& uri);
	~URI();

	bool setProtocol(const std::string& protocol);
	
	bool setHost(const std::string& host,int port = 0);


	//添加地址标识符 val不能为空
	//VDR://local/dvsid/610001/Storages/C?camera=camera/1&date=2012-3-5
	//dvsid/storages
	bool addIdentifier(const std::string& mark,const Value& val);

	//添加参数
	//VDR://local/dvsid/610001/Storages/C?camera=camera/1&date=2012-3-5
	//camera /date
	bool addParameter(const std::string& key,const Value& val);
		

	//删除一个地址标识符及对应的值
	bool removeIndentifier(const std::string& mark);
	//删除一个参数及对应的值
	bool removeParameter(const std::string& key);

	//获取参数字符串
	std::string getParmeterString() const;
	//已字符的方式添加附加参数
	bool addParmeterString(const std::string& exstring);

	//获取一个地址标识符  NULL表示失败
	Value* getIdentifier(const std::string& mark) const;
	//获取一个参数 NULL表示失败
	Value* getParameter(const std::string& key) const;


	//获取地址标识符集合
	std::list<URIObject> getIndentifierList() const;
	//获取参数集合
	std::list<URIObject> getParmeterList() const;

	
	//获取协议
	std::string& getProtocol() const;
	//获取协议
	Value getHost() const;

	//获取端口		0表示无
	int getPort() const;
	
	//转成uri字符串
	std::string toString() const;

	///获取最基本的字符串，不包括参数及扩展参数
	std::string getBasicString() const;

	//协议uri字符串
	bool parse(const std::string& uri);

	URI& operator = (const URI& uri);
private:
	URIInternal* internal;
};


}
}

#endif //__VGSII_URI_H__
