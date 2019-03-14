//
//  Copyright (c)1998-2012,  Public Technology
//  All Rights Reserved.
//
//	Description:
//	$Id: File.cpp 251 2013-12-18 04:40:13Z  $
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
};


///////////////////////////////////////////////////////////////////////////////
///////   File implement

File::File()
{
	internal = new(std::nothrow) FileInternal;
	internal->file = NULL;
}

File::~File()
{
	if(internal->file)
	{
		close();
	}
	delete internal;
}


bool File::open(const std::string& pFileName, uint32_t dwFlags)
{
	if(internal->file)
	{
		close();
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

	internal->file = ::fopen(pFileName.c_str(), mode);


	if(internal->file == NULL)
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
}

long File::read(void* pBuffer, size_t dwCount)
{
	if(internal->file == NULL)
	{
		return -1;
	}

	size_t len = ::fread(pBuffer, 1, dwCount, internal->file);


	return len;
}
long File::write(void *pBuffer, size_t dwCount)
{
	if(internal->file == NULL)
	{
		return -1;
	}

	size_t len = ::fwrite(pBuffer, 1, dwCount, internal->file);


	return len;
}

void File::flush()
{
	if (internal->file == NULL)
	{
		return;
	}
	::fflush(internal->file);
	return;

}

uint64_t File::seek(int64_t lOff, SeekPosition nFrom)
{
	if (internal->file == NULL)
	{
		return 0;
	}
	int origin = 0;
	
	switch(nFrom){
	case begin:
		origin = SEEK_SET;
		break;
	case current:
		origin = SEEK_CUR;
		break;
	case end:
		origin = SEEK_END;
		break;
	}

#ifdef WIN32
	if(::_fseeki64(internal->file, lOff, origin) >= 0)
	{
		int64_t pos = ::_ftelli64(internal->file);

		return pos;
	};
#else
	if(::fseeko(internal->file, lOff, origin) >= 0)
	{
		int64_t pos = ::ftello(internal->file);

		return pos;
	};
#endif // WIN32

	return 0;
}

uint64_t File::getPosition()
{
#ifdef WIN32
	int64_t pos = ::_ftelli64(internal->file);

	return pos;
#else
	int64_t pos = ::ftello(internal->file);

	return pos;
#endif // WIN32
}

char* File::gets(char* s, size_t size)
{
	if(!internal->file)
	{
		return 0;
	}

	char* p = ::fgets(s, size, internal->file);


	return p;
}


long File::puts(const char* s)
{
	if(!internal->file)
	{
		return 0;
	}

	long ret = ::fputs(s, internal->file);

	return ret;
}

bool File::isOpen()
{
	return (internal->file != 0);
}

std::string File::load(const std::string& pFileName)
{
	uint64_t filelen = getLength(pFileName);
	if (filelen == 0)
	{
		return "";
	}
	char* buffer = new (std::nothrow)char[(size_t)filelen];
	if (buffer == NULL)
	{
		return "";
	}

	FILE* fd = fopen(pFileName.c_str(), "rb");
	if (fd == NULL)
	{
		SAFE_DELETEARRAY(buffer);
		return "";
	}

	size_t readlen = fread(buffer, 1, (size_t)filelen, fd);
	if (readlen != (size_t)filelen)
	{
		SAFE_DELETEARRAY(buffer);
		fclose(fd);
		return "";
	}
	fclose(fd);
	std::string outputstr(buffer,readlen);
	SAFE_DELETEARRAY(buffer);

	return move(outputstr);
}

bool File::save(const std::string& pFileName, const std::string& data)
{
	FILE* fd = fopen(pFileName.c_str(), "wb+");
	if (fd == NULL) return false;

	fwrite(data.c_str(), 1, data.length(), fd);

	fclose(fd);

	return true;
}

uint64_t File::getLength(const std::string& pFileName)
{
	FileInfo info;

	if (!stat(pFileName, info))
	{
		return 0;
	}

	return info.size;
}

bool File::rename(const std::string& oldName, const std::string&  newName)
{
	return (::rename(oldName.c_str(), newName.c_str()) == 0);
}

std::string File::getExcutableFileName()
{
	char s[256] = { 0 };
	size_t ret = -1;
#ifdef WIN32
	ret = GetModuleFileName(NULL, s, 255);
#else
	ret = readlink("/proc/self/exe", s, 255);
#endif // WIN32

	if (ret <0 || ret >= 256)
	{
		return "";
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

	return s;
}
std::string File::getExcutableFileFullPath()
{
	char s[256] = { 0 };
	size_t ret = -1;
#ifdef WIN32
	ret = GetModuleFileName(NULL, s, 255);
#else
	ret = readlink("/proc/self/exe", s, 255);
#endif // WIN32
	
	if (ret <0 || ret >= 256)
	{
		return "";
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

	return s;
}
std::string File::getCurrentDirectory()
{
	char buffer[256] = { 0 };
#ifdef WIN32
	GetCurrentDirectory(255, buffer);
#else
	if(getcwd(buffer, 255) == NULL) return "";
#endif

	return buffer;
}

bool File::setCurrentDirectory(const std::string& path)
{
#ifdef WIN32
	SetCurrentDirectory(path.c_str());
#else
	if(chdir(path.c_str()) != 0) return false;
#endif

	return true;
}
bool File::remove(const std::string& fileName)
{
	return (::remove(fileName.c_str()) == 0);
}
bool File::makeDirectory(const std::string& dirName)
{
#ifndef WIN32
	if(!File::access(dirName.c_str(), File::accessExist))
	{
		char tmp[256];
		snprintf_x(tmp,255,"mkdir -pv \"%s\"",dirName.c_str()); //创建多重目录

		SystemCall(tmp);

		return File::access(dirName.c_str(), File::accessExist);
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

bool File::removeDirectory(const std::string& dirName)
{
#ifndef WIN32
	if(File::access(dirName.c_str(), File::accessExist))
	{
		char tmp[256];
		snprintf_x(tmp,255,"rm -rdf \"%s\"",dirName.c_str()); //删除非空目录

		SystemCall(tmp);

		return !File::access(dirName.c_str(), File::accessExist);
	}

	return true;
#else
	DWORD attr = GetFileAttributes(dirName.c_str());
	if(attr != -1 && attr & FILE_ATTRIBUTE_DIRECTORY)
	{
		char cmdstr[256];
		snprintf_x(cmdstr,255,"rd /Q /S \"%s\"", dirName.c_str());
		SystemCall(cmdstr);

		attr = GetFileAttributes(dirName.c_str());
		return (attr == -1 || !(attr & FILE_ATTRIBUTE_DIRECTORY));
	}

	return true;
#endif
}

bool File::statFS(const std::string& path,uint64_t& userFreeBytes,uint64_t& totalBytes,uint64_t& totalFreeBytes)
{
#ifdef WIN32
	GetDiskFreeSpaceEx((LPCTSTR)path.c_str(),
		(PULARGE_INTEGER)&userFreeBytes,
		(PULARGE_INTEGER)&totalBytes,
		(PULARGE_INTEGER)&totalFreeBytes);
#elif defined(__GNUC__)
	userFreeBytes = 0;
	totalBytes = 0;
	totalFreeBytes = 0;

	struct statfs info = {0};
	if (::statfs(path.c_str(), &info) == 0)
	{
		userFreeBytes = (uint64_t)(info.f_bsize) * info.f_bavail;
		totalBytes = (uint64_t)(info.f_bsize) * info.f_blocks;
		totalFreeBytes = (uint64_t)(info.f_bsize) * info.f_bfree;
	}
#endif
	return true;
}
		
bool File::access(const std::string& path, AccessMode mode)
{

#ifndef WIN32
	return (::access(path.c_str(), (int)mode) == 0);
#else
	return (_access(path.c_str(), (int)mode) == 0);
#endif
	
}

bool File::stat(const std::string& path, FileInfo& info)
{

#ifdef WIN32
	struct _stati64 s = {0};
	int ret = _stati64(path.c_str(), &s);
#else
	struct stat s = {0};
	int ret = ::stat(path.c_str(), &s);
#endif
	if(ret != 0)
	{
		return false;
	}

	info.name = path;
	info.attrib = s.st_mode;
	info.timeWrite = s.st_mtime;
	info.timeAccess = s.st_atime;
	info.timeCreate = s.st_ctime;
	info.size = s.st_size;

	return true;
}

bool File::copy(const std::string& srcfile, const std::string& tofile)
{
	FILE* srcfd = fopen(srcfile.c_str(), "rb");
	FILE* tofd = fopen(tofile.c_str(), "wb+");

	if (srcfd == NULL || tofd == NULL)
	{
		if (srcfd != NULL) fclose(srcfd);
		if (tofd != NULL) fclose(tofd);

		return false;
	}

	while (1)
	{
		char buffer[1024];
		int readlen = fread(buffer, 1, 1024, srcfd);
		if (readlen <= 0) break;

		if ((int)fwrite(buffer, 1, readlen, tofd) != readlen)
		{
			if (srcfd != NULL) fclose(srcfd);
			if (tofd != NULL) fclose(tofd);

			return false;
		}
	}

	fclose(srcfd);
	fclose(tofd);

	return true;
}

string File::absPath(const string& path)
{
#ifdef _WIN32
#define max_path 4096
	char resolved_path[max_path] = { 0 };
	_fullpath(resolved_path, path.c_str(), max_path);
#else
	//linux release有个坑，需要大点的空间
#define max_path 40960
	char resolved_path[max_path] = { 0 };
	realpath(path.c_str(), resolved_path);
#endif
	return string(resolved_path);
}

} // namespace Base
} // namespace Public



