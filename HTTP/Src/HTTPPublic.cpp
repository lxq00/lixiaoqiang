#include "boost/asio.hpp"
#include "HTTP/HTTPPublic.h"
#include "HTTPDefine.h"
#include "HTTPCache.h"
namespace Public {
namespace HTTP {

struct ReadContent::ReadContentInternal
{
	shared_ptr<HTTPCache> cache;
	std::string filename;
};
ReadContent::ReadContent(HTTPCacheType type, const std::string& filename)
{
	internal = new ReadContentInternal;

	internal->filename = filename;
	if (internal->filename.length() > 0)
	{
		internal->cache = make_shared<HTTPCacheFile>(internal->filename, false, false);
	}
	else if (type == HTTPCacheType_Mem) internal->cache = make_shared<HTTPCacheMem>();
	else
	{
		internal->filename = HTTPServerCacheFilePath::instance()->cachefilename();
		internal->cache = make_shared<HTTPCacheFile>(internal->filename, true, false);
	}
}
ReadContent::~ReadContent()
{
	internal->cache = NULL;
	SAFE_DELETE(internal);
}
int ReadContent::size() const
{
	return internal->cache->totalsize();
}

std::string ReadContent::cacheFileName() const
{
	return internal->filename;
}
int ReadContent::read(char* buffer, int maxlen) const
{
	return internal->cache->read(buffer, maxlen);
}
std::string ReadContent::read() const
{
	std::string bufferstr;
	char buffer[1024];
	while (1)
	{
		int readlen = internal->cache->read(buffer, 1024);
		if (readlen <= 0) break;

		bufferstr += std::string(buffer, readlen);
	}

	return bufferstr;
}
bool ReadContent::readToFile(const std::string& filename) const
{
	FILE* fd = fopen(filename.c_str(), "wb+");
	if (fd == NULL) return false;

	char buffer[1024];
	while (1)
	{
		int readlen = internal->cache->read(buffer, 1024);
		if (readlen <= 0) break;

		fwrite(buffer, 1, readlen, fd);
	}

	fclose(fd);

	return true;
}

uint32_t ReadContent::append(const char* buffer, uint32_t len)
{
	return internal->cache->write(buffer, len);
}
void ReadContent::read(String& data) {}




struct WriteContent::WriteContentInternal
{
	WriteContenNotify* notify;
	shared_ptr<HTTPCache> cache;
};

WriteContent::WriteContent(WriteContenNotify* notify, HTTPCacheType type)
{
	if (type == HTTPCacheType_Mem) internal->cache = make_shared<HTTPCacheMem>();
	else
	{
		std::string filename = HTTPServerCacheFilePath::instance()->cachefilename();
		internal->cache = make_shared<HTTPCacheFile>(filename, true, true);
	}
}
WriteContent::~WriteContent()
{
	SAFE_DELETE(internal);
}

bool WriteContent::setChunkEOF()
{
	return false;
}
bool WriteContent::write(const char* buffer, int len)
{
	while (len > 0)
	{
		int writelen = internal->cache->write(buffer, len);
		if (writelen <= 0) return false;

		len -= writelen;
		buffer += writelen;
	}

	if (internal->notify) internal->notify->WriteNotify();

	return true;
}
bool WriteContent::write(const std::string& buffer)
{
	return write(buffer.c_str(), buffer.length());
}
bool WriteContent::writeFromFile(const std::string& filename, bool needdeletefile)
{
	internal->cache = make_shared<HTTPCacheFile>(filename, needdeletefile, true);
	if (internal->notify) internal->notify->setWriteFileName(filename);
	if (internal->notify) internal->notify->WriteNotify();

	return true;
}

uint32_t WriteContent::append(const char* buffer, uint32_t len)
{
	return 0;
}
void WriteContent::read(String& data)
{
	char buffer[1024];

	while (1)
	{
		int readlen = internal->cache->read(buffer, 1024);
		if (readlen <= 0) break;

		data += string(buffer, readlen);
	}
}


}
}