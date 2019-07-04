#ifndef __VGSII_URL_H__
#define __VGSII_URL_H__
#include "Base/Defs.h"
#include "Base/IntTypes.h"
#include "Base/Value.h"

namespace Public{
namespace Base{


class BASE_API URL
{
	//protocol 协议 如：HTTP/FTP/VDR等
public:
	struct BASE_API AuthenInfo
	{
		std::string Username;
		std::string Password;
	};
public:
	URL();
	URL(const std::string& href);
	URL(const URL& url);
	virtual ~URL();

	URL& operator = (const URL& url);
	URL& operator = (const std::string& href);

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
	uint32_t getPort(uint32_t defaultport = 80) const;
	void setPort(uint32_t portnum);

	///p/a/t/h?query=string#hash
	std::string getPath() const;
	void setPath(const std::string& pathstr);

	//p/a/t/h
	std::string pathname;
	const std::string& getPathName() const;
	void setPathName(const std::string& pathnamestr);
	std::vector<Value> getPathNameArray() const;

	//?query=string#hash
	std::string getSearch() const;
	void setSearch(const std::string& searchstr);

	//<query,string#assh>
	std::map<std::string, Value> query;
	const std::map<std::string, Value>& getQuery() const;
	void setQuery(const std::map<std::string, Value>& queryobj);
};


}
}

#endif //__VGSII_URL_H__
