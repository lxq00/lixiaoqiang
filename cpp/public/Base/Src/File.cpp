//
//  Copyright (c)1998-2012, Chongqing Public Technology
//  All Rights Reserved.
//
//	Description:
//	$Id: File.cpp 251 2013-12-18 04:40:13Z lixiaoqiang $
//


#include <assert.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <map>

#ifdef WIN32
#include <direct.h>
#include <Windows.h>
#undef RemoveDirectory
#endif

#ifdef  __linux__
#include <sys/statfs.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#endif
#include "Base/File.h"
#include "Base/Mutex.h"
#include "Base/String.h"
#include "Base/System.h"
#ifndef WIN32
#define _O_BINARY 0
#define _O_RDONLY O_RDONLY
#define _O_WRONLY O_WRONLY
#define _O_CREAT O_CREAT
#define _O_TRUNC O_TRUNC
#define _O_RDWR O_RDWR
#define _S_IREAD (S_IRUSR | S_IRGRP | S_IROTH)
#define _S_IWRITE (S_IWUSR | S_IWGRP | S_IWOTH)
#define _open open
#define _access access
#define _stat stat
#endif

#include <string>

namespace Public {
namespace Base {

struct File::FileInternal
{	
	FILE *file;				
	unsigned char *buffer;	///< 文件数据缓冲，Load和UnLoad接口使用。
	unsigned long long length;		///< 文件长度。
	unsigned long long position;		///< 文件读写指针。
};


///////////////////////////////////////////////////////////////////////////////
///////   File implement

File::File()
{
	internal = new(std::nothrow) FileInternal;
	internal->file = NULL;
	internal->buffer = NULL;
	internal->length = 0;
	internal->position = 0;
}

File::~File()
{
	if(internal->file)
	{
		close();
	}
	if(internal->buffer)
	{
		unload();
	}
	delete internal;
}


bool File::open(const char* pFileName, uint32_t dwFlags)
{
	if(internal->file)
	{
		close();
	}
	if(internal->buffer)
	{
		unload();
	}

	const char* mode = "";
	switch (dwFlags & 0xf)
	{
	case modeRead:
		mode = "rb";
		break;
	case modeWrite:
		if (dwFlags & modeNoTruncate)
		{
			mode = "ab";
		}
		else
		{
			mode = "wb";
		}
		break;
	case modeReadWrite:
		if(dwFlags & modeCreate)
		{
			mode = "wb+";
		}
		else
		{
			mode = "rb+";
		}
	}

	internal->file = ::fopen(pFileName, mode);


	if(!internal->file)
	{
		return false;
	}

	return true;
}

void File::close()
{
	if(!internal->file)
	{
		return;
	}
	::fclose((FILE *)internal->file);

	internal->file = NULL;
	internal->length = 0;
	internal->buffer = NULL;
	internal->position = 0;
}

uint8_t* File::load(const char* pFileName)
{
	assert(!internal->buffer);
	if(open(pFileName, modeRead | modeNoTruncate))
	{
		if (internal->length == 0) {
			internal->length = seek(0, end);
			seek(0,begin);
		}
		if(internal->length)
		{
			internal->buffer = new(std::nothrow) unsigned char[(unsigned int)internal->length];
			if(internal->buffer)
			{
				seek(0, begin);
				unsigned int ret = read(internal->buffer, (size_t)internal->length);
				assert(ret == internal->length);
				(void)ret;	// 消除变量未使用编译警告
			}
		}
	}
	return internal->buffer;
}

void File::unload()
{
	if(internal->buffer)
	{
		close();

		delete []internal->buffer;
		internal->buffer = NULL;
	}
}
long File::read(void* pBuffer, size_t dwCount)
{
	if(!internal->file)
	{
		return -1;
	}

	size_t len = ::fread(pBuffer, 1, dwCount, internal->file);

	internal->position += len;

	return len;
}
long File::write(void *pBuffer, size_t dwCount)
{
	if(!internal->file)
	{
		return -1;
	}

	size_t len = ::fwrite(pBuffer, 1, dwCount, internal->file);

	internal->position += len;

	if(internal->length < internal->position)
	{
		internal->length = internal->position;
	}

	return len;
}

void File::flush()
{
	if(!internal->file)
	{
		return;
	}
	::fflush(internal->file);
	return;

}

uint64_t File::seek(int64_t lOff, SeekPosition nFrom)
{
	if(!internal->file)
	{
		return 0;
	}
	int origin = 0;
	uint64_t position = 0;

	switch(nFrom){
	case begin:
		position = lOff;
		origin = SEEK_SET;
		break;
	case current:
		position = internal->position + lOff;
		origin = SEEK_CUR;
		break;
	case end:
		position = internal->length + lOff;
		origin = SEEK_END;
		break;
	}

	// 如果文件偏移一样，直接返回，提高效率
	if(position == internal->position&& origin != SEEK_END)
	{
		return position;
	}


#ifdef WIN32
	if(::_fseeki64(internal->file, lOff, origin) >= 0)
	{
		int64_t pos = ::_ftelli64(internal->file);
		internal->position = pos < 0 ? 0 : pos;

		return internal->position;
	};
#else
	if(::fseeko(internal->file, lOff, origin) >= 0)
	{
		int64_t pos = ::ftello(internal->file);
		internal->position = pos < 0 ? 0 : pos;

		return internal->position;
	};
#endif // WIN32

	return 0;
}

uint64_t File::getPosition()
{
	if(!internal->file)
	{
		return 0;
	}

	return internal->position;
}

uint64_t File::getLength()
{
	if(!internal->file)
	{
		return 0;
	}
	if (internal->length == 0) {
		uint64_t pos = getPosition();
		internal->length = seek(0, end);
		seek(pos, begin);
	}
	return internal->length;
}

char* File::gets(char* s, size_t size)
{
	if(!internal->file)
	{
		return 0;
	}

	char* p = ::fgets(s, size, internal->file);

	if(p)
	{
		internal->position += strlen(p);
	}

	return p;
}


long File::puts(const char* s)
{
	if(!internal->file)
	{
		return 0;
	}

	long ret = ::fputs(s, internal->file);
	if(ret != EOF)
	{
		internal->position += strlen(s);
	}

	return ret;
}

bool File::isOpen()
{
	return (internal->file != 0);
}
bool File::rename(const char* oldName, const char* newName)
{
	return (::rename(oldName, newName) == 0);
}

size_t File::getExcutableFileName(char* s, size_t size)
{
	size_t ret = -1;
#ifdef WIN32
	ret = GetModuleFileName(NULL, s, size-1);
#else
	ret = readlink("/proc/self/exe", s, size-1);
#endif // WIN32

	if (ret <0)
	{
		return ret;
	}
	if (ret >= size-1)
	{
		return -1;
	}
	s[ret] = 0;
	for (int i = ret; i >= 0; i--)
	{
		if (s[i] == PATH_SEPARATOR)
		{
			ret = ret - i - 1;
			::memcpy(s, s + i + 1, ret);
			s[ret] = 0;
			break;
		}
	}

	return ret;
}
size_t File::getExcutableFileFullPath(char* s, size_t size)
{
	size_t ret = -1;
	s[0] = 0;
#ifdef WIN32
	ret = GetModuleFileName(NULL, s, size-1);
#else
	ret = readlink("/proc/self/exe", s, size-1);
#endif // WIN32
	
	if (ret <0)
	{
		return ret;
	}
	if (ret >= size-1)
	{
		return -1;
	}
	s[ret] = 0;
	for (int i = ret; i >= 0; i--)
	{
		if (s[i] == PATH_SEPARATOR)
		{
			s[i] = 0;
			ret = i;
			break;
		}
	}

	return ret;
}
bool File::remove(const char* fileName)
{
	return (::remove(fileName) == 0);
}
bool File::makeDirectory(const char* dirName)
{
#ifndef WIN32
	if(!File::access(dirName, File::accessExist))
	{
		char tmp[256];
		snprintf_x(tmp,255,"mkdir -pv \"%s\"",dirName); //创建多重目录

		SystemCall(tmp);

		return File::access(dirName, File::accessExist);
	}
	return true;
#else
	bool creatret = true;
	std::string dirpath = dirName;
	char* tmp = (char*)dirpath.c_str();
	while(*tmp)
	{
		if(*(tmp+1) == '\\' || *(tmp+1) == '/' || *(tmp+1) == '\0') //循环创建多重目录
		{
			char tmpchr = *(tmp + 1);
			*(tmp+1) = 0;
			DWORD attr = GetFileAttributes(dirpath.c_str());
			if(attr == -1 || !(attr & FILE_ATTRIBUTE_DIRECTORY))
			{
				int ret = _mkdir(dirpath.c_str());
				if(ret != 0)
				{
					int error = GetLastError();
					int b = 0;
					creatret = false;
					break;
				}
			}
			*(tmp+1) = tmpchr;
		}
		tmp ++;
	}
	return creatret;
#endif

}

bool File::removeDirectory(const char* dirName)
{
#ifndef WIN32
	if(File::access(dirName, File::accessExist))
	{
		char tmp[256];
		snprintf_x(tmp,255,"rm -rdf \"%s\"",dirName); //删除非空目录

		SystemCall(tmp);

		return !File::access(dirName, File::accessExist);
	}

	return true;
#else
	DWORD attr = GetFileAttributes(dirName);
	if(attr != -1 && attr & FILE_ATTRIBUTE_DIRECTORY)
	{
		char cmdstr[256];
		snprintf_x(cmdstr,255,"rd /Q /S \"%s\"", dirName);
		SystemCall(cmdstr);

		attr = GetFileAttributes(dirName);
		return (attr == -1 || !(attr & FILE_ATTRIBUTE_DIRECTORY));
	}

	return true;
#endif
}

bool File::statFS(const char* path,
		uint64_t& userFreeBytes,
		uint64_t& totalBytes,
		uint64_t& totalFreeBytes)
{
#ifdef WIN32
	GetDiskFreeSpaceEx((LPCTSTR)path,
		(PULARGE_INTEGER)&userFreeBytes,
		(PULARGE_INTEGER)&totalBytes,
		(PULARGE_INTEGER)&totalFreeBytes);
#elif defined(__GNUC__)
	userFreeBytes = 0;
	totalBytes = 0;
	totalFreeBytes = 0;

	struct statfs info = {0};
	if (::statfs(path, &info) == 0)
	{
		userFreeBytes = (uint64_t)(info.f_bsize) * info.f_bavail;
		totalBytes = (uint64_t)(info.f_bsize) * info.f_blocks;
		totalFreeBytes = (uint64_t)(info.f_bsize) * info.f_bfree;
	}
#endif
	return true;
}
		
bool File::access(const char* path, int mode)
{

#ifndef WIN32
	return (::access(path, mode) == 0);
#else
	return (_access(path, mode) == 0);
#endif
	
}

bool File::stat(const char* path, FileInfo& info)
{

#ifdef WIN32
	struct _stat s = {0};
	int ret = _stat(path, &s);
#else
	struct stat s = {0};
	int ret = ::stat(path, &s);
#endif
	if(ret != 0)
	{
		return false;
	}

	strncpy(info.name, FileInfo::maxPathSize - 1,path, strlen(path));
	info.attrib = s.st_mode;
	info.timeWrite = s.st_mtime;
	info.timeAccess = s.st_atime;
	info.timeCreate = s.st_ctime;
	info.size = s.st_size;

	return true;
}

File::File(File const& fs)
{
	internal = new(std::nothrow) FileInternal;
	internal->file = fs.internal->file;
	internal->buffer = fs.internal->buffer;
	internal->length = fs.internal->length;
	internal->position = fs.internal->position;
}

File& File::operator=(File const& fs)
{
	if (this == &fs)
		return *this;
	if (internal->file)
	{
	 	::fclose(internal->file);
	}
	if (internal->buffer)
	{
		unload();
	}
	internal->file = fs.internal->file;
	internal->buffer = fs.internal->buffer;
	internal->length = fs.internal->length;
	internal->position = fs.internal->position;
	return *this;
}
#ifndef WIN32

typedef uint32_t _fsize_t;
#define _MAX_FNAME 	256
struct _finddata_t
{
	unsigned int 	attrib;
	time_t 			time_create;
	time_t			time_access;
	time_t			time_write;
	_fsize_t		size;
	char 			name[_MAX_FNAME];
};

struct linuxfilestate
{
	std::string 	name;
	uint32_t		size;
	int 			type;
};

class LinuxFinderFile
{
public:	
	LinuxFinderFile(const std::string& name):readfileindex(0)
	{
		find(name);
	}
	~LinuxFinderFile(){}
	

	bool get(_finddata_t& file)
	{
		std::map<int,linuxfilestate>::iterator iter = linuxfilestateList.find(readfileindex);
		if(iter == linuxfilestateList.end())
		{
			return false;
		}

		strncpy(file.name,_MAX_FNAME - 1,iter->second.name.c_str(),iter->second.name.length());
		file.size = iter->second.size;
		file.attrib = File::normal ;
		if(!iter->second.type)
		{
			file.attrib &= File::directory;
		}

		readfileindex ++;

		return true;
	}
private:	
	void find(const std::string& name)
	{
//		{
//			struct stat buf;
//			int ret = stat(name.c_str(),&buf);
//			if(ret < 0)
//			{
//				return;
//			}
//		}

		std::string filepath;
		std::string filename;

		getFilePathAndNameByAddr(name,filepath,filename);
		
		char tmp[512];
		sprintf(tmp,"find %s 2>/dev/null",name.c_str());

		FILE* fd = popen(tmp,"r");
		if(fd == NULL)
		{
			return;
		}

		while(fgets(tmp,511,fd) != NULL)
		{
			/// 去掉回车符
			int filelen = strlen(tmp);
			while(filelen > 0 && (tmp[filelen-1] == '\n' || tmp[filelen-1] == '\r'))
			{
				tmp[filelen-1] = 0;
				filelen --;
			}

			addfiletolist(tmp,filepath);
		}

		fclose(fd);
	}
	void addfiletolist(const std::string& name,const std::string& findfiledir)
	{
		linuxfilestate fileitem;
		struct stat filestat;

		if(stat(name.c_str(),&filestat) != 0)
		{
			return;
		}

		fileitem.type = !S_ISDIR(filestat.st_mode);
		fileitem.size = filestat.st_size;

		if(name.size() < findfiledir.size() || memcmp(name.c_str(),findfiledir.c_str(),findfiledir.size()) != 0)
		{
			return;
		}
		fileitem.name = name.c_str() + findfiledir.size() + 1;
		
		linuxfilestateList.insert(std::pair<int,linuxfilestate>(linuxfilestateList.size(),fileitem));
	}
	void getFilePathAndNameByAddr(const std::string& name, std::string &filepath, std::string &filename)
	{
		filename = name;

		const char* dirtmp = strrchr(name.c_str(),PATH_SEPARATOR);
		if(dirtmp != NULL)
		{
			filepath = string(name.c_str(),dirtmp - name.c_str());
			filename = dirtmp + 1;
		}
	}
private:	
	std::map<int,linuxfilestate> 	linuxfilestateList;
	int 							readfileindex;
};


static long _findfirst(const std::string& fileName,_finddata_t* finddata)
{
	if(finddata == NULL)
	{
		return 0;
	}

	LinuxFinderFile* file = new(std::nothrow) LinuxFinderFile(fileName);
	if(file == NULL)
	{
		return 0;
	}

	if(!file->get(*finddata))
	{
		delete file;
		return 0;
	}

	return (long)file;
}

static long _findnext(long handle,_finddata_t* finddata)
{
	LinuxFinderFile* file = (LinuxFinderFile*)handle;
	if(file == NULL || finddata == NULL)
	{
		return -1;
	}

	bool ret = file->get(*finddata);

	return ret ? 0:-1;
}


static void _findclose(long handle)
{
	if(handle <= 0)
	{
		return;
	}
	
	LinuxFinderFile* file = (LinuxFinderFile*)handle;
	
	delete file;
	file = NULL;
}

#endif

struct FileFind::FileFindInternal
{
	long handle;
	FileInfo fileInfo;
	std::string path;			///< 查找路径。
};

///////////////////////////////////////////////////////////////////////////////
///////   FileFind implement
FileFind::FileFind(FileFind const& fs)
{
	internal = new(std::nothrow) FileFindInternal;
	internal->handle = fs.internal->handle;
	memcpy(&internal->fileInfo, &fs.internal->fileInfo, sizeof(FileInfo));
}
FileFind& FileFind::operator=(FileFind const&fs)
{
	if (this == &fs)
		return *this;
	close();
	internal->handle = fs.internal->handle;
	memcpy(&internal->fileInfo, &fs.internal->fileInfo, sizeof(FileInfo));
	return *this;
}

FileFind::FileFind()
{
	internal = new(std::nothrow) FileFindInternal;
	internal->handle = -1;
	memset(&internal->fileInfo, 0, sizeof(FileInfo));
}

FileFind::~FileFind()
{
	close();
	delete internal;
}

bool FileFind::findFile(const char* fileName)
{
	close();
	const char* p = fileName + strlen(fileName);
	while(*p != '/' && *p != '\\' && p != fileName)
	{
		p--;
	}
	strncpy(internal->fileInfo.name, FileInfo::maxPathSize - 1,p + 1, strlen(p + 1));
	internal->path.assign(fileName, p - fileName + 1);
	
	_finddata_t finddata = {0};
	long ret = _findfirst(fileName, &finddata);
	if(ret <= 0)
	{
		return false;
	}
	internal->fileInfo.attrib = finddata.attrib;
	internal->fileInfo.size = finddata.size;
	memcpy( internal->fileInfo.name, finddata.name,  sizeof( internal->fileInfo.name) - 1);
	internal->fileInfo.name[sizeof(internal->fileInfo.name) - 1] = '\0';
	internal->fileInfo.timeWrite = finddata.time_write;
	internal->fileInfo.timeAccess = finddata.time_access;
	internal->fileInfo.timeCreate = finddata.time_create;
	internal->handle = ret;
	return true;
}

bool FileFind::findNextFile()
{
	_finddata_t finddata = {0};
	long ret = _findnext((long)internal->handle, &finddata);
	if(ret < 0)
	{
		return false;
	}
	internal->fileInfo.attrib = finddata.attrib;
	internal->fileInfo.size = finddata.size;
	memcpy(internal->fileInfo.name, finddata.name, sizeof(internal->fileInfo.name) - 1);
	internal->fileInfo.name[ sizeof(internal->fileInfo.name) - 1] = '\0';
	internal->fileInfo.timeWrite = finddata.time_write;
	internal->fileInfo.timeAccess = finddata.time_access;
	internal->fileInfo.timeCreate = finddata.time_create;

	return true;
}

void FileFind::close()
{
	if(internal->handle < 0)
	{
		return;
	}
	_findclose((long)internal->handle);
	internal->handle = -1;
}

size_t FileFind::getLength()
{
	return (unsigned int)internal->fileInfo.size;
}

std::string FileFind::getFileName()
{
	return internal->fileInfo.name;
}

std::string FileFind::getFilePath()
{
	return internal->path + internal->fileInfo.name;
}

bool FileFind::isReadOnly()
{
	return ((internal->fileInfo.attrib & File::readOnly) != 0);
}

bool FileFind::isDirectory()
{
	return ((internal->fileInfo.attrib & File::directory) != 0);
}


bool FileFind::isHidden()
{
	return ((internal->fileInfo.attrib & File::hidden) != 0);
}

bool FileFind::isNormal()
{
	return (internal->fileInfo.attrib == File::normal);
}


} // namespace Base
} // namespace Public



