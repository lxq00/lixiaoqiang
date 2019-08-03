#pragma once
#include "Base/Base.h"
using namespace Public::Base;

namespace Public {
namespace HTTP {

class HTTPCache
{
public:
	HTTPCache() {}
	virtual ~HTTPCache() {}
	virtual bool write(const char* buffer, int len) = 0;
	virtual int read(char* buffer, int len) = 0;
	virtual int totalsize() = 0;
};

#define MAXBUFFERCACHESIZE	5*1024*1024

class HTTPCacheMem :public HTTPCache
{
	struct BufferItem
	{
		BufferItem() :havereadlen(0) {}
		std::string		data;
		int				havereadlen;
	};
public:
	HTTPCacheMem() :cachetotalsize(0) {}
	virtual ~HTTPCacheMem() {}
	virtual bool write(const char* buffer, int len)
	{
		Guard locker(mutex);
		//	if (cachetotalsize >= MAXBUFFERCACHESIZE) return false;

		BufferItem item;
		item.data = std::string(buffer, len);

		memcache.push_back(item);

		cachetotalsize += len;

		return true;
	}
	virtual int totalsize()
	{
		Guard locker(mutex);

		return cachetotalsize;
	}
	virtual int read(char* buffer, int len)
	{
		Guard locker(mutex);

		int havereadlen = 0;
		while (havereadlen < len && memcache.size() > 0)
		{
			BufferItem& item = memcache.front();
			int readlen = min((int)item.data.size() - item.havereadlen, len - havereadlen);
			memcpy(buffer + havereadlen, item.data.c_str() + item.havereadlen, readlen);
			item.havereadlen += readlen;
			havereadlen += readlen;
			if (item.havereadlen == item.data.length())
			{
				cachetotalsize -= (int)item.data.length();
				memcache.pop_front();
			}
		}

		return havereadlen;
	}
private:
	uint32_t				cachetotalsize;
	Mutex					mutex;
	std::list<BufferItem>	memcache;
};

class HTTPCacheFile :public HTTPCache
{
public:
	HTTPCacheFile(const std::string& _filename, bool _deletefile, bool readmode)
		:filename(_filename), needdelete(_deletefile), filesize(0), writepos(0), readpos(0)
	{
		fd = fopen(filename.c_str(), readmode ? "rb" : "wb+");

		FileInfo info;
		if (File::stat(filename, info) && readmode) filesize = (int)info.size;
	}
	virtual ~HTTPCacheFile()
	{
		if (fd != NULL)
		{
			fclose(fd);
			fd = NULL;
		}
		if (needdelete)
		{
			int deletetimes = 0;
			while (deletetimes++ <= 10 && File::access(filename, File::accessExist))
			{
				File::remove(filename);
				if (!File::access(filename, File::accessExist)) break;

				Thread::sleep(100);
			}
		}
	}
	virtual int totalsize()
	{
		return filesize;
	}
	virtual bool write(const char* buffer, int len)
	{
		if (fd == NULL) return false;

		fseek(fd, (int)writepos, SEEK_SET);
		size_t ret = fwrite(buffer, 1, len, fd);
		fflush(fd);
		if (ret > 0) writepos += ret;

		return ret == len;
	}
	virtual int read(char* buffer, int len)
	{
		if (fd == NULL) return 0;

		fseek(fd, (int)readpos, SEEK_SET);
		int readlen = (int)fread(buffer, 1, len, fd);
		if (readlen > 0) readpos += readlen;

		return readlen;
	}
private:
	int					filesize;
	std::string			filename;
	bool				needdelete;

	size_t				writepos;
	size_t				readpos;
public:
	FILE*				fd;
};


struct HTTPServerCacheFilePath
{
public:
	HTTPServerCacheFilePath()
	{
		cachepath = File::getExcutableFileFullPath() + PATH_SEPARATOR + ".httpcache";

		File::removeDirectory(cachepath);
	}
	~HTTPServerCacheFilePath()
	{
		File::removeDirectory(cachepath);
	}
	static HTTPServerCacheFilePath* instance()
	{
		static HTTPServerCacheFilePath file;

		return &file;
	}

	std::string cachefilename()
	{
		if (!File::access(cachepath, File::accessExist)) File::makeDirectory(cachepath);


		char buffer[256];
		snprintf(buffer, 255, "%s%c%llu_%06x.md", cachepath.c_str(), PATH_SEPARATOR, Time::getCurrentMilliSecond(), buffer);

		return buffer;
	}
private:
	std::string cachepath;
};

}
}

