#include "boost/asio.hpp"
#include "HTTP/HTTPPublic.h"
namespace Public {
namespace HTTP {

HTTPBuffer::HTTPBuffer()
{
}
HTTPBuffer::~HTTPBuffer() 
{
}

uint32_t HTTPBuffer::size()
{
	return (uint32_t)buf.gcount();
}
//when end of file return ""
int HTTPBuffer::read(char* buffer, int len) const
{
	if (buffer == NULL || len <= 0) return 0;

	std::string string = read();

	int readlen = min(len, (int)string.size());
	memcpy(buffer, string.c_str(), readlen);

	return readlen;
}

std::string HTTPBuffer::read() const
{
	return buf.str();
}
bool HTTPBuffer::readToFile(const std::string& filename) const
{
	ofstream outfile;
	outfile.open(filename.c_str(), std::ofstream::out | std::ofstream::binary);
	if (!outfile.is_open()) return false;

	outfile << buf.str();

	outfile.close();

	return true;
}


bool HTTPBuffer::write(const char* buffer, int len)
{
	if (buffer == NULL || len <= 0) return false;

	return write(buffer, len);
}
bool HTTPBuffer::write(const std::string& buffer)
{
	buf << buffer;

	return true;
}
bool HTTPBuffer::write(const HTTPTemplate& temp)
{
	return write(temp.toValue());
}
bool HTTPBuffer::writeFromFile(const std::string& filename, bool deleteFile)
{
	ifstream infile;
	infile.open(filename, ifstream::in | ifstream::binary);
	if (!infile.is_open()) return false;

	buf << infile.rdbuf();

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

const URI& HTTPRequest::getUrl() const
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