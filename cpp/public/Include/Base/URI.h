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
public:
	//protocol 协议 如：HTTP/FTP/VDR等
public:
	struct BASE_API AuthenInfo
	{
		std::string Username;
		std::string Password;
	};
public:
	URI();
	URI(const std::string& href);
	URI(const URI& url);
	virtual ~URI();

	URI& operator = (const URI& url);
	URI& operator = (const std::string& href);

	void clean();

	//http://user:pass@host.com:8080/p/a/t/h?query=string#hash
	std::string href() const;
	void href(const std::string& url);

	//http
	std::string protocol;
	const std::string& getProtocol() const;
	void setProtocol(const std::string& protocolstr);

	//user:pass
	AuthenInfo	authen;
	std::string getAuhen() const;
	const AuthenInfo& getAuthenInfo() const;
	void setAuthen(const std::string& authenstr);
	void setAuthen(const std::string& username, std::string& password);
	void setAuthen(const AuthenInfo& info);


	//host.com:8080
	std::string getHost() const;
	void setHost(const std::string& hoststr);

	//host.com
	std::string hostname;
	const std::string& getHostname() const;
	void setHostname(const std::string& hostnamestr);

	//8080
	uint32_t port;
	uint32_t getPort() const;
	void setPort(uint32_t portnum);

	///p/a/t/h?query=string#hash
	std::string getPath() const;
	void setPath(const std::string& pathstr);

	//p/a/t/h
	std::string pathname;
	const std::string& getPathname() const;
	void setPathname(const std::string& pathnamestr);

	//?query=string#hash
	std::string getSearch() const;
	void setSearch(const std::string& searchstr);

	//<query,string#assh>
	std::map<std::string, URI::Value> query;
	const std::map<std::string, URI::Value>& getQuery() const;
	void setQuery(const std::map<std::string, URI::Value>& queryobj);
};


}
}

#endif //__VGSII_URI_H__
