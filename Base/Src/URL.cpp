#include "Base/URL.h"
#include "Base/URLEncoding.h"
#include "Base/String.h"
#include <sstream>

namespace Public{
namespace Base{

//http://user:pass@host.com:8080/p/a/t/h?query=string#hash
URL::URL()
{
	clean();
}
URL::URL(const URL& URL)
{
	href(URL.href());
}
URL::URL(const std::string& URLstr)
{
	href(URLstr);
}
URL::~URL()
{
}

URL& URL::operator = (const URL& URL)
{
	hostname = URL.hostname;
	protocol = URL.protocol;
	pathname = URL.pathname;
	authen = URL.authen;
	port = URL.port;
	query = URL.query;

	return *this;
}
URL& URL::operator = (const std::string& _href)
{
	href(_href);

	return *this;
}

void URL::clean()
{
	protocol = "http";
	authen.Username = authen.Password = "";
	pathname = "/";
	port = 0;
	query.clear();
}

std::string URL::href() const
{
	stringstream sstream;

	std::string hoststr = getHost();

	if (hoststr != "")
	{
		if (protocol == "") sstream << "http";
		else sstream << protocol;

		sstream << "://";

		std::string authenstr = getAuhen();
		if (authenstr != "") sstream << authenstr;

		sstream << hoststr;
	}

	std::string pathstr = getPath();
	if (pathstr != "")
	{
		if (*pathstr.c_str() != '/') sstream << '/';
		sstream << pathstr;
	}

	return sstream.str();
}
void URL::href(const std::string& URL)
{
	clean();

	const char* URLtmp = URL.c_str();
	//解析协议
	const char* tmp = strstr(URLtmp, "://");
	if (tmp != NULL)
	{
		setProtocol(std::string(URLtmp, tmp - URLtmp));
		URLtmp = tmp += 3;
	}
	//解析用户密码
	tmp = strchr(URLtmp, '@');
	if (tmp != NULL)
	{
		setAuthen(std::string(URLtmp, tmp - URLtmp));
		URLtmp = tmp + 1;
	}
	//解析主机信息
	if (*URLtmp != '/')
	{
		tmp = strchr(URLtmp, '/');
		if (tmp == NULL) tmp = URLtmp + strlen(URLtmp);

		setHost(std::string(URLtmp, tmp - URLtmp));

		URLtmp = tmp;
	}

	setPath(URLtmp);

	if (pathname == "") pathname = "/";
}

const std::string& URL::getProtocol() const
{
	return protocol;
}
void URL::setProtocol(const std::string& protocolstr)
{
	protocol = protocolstr;
}

std::string URL::getAuhen() const
{
	if (authen.Username == "" || authen.Password == "") return "";
	if (authen.Password == "") return URLEncoding::encode(authen.Username);
	return URLEncoding::encode(authen.Username) + ":" + URLEncoding::encode(authen.Password);
}
const URL::AuthenInfo& URL::getAuthenInfo() const
{
	return authen;
}
void URL::setAuthen(const std::string& authenstr)
{
	const char* authenstrtmp = authenstr.c_str();
	const char* tmp1 = strchr(authenstrtmp, ':');
	if (tmp1 != NULL)
	{
		authen.Username = URLEncoding::decode(std::string(authenstrtmp, tmp1 - authenstrtmp));
		authen.Password = URLEncoding::decode(tmp1 + 1);
	}
	else
	{
		authen.Username = URLEncoding::decode(authenstr);
	}
}
void URL::setAuthen(const std::string& username, std::string& password)
{
	authen.Username = username;
	authen.Password = password;
}
void URL::setAuthen(const AuthenInfo& info)
{
	authen = info;
}
std::string URL::getHost() const
{
	stringstream sstream;
	sstream << hostname;
	if (port != 0) sstream << ":" << port;

	return sstream.str();
}
void URL::setHost(const std::string& hoststr)
{
	std::string hostnamestr = hoststr;
	const char* hostnametmp = hostnamestr.c_str();
	const char* tmp1 = strchr(hostnametmp, ':');
	if (tmp1 != NULL)
	{
		hostname = std::string(hostnametmp, tmp1 - hostnametmp);
		port = atoi(tmp1 + 1);
	}
	else
	{
		hostname = hoststr;
	}
}

const std::string& URL::getHostname() const
{
	return hostname;
}
void URL::setHostname(const std::string& hostnamestr)
{
	hostname = hostnamestr;
}

uint32_t URL::getPort(uint32_t defaultport) const
{
	return port == 0 ? defaultport : port;
}
void URL::setPort(uint32_t portnum)
{
	port = portnum;
}

std::string URL::getPath() const
{
	std::string querystr = getSearch();

	return pathname + querystr;
}
void URL::setPath(const std::string& pathstr)
{
	const char* URLtmp = pathstr.c_str();
	const char* tmp = strchr(URLtmp, '?');
	if (tmp != NULL)
	{
		pathname = std::string(URLtmp, tmp - URLtmp);

		setSearch(tmp + 1);
	}
	else
	{
		pathname = pathstr;
	}
}

const std::string& URL::getPathName() const
{
	return pathname;
}
void URL::setPathName(const std::string& pathnamestr)
{
	pathname = pathnamestr;
}

std::vector<Value> URL::getPathNameArray() const
{
	std::vector<std::string> tmps = String::split(pathname,"/");
	
	std::vector<Value> tmpval;
	for(uint32_t i = 0;i < tmps.size();i ++)
	{
		tmpval.push_back(Value(URLEncoding::decode(tmps[i])));
	}

	return tmpval;
}
std::string URL::getSearch() const
{
	stringstream sstream;
	
	for (std::map<std::string, Value>::const_iterator iter = query.begin(); iter != query.end(); iter++)
	{
		sstream << (iter == query.begin() ? "?":"&") << URLEncoding::encode(iter->first) << "=" << URLEncoding::encode(iter->second.readString());
	}

	return sstream.str();
}
void URL::setSearch(const std::string& searchstr)
{
	const char* querystr = searchstr.c_str();

	while (1)
	{
		const char* tmp1 = strchr(querystr, '&');
		std::string querystrtmp = querystr;
		if (tmp1 != NULL) querystrtmp = std::string(querystr, tmp1 - querystr);

		std::string key = querystrtmp;
		std::string val;
		const char* querystrtmpstr = querystrtmp.c_str();
		const char* tmp2 = strchr(querystrtmpstr, '=');
		if (tmp2 != NULL)
		{
			key = std::string(querystrtmpstr, tmp2 - querystrtmpstr);
			val = tmp2 + 1;
		}

		query[URLEncoding::decode(key)] = URLEncoding::decode(val);

		if (tmp1 == NULL) break;
		querystr = tmp1 + 1;
	}
}

const std::map<std::string, Value>& URL::getQuery() const
{
	return query;
}
void URL::setQuery(const std::map<std::string, Value>& queryobj)
{
	query = queryobj;
}


}
}


