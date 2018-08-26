#include "boost/asio.hpp"
#include "HTTP/HTTPPublic.h"
namespace Public {
namespace HTTP {

struct HTTPBuffer::HTTPBufferInternal
{
	HTTPBufferType  readtype;
	std::string		readfilename;
	bool			readfiledelete;
	FILE*			readfile;
	StreamCallback	readcallback;

	uint32_t		writetcontentsize;
	std::string		writetcontent;
	HTTPBufferType  writetype;
	int				writefilesize;
	std::string		writefilename;
	bool			writefiledelete;
	FILE*			writefile;


	HTTPBufferInternal(HTTPBufferType _type)
		:readtype(_type), readfiledelete(false), readfile(NULL)
		, writetype(HTTPBufferType_String), writefilesize(0), writefiledelete(false), writefile(NULL), writetcontentsize(0){}
	~HTTPBufferInternal()
	{
		cleanread();
		cleanwrite();
	}

	void cleanread()
	{
		if (readtype == HTTPBufferType_File && readfile != NULL)
		{
			fclose(readfile);
			readfile = NULL;
		}
		if (readfiledelete && File::access(readfilename, File::accessExist))
		{
			File::remove(readfilename);
		}
		readtype = HTTPBufferType_String;
	}
	void cleanwrite()
	{
		if (writetype == HTTPBufferType_File && writefile != NULL)
		{
			fclose(writefile);
			writefile = NULL;
		}
		if (writefiledelete && File::access(writefilename, File::accessExist))
		{
			File::remove(writefilename);
		}
		writetype = HTTPBufferType_String;
		writetcontent = "";
		writetcontentsize = 0;
	}
};

HTTPBuffer::HTTPBuffer(HTTPBufferType type)
{
	internal = new HTTPBufferInternal(type);
}
HTTPBuffer::~HTTPBuffer() 
{
	SAFE_DELETE(internal);
}

HTTPBuffer::HTTPBufferType HTTPBuffer::readtype()
{
	return internal->readtype;
}

int HTTPBuffer::size()
{
	if (internal->writetype == HTTPBufferType_File)
	{
		return internal->writefilesize;
	}
	else if (internal->writetype == HTTPBufferType_String)
	{
		return  internal->writetcontentsize;
	}

	return -1;
}
std::string HTTPBuffer::read(int start, int maxlen)
{
	if (internal->writetype == HTTPBufferType_String)
	{
		if (start > (int)internal->writetcontent.size())  return "";
		int readlen = min(maxlen, (int)internal->writetcontent.size() - start);

		return std::string(internal->writetcontent.c_str() + start, readlen);
	}
	else if (internal->writetype == HTTPBufferType_File && internal->writefile != NULL)
	{
		fseek(internal->writefile, SEEK_SET, start);
		char* readbuffer = new char[maxlen];
		int readlen = fread(readbuffer, 1, maxlen, internal->writefile);

		std::string content(readbuffer, readlen < 0 ? 0 : readlen);

		delete[] readbuffer;

		return std::move(content);
	}

	return "";
}

std::string HTTPBuffer::read()
{
	if (internal->writetype == HTTPBufferType_String)
	{
		return internal->writetcontent;
	}
	else if (internal->writetype == HTTPBufferType_File && internal->writefile != NULL)
	{
		fseek(internal->writefile, SEEK_SET, 0);
		char* readbuffer = new char[internal->writefilesize];
		int readlen = fread(readbuffer, 1, internal->writefilesize, internal->writefile);

		std::string content(readbuffer, readlen < 0 ? 0 : readlen);

		delete[] readbuffer;

		return std::move(content);
	}

	return "";
}
bool HTTPBuffer::readToFile(const std::string& filename, bool deleteFile)
{
	internal->cleanread();
	internal->readfile = fopen(filename.c_str(), "wb+");
	internal->readtype = HTTPBufferType_File;
	internal->readfiledelete = deleteFile;
	
	if (internal->readfile == NULL)
	{
		return false;
	}

	if (internal->writetype == HTTPBufferType_String)
	{
		fwrite(internal->writetcontent.c_str(), 1, internal->writetcontent.size(), internal->readfile);
	}
	else if (internal->writetype == HTTPBufferType_File && internal->writefile != NULL)
	{
		fseek(internal->writefile, SEEK_SET, 0);
		char buffer[1024];
		while (1)
		{
			int readlen = fread(buffer, 1, 1024, internal->writefile);
			if (readlen <= 0) break;
			fwrite(buffer, 1, readlen, internal->readfile);
		}
	}
	else
	{
		return true;
	}

	return true;
}
void HTTPBuffer::setReadCallback(const StreamCallback& callback)
{
	internal->cleanread();
	internal->readcallback = callback;
	internal->readtype = HTTPBufferType_Stream;
}
HTTPBuffer::HTTPBufferType HTTPBuffer::writetype()
{
	return internal->writetype;
}
void HTTPBuffer::writetype(HTTPBufferType type)
{
	internal->writetype = type;
}
bool HTTPBuffer::write(const char* buffer, int len)
{
	if (buffer == NULL || len <= 0) return false;

	//internal->writetcontent = std::string(buffer, len);
	internal->writetcontentsize += len;
	if (internal->readtype == HTTPBufferType_String)
	{
		internal->writetcontent += std::string(buffer, len);

		return true;
	}
	else if (internal->readtype == HTTPBufferType_File && internal->readfile != NULL)
	{
		int writelen = fwrite(buffer, 1, len, internal->readfile);
		return writelen == len;
	}
	else if (internal->readtype == HTTPBufferType_Stream && internal->readcallback != NULL)
	{
		internal->readcallback(this, buffer, len);

		return true;
	}

	return false;
}
bool HTTPBuffer::write(const std::string& buffer)
{
	return write(buffer.c_str(), buffer.length());
}
//bool HTTPBuffer::write(const HTTPTemplate& temp)
//{
//	return write(temp.toValue());
//}
bool HTTPBuffer::writeFromFile(const std::string& filename, bool deleteFile)
{
	internal->cleanwrite();

	FileInfo info;
	info.size = 0;
	File::stat(filename, info);

	internal->writefilesize = (int)info.size;
	internal->writefile = fopen(filename.c_str(), "rb");
	internal->writetype = HTTPBufferType_File;
	internal->writefiledelete = deleteFile;

	if (internal->readtype == HTTPBufferType_File && internal->readfile != NULL && internal->writefile != NULL)
	{
		fseek(internal->writefile, SEEK_SET, 0);
		char buffer[1024];
		while (1)
		{
			int readlen = fread(buffer, 1, 1024, internal->writefile);
			if (readlen <= 0) break;
			fwrite(buffer, 1, readlen, internal->readfile);
		}
	}
	else if (internal->readtype == HTTPBufferType_Stream && internal->readcallback != NULL && internal->writefile != NULL)
	{
		fseek(internal->writefile, SEEK_SET, 0);
		char buffer[1024];
		while (1)
		{
			int readlen = fread(buffer, 1, 1024, internal->writefile);
			if (readlen <= 0) break;
			internal->readcallback(this, buffer, readlen);
		}
	}

	return internal->writefile != NULL;
}

struct HTTPRequest::HTTPRequestInternal
{
	std::string					 method;
	URL							 url;
	std::map<std::string, Value> headers;
	uint32_t					 timeout;

	NetAddr						 remoteAddr;
	NetAddr						 myAddr;
	DisconnectCallback			 discallback;

	shared_ptr<HTTPBuffer>		 content;

	HTTPRequestInternal(HTTPBuffer::HTTPBufferType type) :method("GET"),timeout(5000)
	{
		content = make_shared<HTTPBuffer>(type);
	}
};

HTTPRequest::HTTPRequest(HTTPBuffer::HTTPBufferType type)
{
	internal = new HTTPRequestInternal(type);
}
HTTPRequest::~HTTPRequest() 
{
	SAFE_DELETE(internal);
}

std::map<std::string, Value>& HTTPRequest::headers()
{
	return internal->headers;
}
Value HTTPRequest::header(const std::string& key)
{
	for (std::map<std::string, Value>::const_iterator iter = internal->headers.begin(); iter != internal->headers.end(); iter++)
	{
		if (strcasecmp(key.c_str(), iter->first.c_str()) == 0) return iter->second;
	}
	
	return Value();
}
std::string& HTTPRequest::method()
{
	return internal->method;
}

URL& HTTPRequest::url()
{
	return internal->url;
}

shared_ptr<HTTPBuffer>& HTTPRequest::content()
{
	return internal->content;
}

uint32_t& HTTPRequest::timeout()
{
	return internal->timeout;
}

NetAddr& HTTPRequest::remoteAddr()
{
	return internal->remoteAddr;
}

NetAddr& HTTPRequest::myAddr()
{
	return internal->myAddr;
}

HTTPRequest::DisconnectCallback&	HTTPRequest::discallback()
{
	return internal->discallback;
}

bool HTTPRequest::push(){return false;}

struct HTTPResponse::HTTPResponseInternal
{
	uint32_t statusCode;
	std::string errorMessage;

	std::map<std::string, Value> headers;
	shared_ptr<HTTPBuffer> content;

	HTTPResponseInternal(HTTPBuffer::HTTPBufferType type):statusCode(200),errorMessage("OK")
	{
		content = make_shared<HTTPBuffer>(type);
	}
};

HTTPResponse::HTTPResponse(HTTPBuffer::HTTPBufferType type)
{
	internal = new HTTPResponseInternal(type);
}
HTTPResponse::~HTTPResponse()
{
	SAFE_DELETE(internal);
}

uint32_t& HTTPResponse::statusCode()
{
	return internal->statusCode;
}
std::string& HTTPResponse::errorMessage()
{
	return internal->errorMessage;
}

std::map<std::string, Value>& HTTPResponse::headers()
{
	return internal->headers;
}
Value HTTPResponse::header(const std::string& key)
{
	for (std::map<std::string, Value>::const_iterator iter = internal->headers.begin(); iter != internal->headers.end(); iter++)
	{
		if (strcasecmp(key.c_str(), iter->first.c_str()) == 0) return iter->second;
	}

	return Value();
}

shared_ptr<HTTPBuffer>& HTTPResponse::content()
{
	return internal->content;
}

bool HTTPResponse::push(){return false;}

}
}