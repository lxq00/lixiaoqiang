#include "boost/asio.hpp"
#include "HTTP/HTTPPublic.h"
#include "HTTPDefine.h"
#include "HTTPCache.h"
#include "HTTP/HTTPParse.h"
namespace Public {
namespace HTTP {

struct ReadContent::ReadContentInternal
{
	shared_ptr<HTTPCache> cache;
	std::string			  filename;
	DataCalback			  callback;
	shared_ptr<ChunkData> chunk;

	WriteContenNotify*	  notify;


	bool chunkReadCallback(const char* buffer, int size)
	{
		callback(buffer, size);

		return true;
	}
};
ReadContent::ReadContent(const shared_ptr<HTTPHeader>& header, WriteContenNotify* notify, HTTPCacheType type, const std::string& filename)
{
	internal = new ReadContentInternal;
	internal->notify = notify;
	internal->filename = filename;

	if (internal->filename.length() > 0)
	{
		internal->cache = make_shared<HTTPCacheFile>(internal->filename, false, false);
	}
	else if(type == HTTPCacheType_File)
	{
		internal->filename = HTTPServerCacheFilePath::instance()->cachefilename();
		internal->cache = make_shared<HTTPCacheFile>(internal->filename, true, false);
	}

	Value chunkval = header->header(Transfer_Encoding);
	if (!chunkval.empty() && strcasecmp(chunkval.readString().c_str(), CHUNKED) == 0)
	{
		if (internal->cache)
		{
			internal->chunk->setReadCallback(ChunkData::ReadCallback(&HTTPCache::write, internal->cache));

			if (internal->notify) internal->notify->ReadReady();
		}
	}
	else if (type == HTTPCacheType_Mem) internal->cache = make_shared<HTTPCacheMem>();
}
ReadContent::~ReadContent()
{
	internal->cache = NULL;
	SAFE_DELETE(internal);
}
uint32_t ReadContent::size()
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

void ReadContent::setDataCallback(const DataCalback& callback)
{
	if (internal->chunk)
	{
		internal->callback = callback;
		internal->chunk->setReadCallback(ChunkData::ReadCallback(&ReadContentInternal::chunkReadCallback,internal));
		internal->cache = NULL;
		if (internal->notify)
		{
			internal->notify->ReadReady();
		}
	}
}

uint32_t ReadContent::append(const char* buffer, uint32_t len)
{
	if (internal->chunk) return internal->chunk->append(buffer, len);
	else if(internal->cache)return internal->cache->write(buffer, len);

	return len;
}
void ReadContent::read(String& data) {}




struct WriteContent::WriteContentInternal
{
	shared_ptr<HTTPHeader> header;
	WriteContenNotify* notify;
	shared_ptr<HTTPCache> cache;

	shared_ptr<ChunkData> chunk;

	virtual void setWriteFileName(const std::string& filename)
	{
		std::string contenttype = "application/octet-stream";

		do
		{
			int pos = (int)String::lastIndexOf(filename, ".");
			if (pos == -1) break;

			std::string pres = filename.c_str() + pos + 1;

			uint32_t mimetypeslen = 0;
			ContentInfo* mimetypes = MediaType::info(mimetypeslen);

			bool haveFind = false;
			for (uint32_t i = 0; i < mimetypeslen; i++)
			{
				if (strcasecmp(pres.c_str(), mimetypes[i].filetype) == 0)
				{
					contenttype = mimetypes[i].contentType;
					break;
				}
			}

		} while (0);

		header->headers[Content_Type] = contenttype;
	}
};

WriteContent::WriteContent(const shared_ptr<HTTPHeader>& header,WriteContenNotify* notify, HTTPCacheType type)
{
	internal = new WriteContentInternal;
	internal->header = header;

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
bool WriteContent::writeString(const std::string& buffer)
{
	return write(buffer.c_str(), buffer.length());
}
bool WriteContent::writeFromFile(const std::string& filename, bool needdeletefile)
{
	internal->cache = make_shared<HTTPCacheFile>(filename, needdeletefile, true);
	internal->setWriteFileName(filename);

	if (internal->notify) internal->notify->WriteNotify();

	return true;
}
uint32_t WriteContent::size()
{
	return internal->cache->totalsize();
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

void WriteContent::writeChunk(const char* buffer, uint32_t len)
{
	if (!internal->chunk)
	{
		internal->header->headers[Transfer_Encoding] = CHUNKED;

		internal->chunk = make_shared<ChunkData>(ChunkData::WriteCallback(&WriteContent::write, this));
	}

	if(internal->chunk)
		internal->chunk->append(buffer, len);
}
void WriteContent::writeChunkEnd()
{
	writeChunk(NULL, 0);
}

}
}