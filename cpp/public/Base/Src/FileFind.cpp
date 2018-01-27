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
#endif

#ifdef  __linux__
#include <sys/statfs.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#endif
#include "Base/File.h"
#include "Base/FileFind.h"
#include "Base/Mutex.h"
#include "Base/String.h"
#include "Base/System.h"


#include <string>

namespace Public {
namespace Base {

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
	std::string searchfile;
	std::string path;			///< 查找路径。
};


FileFind::FileFind(const std::string& searchfile)
{
	internal = new(std::nothrow) FileFindInternal;
	internal->handle = -1;
	internal->searchfile = searchfile;

	const char* p = searchfile.c_str() + searchfile.length();
	while (*p != '/' && *p != '\\' && p != searchfile.c_str())
	{
		p--;
	}
	internal->path = std::string(searchfile.c_str(), p - searchfile.c_str() + 1);
}

FileFind::~FileFind()
{
	if (internal->handle >= 0)
	{
		_findclose((long)internal->handle);
		internal->handle = -1;
	}
	delete internal;
}

shared_ptr<FileFind::FileFindInfo> FileFind::find()
{
	_finddata_t finddata = { 0 };

	if (internal->handle == -1)
	{
		internal->handle = _findfirst(internal->searchfile.c_str(), &finddata);
		if (internal->handle <= 0)
		{
			return shared_ptr<FileFind::FileFindInfo>();
		}
	}
	else
	{
		long ret = _findnext((long)internal->handle, &finddata);
		if (ret < 0)
		{
			return shared_ptr<FileFind::FileFindInfo>();
		}
	}

	return make_shared<FileFind::FileFindInfo>(new FileFindInfo(&finddata));
}

struct FileFind::FileFindInfo::FileFindInfoInternal
{
	_finddata_t fileInfo;
	std::string path;
};

FileFind::FileFindInfo::FileFindInfo(const std::string& path, void* findinfo)
{
	internal = new FileFindInfoInternal;
	internal->fileInfo = *(_finddata_t*)findinfo;
	internal->path = path;
}
FileFind::FileFindInfo::~FileFindInfo()
{
	SAFE_DELETE(internal);
}

size_t FileFind::FileFindInfo::getLength()
{
	return (unsigned int)internal->fileInfo.size;
}

std::string FileFind::FileFindInfo::getFileName()
{
	return internal->fileInfo.name;
}

std::string FileFind::FileFindInfo::getFilePath()
{
	return internal->path + internal->fileInfo.name;
}

bool FileFind::FileFindInfo::isReadOnly()
{
	return ((internal->fileInfo.attrib & File::readOnly) != 0);
}

bool FileFind::FileFindInfo::isDirectory()
{
	return ((internal->fileInfo.attrib & File::directory) != 0);
}


bool FileFind::FileFindInfo::isHidden()
{
	return ((internal->fileInfo.attrib & File::hidden) != 0);
}

bool FileFind::FileFindInfo::isNormal()
{
	return (internal->fileInfo.attrib == File::normal);
}


} // namespace Base
} // namespace Public



