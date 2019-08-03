#include "boost/asio.hpp"
#include "HTTP/HTTPPublic.h"
#include "HTTPDefine.h"
namespace Public {
namespace HTTP {

class FileMediaInfo
{
public:
	FileMediaInfo(){}
	virtual ~FileMediaInfo() {}
	void setContentFilename(const std::string& filename)
	{
		int pos = (int)String::lastIndexOf(filename, ".");
		if (pos == -1) return;

		std::string pres = filename.c_str() + pos + 1;
		
		uint32_t mimetypeslen = 0;
		ContentInfo* mimetypes = MediaType::info(mimetypeslen);
		
		bool haveFind = false;
		for (uint32_t i = 0; i < mimetypeslen; i++)
		{
			if (strcasecmp(pres.c_str(), mimetypes[i].filetype.c_str()) == 0)
			{
				headers[Content_Type] = mimetypes[i].contentType;
				haveFind = true;
				break;
			}
		}
		if (!haveFind)
		{
			headers[Content_Type] = "application/octet-stream";
		}
	}
	virtual Value getHeader(const std::string& key)
	{
		for (std::map<std::string, Value>::const_iterator iter = headers.begin(); iter != headers.end(); iter++)
		{
			if (strcasecmp(key.c_str(), iter->first.c_str()) == 0) return iter->second;
		}

		return Value();
	}
	std::map<std::string, Value>& headersmap() { return headers; }
private:
	std::map<std::string, Value> headers;
};


struct HTTPContent::HTTPContentInternal
{
	FileMediaInfo*	mediainfo;

	shared_ptr<HTTPCache> cache;

	HTTPContentType  writetype;

	uint32_t		chunkbodysize;

	CheckConnectionIsOk	checkfunc;

	std::string			readtofilename;
	ReadDataCallback	readcalblack;

	WriteDataCallback	writecallback;

	HTTPContentInternal(FileMediaInfo* info)
		:mediainfo(info),writetype(HTTPContentType_Normal), chunkbodysize(0)
	{
		cache = make_shared<HTTPCacheMem>();
	}
	~HTTPContentInternal()
	{
		cache = NULL;
	}
};

HTTPContent::HTTPContent(FileMediaInfo* info,const CheckConnectionIsOk& check)
{
	internal = new HTTPContentInternal(info);
	internal->checkfunc = check;
}
HTTPContent::~HTTPContent() 
{
	SAFE_DELETE(internal);
}

int HTTPContent::size()
{
	return internal->cache->totalsize();
}

int HTTPContent::read(char* buffer, int maxlen)
{
	if (buffer == NULL || maxlen <= 0) return 0;

	if (internal->writecallback)
	{
		return internal->writecallback(buffer, maxlen);
	}

	return internal->cache->read(buffer, maxlen);
}
std::string HTTPContent::read()
{
	std::string memdata;

	char buffer[1024];
	while (true)
	{
		int readlen = internal->cache->read(buffer, 1024);
		if (readlen <= 0) break;

		memdata += std::string(buffer,readlen);
	}

	return std::move(memdata);
}
bool HTTPContent::setReadToFile(const std::string& filename, bool deletefile)
{
	shared_ptr<HTTPCacheFileRead> cache = make_shared<HTTPCacheFileRead>(filename, deletefile, false);
	if (cache->fd == NULL) return false;

	internal->readtofilename = filename;
	internal->cache = cache;

	return true;
}
std::string HTTPContent::cacheFileName() const
{
	return internal->readtofilename;
}
bool HTTPContent::readToFile(const std::string& filename)
{
	FILE* readfile = fopen(filename.c_str(), "wb+");
	if (readfile == NULL) return false;
	
	char buffer[1024];
	while (1)
	{
		int readlen = internal->cache->read(buffer, 1024);
		if (readlen <= 0) break;
		fwrite(buffer, 1, readlen, readfile);
	}
	fclose(readfile);

	return true;
}
bool HTTPContent::setReadCallback(const ReadDataCallback& callback)
{
	internal->readcalblack = callback;

	return true;
}

HTTPContent::HTTPContentType& HTTPContent::writetype()
{
	return internal->writetype;
}
bool HTTPContent::setChunkEOF()
{
	if (internal->writetype != HTTPContentType_Chunk) return false;

	char buffer[32] = { 0 };
	sprintf(buffer, "0" HTTPHREADERLINEEND HTTPHREADERLINEEND);

	internal->cache->write(buffer, (int)strlen(buffer));

	return true;
}
bool HTTPContent::write(const char* buffer, int len)
{
	if (buffer == NULL && len <= 0) return false;

	if (internal->readcalblack)
	{
		internal->readcalblack(buffer, len);

		return true;
	}

	if (internal->checkfunc != NULL && !internal->checkfunc()) return false;

	//write chunk header
	if (internal->writetype == HTTPContentType_Chunk)
	{
		char buffer[32] = { 0 };
		sprintf(buffer, "%x" HTTPHREADERLINEEND, len);
		if (!internal->cache->write(buffer, (int)strlen(buffer)))
		{
			return false;
		}
	}

	if (!internal->cache->write(buffer, len)) return false;
	
	//write chunk end
	if (internal->writetype == HTTPContentType_Chunk)
	{
		if (!internal->cache->write(HTTPHREADERLINEEND, (int)strlen(HTTPHREADERLINEEND)))
		{
			return false;
		}
	}

	return true;
}
bool HTTPContent::write(const std::string& buffer)
{
	return write(buffer.c_str(), (int)buffer.length());
}

const char* HTTPContent::inputAndParse(const char* buffer, int len, bool chunked, bool& chunedfinish)
{
	chunedfinish = false;

	//not chunked
	if(!chunked)
	{
		internal->writetype = HTTPContentType_Normal;

		write(buffer, len);
		return buffer + len;
	}


	//chunked
	internal->writetype = HTTPContentType_Chunk;
	while (len >= (int)strlen(HTTPHREADERLINEEND))
	{
		//find 
		if (internal->chunkbodysize == 0)
		{
			int pos = (int)String::indexOf(buffer, len, HTTPHREADERLINEEND);
			if (pos == -1) return buffer;

			std::string chunksizestr = std::string(buffer, pos);
			sscanf(String::tolower(chunksizestr).c_str(), "%x", &internal->chunkbodysize);
			//chuned eof
			if (internal->chunkbodysize == 0)
			{
				if (len < (int)(pos + strlen(HTTPHREADERLINEEND)*2)) return buffer;
				//是否结束标识 2个HTTPHREADERLINEEND 
				if (memcmp(buffer + pos, HTTPHREADERLINEEND HTTPHREADERLINEEND, strlen(HTTPHREADERLINEEND) * 2) != 0) return buffer;

				len -= pos + (int)strlen(HTTPHREADERLINEEND) * 2;
				buffer += pos + (int)strlen(HTTPHREADERLINEEND) * 2;

				chunedfinish = true;
				return buffer;
			}
			buffer += pos + (int)strlen(HTTPHREADERLINEEND);
			len -= pos + (int)strlen(HTTPHREADERLINEEND);

			//数据长度加上结束标识长度
			internal->chunkbodysize += (int)strlen(HTTPHREADERLINEEND);
		}
		else
		{
			//当数据还不够时，返回数据长度，当数据满了，返回chunk缺少长度
			int datalen = min(len, (int)(internal->chunkbodysize > strlen(HTTPHREADERLINEEND) ? 
				internal->chunkbodysize - strlen(HTTPHREADERLINEEND) : internal->chunkbodysize));

			//数据不够时，处理数据
			if (internal->chunkbodysize > strlen(HTTPHREADERLINEEND))
			{
				internal->cache->write(buffer, datalen);
			}
			buffer += datalen;
			len -= datalen;
			internal->chunkbodysize -= datalen;
		}
	}

	return buffer;
}
//bool HTTPContent::write(const HTTPTemplate& temp)
//{
//	return write(temp.toValue());
//}
bool HTTPContent::writeFromFile(const std::string& filename, bool deleteFile)
{
	shared_ptr<HTTPCacheFileRead> cache = make_shared<HTTPCacheFileRead>(filename, deleteFile, true);
	if (cache->fd == NULL) return false;

	internal->mediainfo->setContentFilename(filename);
	internal->cache = cache;
	internal->writetype = HTTPContentType_Normal;

	return true;
}

bool HTTPContent::setWriteCallback(const WriteDataCallback& _writecallback)
{
	internal->writecallback = _writecallback;

	return true;
}

struct HTTPRequest::HTTPRequestInternal:public FileMediaInfo
{
	std::string					 method;
	URL							 url;
	
	uint32_t					 timeout;

	NetAddr						 remoteAddr;
	NetAddr						 myAddr;
	DisconnectCallback			 discallback;

	shared_ptr<HTTPContent>		 content;

	HTTPRequestInternal() :method("GET"),timeout(5000)
	{
		content = make_shared<HTTPContent>(this);
	}
};

HTTPRequest::HTTPRequest()
{
	internal = new HTTPRequestInternal();
}
HTTPRequest::~HTTPRequest() 
{
	internal->content = NULL;
	SAFE_DELETE(internal);
}

std::map<std::string, Value>& HTTPRequest::headers()
{
	return internal->headersmap();
}
Value HTTPRequest::header(const std::string& key)
{
	return internal->getHeader(key);
}
std::string& HTTPRequest::method()
{
	return internal->method;
}

URL& HTTPRequest::url()
{
	return internal->url;
}

shared_ptr<HTTPContent>& HTTPRequest::content()
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

struct HTTPResponse::HTTPResponseInternal :public FileMediaInfo
{
	uint32_t statusCode;
	std::string errorMessage;

	shared_ptr<HTTPContent> content;
	HTTPResponse::DisconnectCallback  discallback;

	HTTPResponseInternal(const HTTPContent::CheckConnectionIsOk& check):statusCode(200),errorMessage("OK")
	{
		content = make_shared<HTTPContent>(this,check);
	}
};

HTTPResponse::HTTPResponse(const HTTPContent::CheckConnectionIsOk& check)
{
	internal = new HTTPResponseInternal(check);
}
HTTPResponse::~HTTPResponse()
{
	internal->content = NULL;
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
	return internal->headersmap();
}
Value HTTPResponse::header(const std::string& key)
{
	return internal->getHeader(key);
}

shared_ptr<HTTPContent>& HTTPResponse::content()
{
	return internal->content;
}

bool HTTPResponse::push(){return false;}

HTTPResponse::DisconnectCallback&	HTTPResponse::discallback()
{
	return internal->discallback;
}

}
}