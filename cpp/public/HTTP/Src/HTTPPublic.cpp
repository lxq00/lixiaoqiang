#include "HTTP/HTTPPublic.h"

namespace Public {
namespace HTTP {

//http://user:pass@host.com:8080/p/a/t/h?query=string#hash
struct URL::URLInternal
{
	std::string		href;
};

URL::URL(const URL& url)
{
	internal = new URLInternal;
	internal->href = url.internal->href;
}
URL::URL(const std::string& urlstr)
{
	internal = new URLInternal;
	internal->href = urlstr;
}
URL::~URL()
{
	SAFE_DELETE(internal);
}

std::string URL::href() const
{
	return internal->href;
}

std::string URL::protocol() const
{
	std::string protocolstr;

	const char* proend = strstr(internal->href.c_str(), "://");
	if (proend != NULL)
	{
		protocolstr = std::string(internal->href.c_str(), proend - internal->href.c_str());
	}

	return protocolstr == "" ? "http" : protocolstr;
}

std::string URL::auhen() const
{
	const char* authenstart = internal->href.c_str();
	const char* proend = strstr(authenstart, "://");
	if (proend != NULL)
	{
		authenstart = proend + 3;
	}

	std::string autenstr;

	const char* authenend = strchr(authenstart, '@');
	if (authenend != NULL)
	{
		autenstr = std::string(authenstart, authenend - authenstart);
	}

	return autenstr;
}

//host.com:8080
std::string URL::host() const
{
	const char* authenstart = internal->href.c_str();
	const char* proend = strstr(authenstart, "://");
	if (proend != NULL)
	{
		authenstart = proend + 3;
	}
	const char* authenend = strchr(authenstart, '@');
	if (authenend != NULL)
	{
		authenstart = authenend + 1;
	}

	std::string autenstr = authenstart;

	const char* hostend = strchr(authenstart, '/');
	if (hostend != NULL)
	{
		autenstr = std::string(authenstart, hostend - authenstart);
	}

	return autenstr;
}
//host.com
std::string URL::hostname() const
{
	std::string tmp = host();
	std::string hoststr = tmp;

	const char* hostnameend = strchr(tmp.c_str(), ':');
	if (hostnameend != NULL)
	{
		hoststr = std::string(tmp.c_str(), hostnameend - tmp.c_str());
	}

	return hoststr;
}
//8080
uint32_t URL::port() const
{
	int httpport = 80;
	std::string tmp = host();
	const char* hostnameend = strchr(tmp.c_str(), ':');
	if (hostnameend != NULL)
	{
		httpport = atoi(hostnameend + 1);
	}

	return httpport;
}

///p/a/t/h?query=string#hash
std::string URL::path() const
{
	const char* authenstart = internal->href.c_str();
	const char* proend = strstr(authenstart, "://");
	if (proend != NULL)
	{
		authenstart = proend + 3;
	}
	const char* authenend = strchr(authenstart, '@');
	if (authenend != NULL)
	{
		authenstart = authenend + 1;
	}

	std::string pathstr = "/";

	const char* hostend = strchr(authenstart, '/');
	if (hostend != NULL)
	{
		pathstr = hostend;
	}

	return pathstr;
}
//p/a/t/h
std::string URL::pathname() const
{
	std::string tmp = path();
	std::string pathstr = tmp;

	const char* pathend = strchr(tmp.c_str(), '?');
	if (pathend != NULL)
	{
		pathstr = std::string(tmp.c_str(), pathend - tmp.c_str());
	}

	return pathstr;
}
//?query=string#hash
std::string URL::search() const
{
	std::string tmp = path();
	std::string serarchstr = "";

	const char* pathend = strchr(tmp.c_str(), '?');
	if (pathend != NULL)
	{
		serarchstr = pathend + 1;
	}

	return serarchstr;
}

//<query,string#assh>
std::map<std::string, std::string> URL::query() const
{
	std::map<std::string, std::string> valuemap;
	std::string searchstr = search();

	const char* searchstrptr = searchstr.c_str();
	while (*searchstrptr)
	{
		bool isend = false;
		const char* serachendflag = strchr(searchstrptr, '&');
		if (serachendflag == NULL)
		{
			isend = true;
		}
		std::string searchitem = serachendflag == NULL ? searchstrptr : std::string(searchstrptr, serachendflag - searchstrptr);

		std::string key, value;
		const char* keyend = strchr(searchitem.c_str(), '=');
		if (keyend != NULL)
		{
			key = std::string(searchitem.c_str(), keyend - searchitem.c_str());
			value = keyend + 1;
		}
		else
		{
			key = searchitem;
		}

		if (key != "")
		{
			valuemap[key] = value;
		}
		if (isend)
		{
			break;
		}
		searchstrptr = serachendflag + 1;
	}

	return valuemap;
}

#define HTTPTMP		"./_html_tmp/"

struct HTTPCache
{
	virtual ~HTTPCache() {}
	virtual int writeData(const char* buffer, int len) = 0;
	virtual int readData(char* buffer, int len) = 0;
	bool copyToFile(const std::string& dstfile)
	{
		FILE* dstfd = fopen(dstfile.c_str(), "wb+");
		if (dstfd == NULL)
		{
			return false;
		}
		char buffer[1024];
		while (1)
		{
			int readlen = readData(buffer, 1024);
			if (readlen <= 0)
			{
				break;
			}
			fwrite(buffer, 1, readlen, dstfd);
		}
		fclose(dstfd);

		return true;
	}
	bool appendFromFile(const std::string& dstfile)
	{
		FILE* dstfd = fopen(dstfile.c_str(), "rb");
		if (dstfd == NULL)
		{
			return false;
		}
		char buffer[1024];
		while (1)
		{
			int readlen = fread(buffer, 1, 1024, dstfd);
			if (readlen <= 0)
			{
				break;
			}
			writeData(buffer, readlen);
		}
		fclose(dstfd);

		return true;
	}
	virtual uint32_t totalsize() = 0;
};

struct FileCacheManager
{
	FileCacheManager() {}
	~FileCacheManager()
	{
		File::removeDirectory(HTTPTMP);
	}
	static FileCacheManager* instance()
	{
		static FileCacheManager manager;
		return &manager;
	}

	void openFile(const std::string& filename)
	{
		Guard locker(mutex);
		if (filenamemap.size() == 0 || !File::access(HTTPTMP, File::accessExist))
		{
			File::makeDirectory(HTTPTMP);
		}
		
		filenamemap.insert(filename);
	}
	void deleteFile(const std::string& filename)
	{
		removeExistFile(filename, 100);

		Guard locker(mutex);
		filenamemap.erase(filename);
		if (filenamemap.size() == 0)
		{
			File::removeDirectory(HTTPTMP);
		}
	}
	static void removeExistFile(const std::string& filename, int times = 100)
	{
		while (times-- > 0)
		{
			File::remove(filename.c_str());
			if (!File::access(filename.c_str(), File::accessExist))
			{
				break;
			}
			Thread::sleep(100);
		}
	}
private:
	Mutex	mutex;
	std::set<std::string> filenamemap;
};

struct fileCache:public HTTPCache
{
public:
	fileCache() :fd(NULL),datatotal(0), filename(std::string(HTTPTMP) + "_tmp_" + Guid::createGuid().getStringStream() + ".htm")
	{
		FileCacheManager::instance()->openFile(filename);
		fd = fopen(filename.c_str(), "wb+");
	}
	~fileCache()
	{
		if (fd != NULL)
		{
			fclose(fd);
		}
		
		FileCacheManager::instance()->deleteFile(filename);
	}
	
	int writeData(const char* buffer, int len)
	{
		if (buffer == NULL || len <= 0 || fd == NULL)
		{
			return 0;
		}
		int writelen = fwrite(buffer, 1, len, fd);
		if (writelen > 0)
		{
			datatotal += writelen;
		}

		return writelen;
	}
	int readData(char* buffer, int len)
	{
		if (buffer == NULL || len <= 0 || fd == NULL)
		{
			return 0;
		}
		int readlen = fread(buffer, 1, len, fd);
		return readlen;
	}	
	uint32_t totalsize() { return datatotal; }
private:
	FILE*		fd;
	uint32_t	datatotal;
	std::string filename;
};

struct memCache :public HTTPCache
{
	memCache():readpos(0),databuffer(""){}
	~memCache() {}
	int writeData(const char* buffer, int len)
	{
		if (buffer == NULL || len <= 0)
		{
			return 0;
		}

		databuffer += std::string(buffer, len);

		return len;
	}
	int readData(char* buffer, int len)
	{
		if (buffer == NULL || len <= 0 || (int)readpos >= (int)databuffer.size())
		{
			return 0;
		}

		int readlen = min((int)databuffer.size() - (int)readpos,len);
		memcpy(buffer, databuffer.c_str() + readpos, readlen);
		readpos += readlen;

		return readlen;
	}
	uint32_t totalsize() { return databuffer.size(); }
private:
	uint32_t		readpos;
	std::string		databuffer;
};

struct HTTPBuffer::HTTPBufferInternal
{
	shared_ptr<HTTPCache>		cachefile;

	HTTPBufferInternal(HTTPBufferCacheType type)
	{
		if (type == HTTPBufferCacheType_Mem)
		{
			cachefile = new memCache();
		}
		else
		{
			cachefile = new fileCache();
		}
	}
	~HTTPBufferInternal()
	{
		cachefile = NULL;
	}
};

HTTPBuffer::HTTPBuffer(HTTPBufferCacheType type)
{
	internal = new HTTPBufferInternal(type);
}
HTTPBuffer::~HTTPBuffer()
{
	SAFE_DELETE(internal);
}
int HTTPBuffer::read(char* buffer, int len) const
{
	if (internal == NULL || internal->cachefile == NULL || buffer == NULL || len <= 0)
	{
		return 0;
	}

	return internal->cachefile->readData(buffer, len);
}
int HTTPBuffer::read(std::string& data) const
{
	data = "";

	char buffer[1024];
	while (1)
	{
		int readlen = read(buffer, 1024);
		if (readlen <= 0)
		{
			break;
		}
		data.append(buffer, readlen);
	}

	return data.size();
}
std::string HTTPBuffer::read() const
{
	std::string data;
	read(data);

	return data;
}
uint64_t HTTPBuffer::totalSize() const
{
	if (internal == NULL || internal->cachefile == NULL)
	{
		return 0;
	}

	return internal->cachefile->totalsize();
}

bool HTTPBuffer::readToFile(const std::string& filename) const
{
	if (internal == NULL || internal->cachefile == NULL)
	{
		return false;
	}

	return internal->cachefile->copyToFile(filename);
}


bool HTTPBuffer::write(const char* buffer, int len)
{
	if (internal == NULL || internal->cachefile == NULL || buffer == NULL)
	{
		return false;
	}

	return internal->cachefile->writeData(buffer, len) == len;
}
bool HTTPBuffer::write(const std::string& data)
{
	return write(data.c_str(), data.length());
}
bool HTTPBuffer::write(const HTTPTemplate& temp)
{
	std::string tmpfile = temp.toValue();

	return writeFromFile(tmpfile,true);
}
bool HTTPBuffer::writeFromFile(const std::string& filename, bool deleteFile)
{
	if (internal == NULL || internal->cachefile == NULL)
	{
		if (deleteFile)
		{
			FileCacheManager::removeExistFile(filename);
		}
		return false;
	}
	bool ret = internal->cachefile->appendFromFile(filename);

	if (deleteFile)
	{
		FileCacheManager::removeExistFile(filename);
	}

	return ret;
}

}
}