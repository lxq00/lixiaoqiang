#include "HTTP/HTTPPublic.h"

namespace Public {
namespace HTTP {

//http://user:pass@host.com:8080/p/a/t/h?query=string#hash
URL::URL()
{
	clean();
}
URL::URL(const URL& url)
{
	href(url.href());
}
URL::URL(const std::string& urlstr)
{
	href(urlstr);
}
URL::~URL()
{
}

void URL::clean()
{
	protocol = "http";
	authen.Username = authen.Password = pathname = "";
	port = 80;
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
void URL::href(const std::string& url)
{
	clean();

	const char* urltmp = url.c_str();
	//解析协议
	const char* tmp = strstr(urltmp,"://");
	if (tmp != NULL)
	{
		setProtocol(std::string(urltmp, tmp - urltmp));
		urltmp = tmp += 3;
	}
	//解析用户密码
	tmp = strchr(urltmp, '@');
	if (tmp != NULL)
	{
		setAuthen(std::string(urltmp, tmp - urltmp));
		urltmp = tmp + 1;
	}
	//解析主机信息
	if (*urltmp != '/')
	{
		tmp = strchr(urltmp, '/');
		if (tmp == NULL) tmp = urltmp + strlen(urltmp);

		setHost(std::string(urltmp, tmp - urltmp));

		urltmp = tmp;
	}

	setPath(urltmp);
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
	if (authen.Password == "") return authen.Username;
	return std::string(authen.Username) + ":" + authen.Password;
}
const URL::AuthenInfo& URL::getAuthenInfo() const
{
	return authen;
}
void URL::setAuthen(const std::string& authenstr)
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
	if (port != 80) sstream << ":" << port;

	return sstream.str();
}
void URL::setHost(const std::string& hoststr)
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

const std::string& URL::getHostname() const
{
	return hostname;
}
void URL::setHostname(const std::string& hostnamestr)
{
	hostname = hostnamestr;
}

uint32_t URL::getPort() const
{
	return port;
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
	pathname = pathstr;

	const char* urltmp = pathstr.c_str();
	const char* tmp = strchr(urltmp, '?');
	if (tmp != NULL)
	{
		pathname = std::string(urltmp, tmp - urltmp);

		setSearch(tmp + 1);
	}
}

const std::string& URL::getPathname() const
{
	return pathname;
}
void URL::setPathname(const std::string& pathnamestr)
{
	pathname = pathnamestr;
}

std::string URL::getSearch() const
{
	stringstream sstream;

	if (query.size() > 0) sstream << "?";

	for (std::map<std::string, URI::Value>::const_iterator iter = query.begin(); iter != query.end(); iter++)
	{
		sstream << iter->first << "=" << iter->second.readString();
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

		query[key] = val;

		if (tmp1 == NULL) break;
		querystr = tmp1 + 1;
	}
}

const std::map<std::string, URI::Value>& URL::getQuery() const
{
	return query;
}
void URL::setQuery(const std::map<std::string, URI::Value>& queryobj)
{
	query = queryobj;
}


HTTPBuffer::HTTPBuffer() {}
HTTPBuffer::~HTTPBuffer() {}

uint32_t HTTPBuffer::size()
{
	return buf.gcount();
}

//when end of file return ""
int HTTPBuffer::read(char* buffer, int len) const
{
	if (buffer == NULL || len <= 0) return 0;

	std::string stringbuf = buf.str();
	int readlen = min(len, stringbuf.size());
	memcpy(buffer, stringbuf.c_str(), readlen);

	return readlen;
}
int HTTPBuffer::read(std::ofstream& outfile) const
{
	std::string stringbuf = buf.str();

	outfile.write(stringbuf.c_str(), stringbuf.length());
	
	return stringbuf.length();
}
bool HTTPBuffer::readToFile(const std::string& filename) const
{
	ofstream outfile;
	outfile.open(filename.c_str(), std::ofstream::out | std::ofstream::binary);
	if (!outfile.is_open()) return false;

	read(outfile);
	outfile.close();

	return true;
}


bool HTTPBuffer::write(const char* buffer, int len)
{
	if (buffer == NULL || len <= 0) return false;

	buf << std::string(buffer, len);

	return true;
}
bool HTTPBuffer::write(const HTTPTemplate& temp)
{
	buf << temp.toValue();

	return true;
}
bool HTTPBuffer::write(std::ifstream& infile)
{
	while (!infile.eof())
	{
		char buffer[1024];
		streamsize readlen = infile.readsome(buffer, 1024);
		if (readlen == 0) break;

		buf << std::string(buffer, readlen);
	}

	return true;
}
bool HTTPBuffer::writeFromFile(const std::string& filename, bool deleteFile)
{
	ifstream infile;
	infile.open(filename.c_str(), std::ofstream::in | std::ofstream::binary);
	if (!infile.is_open()) return false;

	write(infile);
	infile.close();

	return true;
}


HTTPRequest::HTTPRequest():timeout(30000) {}
HTTPRequest::~HTTPRequest() {}

std::map<std::string, URI::Value> HTTPRequest::getHeaders() const
{
	return headers;
}
std::string HTTPRequest::getHeader(const std::string& key) const
{
	std::map<std::string, URI::Value>::const_iterator iter = headers.find(key);
	if (iter == headers.end())return "";

	return iter->second.readString();
}

std::string HTTPRequest::getMethod() const
{
	return method;
}

const URL& HTTPRequest::getUrl() const
{
	return url;
}

const HTTPBuffer& HTTPRequest::buffer() const
{
	return buf;
}
void HTTPRequest::setTimeout(uint32_t timeout_ms)
{
	timeout = timeout_ms;
}

HTTPResponse::HTTPResponse() :statusCode(200) {}
HTTPResponse::~HTTPResponse() {}

bool HTTPResponse::setStatusCode(uint32_t code, const std::string& statusMessage)
{
	statusCode = code;
	errorMessage = statusMessage;

	return true;
}

bool HTTPResponse::setHeader(const std::string& key, const URI::Value& val)
{
	headers[key] = val;

	return true;
}
std::string HTTPResponse::getHeader(const std::string& key) const
{
	std::map<std::string, URI::Value>::const_iterator iter = headers.find(key);
	if (iter == headers.end())return "";

	return iter->second.readString();
}

const HTTPBuffer& HTTPResponse::buffer() const
{
	return buf;
}

}
}