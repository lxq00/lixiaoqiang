#include "Base/URI.h"
#include "Base/URLEncoding.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sstream>

namespace Public{
namespace Base{
	
#ifdef WIN32
#define strcasecmp _stricmp
#define snprintf _snprintf
#endif
bool checkURIKeyIsValid(const std::string& key)
{
	char invaildList[] = {' ','/','?','@','~','!','#','$','^','*','&','(',')','[',']','{','}',
		'/','\\','\r','\n','\t',';',':','\"',',','<','>','|','='};

	for(unsigned int i = 0;i < sizeof(invaildList)/sizeof(char);i ++)
	{
		const char* tmp = strchr(key.c_str(),invaildList[i]);
		if(tmp != NULL)
		{
			return false;
		}
	}

	return true;
}

const char* findNodeInString(const char* str,std::string& val,const char* breakstr,const char* endstr,bool& end)
{
	if(str == NULL)
	{
		end = true;
		return NULL;
	}
	
	end = false;

	const char* tmp = str;
	while(1)
	{
		bool isbreak = (strchr(breakstr,*tmp) != NULL || strchr(endstr,*tmp) != NULL || *tmp == 0);
		
		if(isbreak)
		{
			val = std::string(str,tmp - str);
			end = (strchr(endstr,*tmp) != NULL || *tmp == 0);

			break;
		}
		tmp ++;
	}

	return *tmp ? tmp + 1 : NULL;
}
struct URI::Value::ValueInternal
{
	Type		type;
	union {
		uint64_t	_int;
		double		_float;
		bool		_bool;
		char*		_str;
	}val;

	ValueInternal() :type(Type_Empty) { val._int = 0; }
};
URI::Value::Value()
{
	internal = new ValueInternal();
	internal->type = Type_Empty;
}
URI::Value::Value(const std::string& val, URI::Value::Type type)
{
	internal = new ValueInternal();
	internal->type = type;
	switch (type)
	{
		case Type_Char:
		case Type_Int32:
		case Type_Int64:
			sscanf(val.c_str(), "%llu", &internal->val._int);
			break;		
		case Type_String:
			internal->val._str = new char[val.length() + 1];
			strcpy(internal->val._str, val.c_str());
			break;
		case Type_Double:
			sscanf(val.c_str(), "%lf", &internal->val._float);
			break;
		case Type_Bool:
			internal->val._bool = strcasecmp(val.c_str(), "true") == 0;
			break;
	}
}
URI::Value::Value(const char* val)
{
	internal = new ValueInternal();
	if (val != NULL)
	{
		internal->val._str = new char[strlen(val) + 1];
		strcpy(internal->val._str, val);
		internal->type = Type_String;
	}
}
URI::Value::Value(const std::string& val)
{
	internal = new ValueInternal();
	{
		internal->val._str = new char[val.length() + 1];
		strcpy(internal->val._str, val.c_str());
		internal->type = Type_String;
	}
}

URI::Value::Value(const unsigned char* val)
{
	internal = new ValueInternal();
	if (val != NULL)
	{
		internal->val._str = new char[strlen((const char*)val) + 1];
		strcpy(internal->val._str, (const char*)val);
		internal->type = Type_String;
	}
}
URI::Value::Value(char val)
{
	internal = new ValueInternal();
	internal->type = Type_Char;
	internal->val._int = (uint64_t)val;
}
URI::Value::Value(int val)
{
	internal = new ValueInternal();
	internal->type = Type_Int32;
	internal->val._int = (uint64_t)val;
}
URI::Value::Value(double val)
{
	internal = new ValueInternal();
	internal->type = Type_Double;
	internal->val._float = (double)val;
}
URI::Value::Value(long long val)
{
	internal = new ValueInternal();
	internal->type = Type_Int64;
	internal->val._int = (uint64_t)val;
}
URI::Value::Value(uint32_t val)
{
	internal = new ValueInternal();
	internal->type = Type_Int32;
	internal->val._int = (uint64_t)val;
}
URI::Value::Value(uint64_t val)
{
	internal = new ValueInternal();
	internal->type = Type_Int64;
	internal->val._int = (uint64_t)val;
}
URI::Value::Value(bool val)
{
	internal = new ValueInternal();
	internal->type = Type_Bool;
	internal->val._bool = val;
}

URI::Value::Value(const Value& val)
{
	internal = new ValueInternal();
	internal->type = val.internal->type;
	internal->val = val.internal->val;
	if (internal->type == Type_String)
	{
		internal->val._str = new char[strlen(val.internal->val._str) + 1];
		strcpy(internal->val._str, val.internal->val._str);
	}
}

URI::Value::~Value()
{
	if (internal->type == Type_String)
		delete[]internal->val._str;
}

URI::Value& URI::Value::operator = (const Value& val)
{
	if (internal->type == Type_String)
		delete[]internal->val._str;

	internal->type = val.internal->type;
	internal->val = val.internal->val;
	if (internal->type == Type_String)
	{
		internal->val._str = new char[strlen(val.internal->val._str) + 1];
		strcpy(internal->val._str, val.internal->val._str);
	}

	return *this;
}

URI::Value::Type URI::Value::type() const
{
	return internal->type;
}
std::string URI::Value::readString(const std::string& fmt) const
{
	switch (internal->type)
	{
	case Type_Char:
	{
		char buffer[32] = { 0 };
		snprintf(buffer, 31, fmt == "" ? "%c" : fmt.c_str(), (char)internal->val._int);
		return buffer;
	}
	case Type_Int32:
	{
		char buffer[32] = { 0 };
		snprintf(buffer, 31, fmt == "" ? "%d" : fmt.c_str(), (int)internal->val._int);
		return buffer;
	}
	case Type_Int64:
	{
		char buffer[32] = { 0 };
		snprintf(buffer, 31, fmt == "" ? "%lld" : fmt.c_str(), (int64_t)internal->val._int);
		return buffer;
	}
	case Type_String:
		return internal->val._str;
	case Type_Double:
	{
		char buffer[32] = { 0 };
		snprintf(buffer, 31, fmt == "" ? "%lf" : fmt.c_str(), internal->val._float);
		return buffer;
	}
	case Type_Bool:
		return internal->val._bool ? "true" : "false";
	}

	return "";
}
int URI::Value::readInt(const std::string& fmt) const
{
	switch (internal->type)
	{
	case Type_Char:
	case Type_Int32:
	case Type_Int64:
		return (int)internal->val._int;
	case Type_String:
	{
		int val = 0;
		sscanf(internal->val._str, fmt == "" ? "%d" : fmt.c_str(),&val);
		return val;
	}
	case Type_Double:
		return (int)internal->val._float;
	case Type_Bool:
		return internal->val._bool ? 1 : 0;
	}

	return 0;
}
float URI::Value::readFloat(const std::string& fmt) const
{
	switch (internal->type)
	{
	case Type_Char:
	case Type_Int32:
	case Type_Int64:
		return (float)internal->val._int;
	case Type_String:
	{
		float val = 0;
		sscanf(internal->val._str, fmt == "" ? "%f" : fmt.c_str(), &val);
		return val;
	}
	case Type_Double:
		return (float)internal->val._float;
	case Type_Bool:
		return internal->val._bool ? (float)1.0 : (float)0.0;
	}

	return (float)0.0;
}

long long URI::Value::readInt64(const std::string& fmt) const
{
	switch (internal->type)
	{
	case Type_Char:
	case Type_Int32:
	case Type_Int64:
		return (long long)internal->val._int;
	case Type_String:
	{
		long long val = 0;
		sscanf(internal->val._str, fmt == "" ? "%lld" : fmt.c_str(), &val);
		return val;
	}
	case Type_Double:
		return (long long)internal->val._float;
	case Type_Bool:
		return internal->val._bool ? 1 : 0;
	}

	return 0;
}

uint32_t URI::Value::readUint32(const std::string& fmt) const
{
	switch (internal->type)
	{
	case Type_Char:
	case Type_Int32:
	case Type_Int64:
		return (uint32_t)internal->val._int;
	case Type_String:
	{
		uint32_t val = 0;
		sscanf(internal->val._str, fmt == "" ? "%u" : fmt.c_str(), &val);
		return val;
	}
	case Type_Double:
		return (uint32_t)internal->val._float;
	case Type_Bool:
		return internal->val._bool ? 1 : 0;
	}

	return 0;
}

uint64_t URI::Value::readUint64(const std::string& fmt) const
{
	switch (internal->type)
	{
	case Type_Char:
	case Type_Int32:
	case Type_Int64:
		return (uint64_t)internal->val._int;
	case Type_String:
	{
		uint64_t val = 0;
		sscanf(internal->val._str, fmt == "" ? "%llu" : fmt.c_str(), &val);
		return val;
	}
	case Type_Double:
		return (uint64_t)internal->val._float;
	case Type_Bool:
		return internal->val._bool ? 1 : 0;
	}

	return 0;
}
bool URI::Value::readBool() const
{
	switch (internal->type)
	{
	case Type_Char:
	case Type_Int32:
	case Type_Int64:
		return internal->val._int != 0;
	case Type_String:
		return strcasecmp(internal->val._str, "true") == 0;
	case Type_Double:
		return (int)internal->val._float != 0;
	case Type_Bool:
		return internal->val._bool;
	}

	return false;
}
bool URI::Value::isEmpty() const
{
	return internal->type == Type_Empty;
}
//http://user:pass@host.com:8080/p/a/t/h?query=string#hash
URI::URI()
{
	clean();
}
URI::URI(const URI& URI)
{
	href(URI.href());
}
URI::URI(const std::string& URIstr)
{
	href(URIstr);
}
URI::~URI()
{
}

URI& URI::operator = (const URI& URI)
{
	protocol = URI.protocol;
	pathname = URI.pathname;
	authen = URI.authen;
	port = URI.port;
	query = URI.query;

	return *this;
}
URI& URI::operator = (const std::string& _href)
{
	href(_href);

	return *this;
}

void URI::clean()
{
	protocol = "http";
	authen.Username = authen.Password = "";
	pathname = "/";
	port = 80;
	query.clear();
}

std::string URI::href() const
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
void URI::href(const std::string& URI)
{
	clean();

	const char* URItmp = URI.c_str();
	//解析协议
	const char* tmp = strstr(URItmp, "://");
	if (tmp != NULL)
	{
		setProtocol(std::string(URItmp, tmp - URItmp));
		URItmp = tmp += 3;
	}
	//解析用户密码
	tmp = strchr(URItmp, '@');
	if (tmp != NULL)
	{
		setAuthen(std::string(URItmp, tmp - URItmp));
		URItmp = tmp + 1;
	}
	//解析主机信息
	if (*URItmp != '/')
	{
		tmp = strchr(URItmp, '/');
		if (tmp == NULL) tmp = URItmp + strlen(URItmp);

		setHost(std::string(URItmp, tmp - URItmp));

		URItmp = tmp;
	}

	setPath(URItmp);

	if (pathname == "") pathname = "/";
}

const std::string& URI::getProtocol() const
{
	return protocol;
}
void URI::setProtocol(const std::string& protocolstr)
{
	protocol = protocolstr;
}

std::string URI::getAuhen() const
{
	if (authen.Username == "" || authen.Password == "") return "";
	if (authen.Password == "") return authen.Username;
	return std::string(authen.Username) + ":" + authen.Password;
}
const URI::AuthenInfo& URI::getAuthenInfo() const
{
	return authen;
}
void URI::setAuthen(const std::string& authenstr)
{
	authen.Username = authenstr;
	const char* authenstrtmp = authenstr.c_str();
	const char* tmp1 = strchr(authenstrtmp, ':');
	if (tmp1 != NULL)
	{
		authen.Username = std::string(authenstrtmp, tmp1 - authenstrtmp);
		authen.Password = tmp1 + 1;
	}
}
void URI::setAuthen(const std::string& username, std::string& password)
{
	authen.Username = username;
	authen.Password = password;
}
void URI::setAuthen(const AuthenInfo& info)
{
	authen = info;
}
std::string URI::getHost() const
{
	stringstream sstream;
	sstream << hostname;
	if (port != 80) sstream << ":" << port;

	return sstream.str();
}
void URI::setHost(const std::string& hoststr)
{
	std::string hostnamestr = hostname = hoststr;
	const char* hostnametmp = hostnamestr.c_str();
	const char* tmp1 = strchr(hostnametmp, ':');
	if (tmp1 != NULL)
	{
		hostname = std::string(hostnametmp, tmp1 - hostnametmp);
		port = atoi(tmp1 + 1);
	}
}

const std::string& URI::getHostname() const
{
	return hostname;
}
void URI::setHostname(const std::string& hostnamestr)
{
	hostname = hostnamestr;
}

uint32_t URI::getPort() const
{
	return port;
}
void URI::setPort(uint32_t portnum)
{
	port = portnum;
}

std::string URI::getPath() const
{
	std::string querystr = getSearch();

	return pathname + querystr;
}
void URI::setPath(const std::string& pathstr)
{
	pathname = pathstr;

	const char* URItmp = pathstr.c_str();
	const char* tmp = strchr(URItmp, '?');
	if (tmp != NULL)
	{
		pathname = std::string(URItmp, tmp - URItmp);

		setSearch(tmp + 1);
	}
}

const std::string& URI::getPathname() const
{
	return pathname;
}
void URI::setPathname(const std::string& pathnamestr)
{
	pathname = pathnamestr;
}

std::string URI::getSearch() const
{
	stringstream sstream;
	
	for (std::map<std::string, URI::Value>::const_iterator iter = query.begin(); iter != query.end(); iter++)
	{
		sstream << (iter == query.begin() ? "?":"&") << iter->first << "=" << iter->second.readString();
	}

	return sstream.str();
}
void URI::setSearch(const std::string& searchstr)
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

		query[key] = val;

		if (tmp1 == NULL) break;
		querystr = tmp1 + 1;
	}
}

const std::map<std::string, URI::Value>& URI::getQuery() const
{
	return query;
}
void URI::setQuery(const std::map<std::string, URI::Value>& queryobj)
{
	query = queryobj;
}


}
}


