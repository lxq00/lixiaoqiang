#include "Base/Directory.h"
#include "Base/BaseTemplate.h"
#include "Base/File.h"
#include "Base/String.h"
#include "Base/ReadWriteMutex.h"
#include "Base/Guard.h"
#include "Base/FileFind.h"
#ifdef WIN32
#include <windows.h>
#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/vfs.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#endif
#include <list>
#include <map>

namespace Public {
namespace Base {
#ifdef WIN32
struct Directory::DirectoryInternal
{
	FileFind	find;
	std::string	path;

	DirectoryInternal(const std::string& _path):find(_path+ PATH_SEPARATOR +"*"),path(_path){}
};

Directory::Directory(const std::string& path)
{
	internal = new DirectoryInternal(path);
}
Directory::~Directory()
{
	SAFE_DELETE(internal);
}

bool Directory::read(Dirent& dir)
{
	while(true)
	{
		shared_ptr<FileFind::FileFindInfo> info = internal->find.find();
		if (info == NULL)
		{
			return false;
		}
		if (info->getFileName() == "." || info->getFileName() == "..")
		{
			continue;
		}
		dir.Type = info->isDirectory() ? Dirent::DirentType_Dir : Dirent::DirentType_File;
		dir.Name = info->getFileName();
		dir.CompletePath = info->getFilePath();
		dir.Path = internal->path;
		const char* tmp = strrchr(dir.Name.c_str(), '.');
		if (tmp != NULL)
		{
			dir.SuffixName = tmp + 1;
		}

		FileInfo finfo;
		File::stat(dir.CompletePath, finfo);
		dir.CreatTime = finfo.timeCreate;
		dir.FileSize = finfo.size;

		return true;
	}

	return false;
}

#else
struct Directory::DirectoryInternal
{
	DIR* dir;
	std::string	path;

	DirectoryInternal(const std::string& _path) :dir(NULL), path(_path) 
	{
		dir = opendir(path.c_str());
	}
	~DirectoryInternal()
	{
		if (dir != NULL)
		{
			closedir(dir);
			dir = NULL;
		}
	}
};

Directory::Directory(const std::string& path)
{
	internal = new DirectoryInternal(path);
}
Directory::~Directory()
{
	SAFE_DELETE(internal);
}

bool Directory::read(Dirent& dir)
{
	if (internal->dir == NULL) return false;
	while (true)
	{
		struct dirent *dent = readdir(internal->dir);
		if (dent == NULL) return false;


		if (strcmp(dent->d_name,".") == 0 || strcmp(dent->d_name, "..") == 0)
		{
			continue;
		}

		dir.Type = (dent->d_type & DT_DIR) ? Dirent::DirentType_Dir : Dirent::DirentType_File;
		dir.Name = dent->d_name;
		dir.Path = internal->path;
		dir.CompletePath = dir.Path  + PATH_SEPARATOR + dir.Name;
		const char* tmp = strrchr(dir.Name.c_str(), '.');
		if (tmp != NULL)
		{
			dir.SuffixName = tmp + 1;
		}

		FileInfo finfo;
		File::stat(dir.CompletePath, finfo);
		dir.CreatTime = finfo.timeCreate;
		dir.FileSize = finfo.size;

		return true;
	}

	return false;
}
#endif
//
//struct OrderByItem:public Directory::Dirent
//{
//	OrderByItem(const Directory::Dirent& dirent,FileBrowser::OrderMode _mode):mode(_mode)
//	{
//		Name = dirent.Name;
//		CompletePath = dirent.CompletePath;
//		Path = dirent.Path;
//		SuffixName = dirent.SuffixName;
//		FileSize = dirent.FileSize;
//		CreatTime = dirent.CreatTime;
//		Type= dirent.Type;
//	}
//	Directory::Dirent getDirent()
//	{
//		Directory::Dirent dirent;
//
//		dirent.Name = Name;
//		dirent.CompletePath = CompletePath;
//		dirent.Path = Path;
//		dirent.SuffixName = SuffixName;
//		dirent.FileSize = FileSize;
//		dirent.CreatTime = CreatTime;
//		dirent.Type= Type;
//
//		return dirent;
//	}
//
//	FileBrowser::OrderMode mode;
//};
//
//struct OrderByNameItem:public OrderByItem
//{
//	OrderByNameItem(const Directory::Dirent& dirent,FileBrowser::OrderMode mode):OrderByItem(dirent,mode)
//	{
//	}
//	
//	bool operator <(const OrderByNameItem& item) const
//	{
//		if(mode == FileBrowser::OrderMode_Asc)
//		{
//			return strcmp(Name.c_str(),item.Name.c_str()) < 0;
//		}
//		else
//		{
//			return strcmp(Name.c_str(),item.Name.c_str()) > 0;
//		}
//	}	
//};
//struct OrderBySizeItem:public OrderByItem
//{
//	OrderBySizeItem(const Directory::Dirent& dirent,FileBrowser::OrderMode mode):OrderByItem(dirent,mode)
//	{
//	}
//
//	bool operator <(const OrderBySizeItem& item) const
//	{
//		if(mode == FileBrowser::OrderMode_Asc)
//		{
//			return FileSize < item.FileSize;
//		}
//		else
//		{
//			return FileSize > item.FileSize;
//		}
//	}
//};
//struct OrderByCteatTimeItem:public OrderByItem
//{
//	OrderByCteatTimeItem(const Directory::Dirent& dirent,FileBrowser::OrderMode mode):OrderByItem(dirent,mode)
//	{
//	}
//
//	bool operator <(const OrderByCteatTimeItem& item) const
//	{
//		if(mode == FileBrowser::OrderMode_Asc)
//		{
//			return CreatTime.makeTime() < item.CreatTime.makeTime();
//		}
//		else
//		{
//			return CreatTime.makeTime() > item.CreatTime.makeTime();
//		}
//	}
//};
//
//struct FileBrowser::FileBrowserInternal
//{
//	FileBrowserInternal(const std::string& dirpath):path(dirpath)
//	{
//		load(dirpath);
//	}
//
//	static uint64_t getTotalFileSize(const std::string& dirpath)
//	{
//		uint64_t totalSize = 0;
//
//		Directory dirent(dirpath);
//
//		Directory::Dirent dinfo;
//		while (dirent.read(dinfo))
//		{
//			if (dinfo.Type == Directory::Dirent::DirentType_Dir)
//			{
//				totalSize += getTotalFileSize(dinfo.CompletePath);
//			}
//			else
//			{
//				totalSize += dinfo.FileSize;
//			}
//		}
//
//		return totalSize;
//	}
//	void load(const std::string& dirpath)
//	{
//		Directory dirent(dirpath);
//
//		Directory::Dirent dinfo;
//		while (dirent.read(dinfo))
//		{
//			direntList.push_back(dinfo);
//		}
//	}	
//public:
//	std::vector<Directory::Dirent>	direntList;
//	std::string						path;
//};
//
//FileBrowser::FileBrowser(const std::string& dirpath)
//{
//	internal = new(std::nothrow) FileBrowserInternal(dirpath);
//}
//FileBrowser::~FileBrowser()
//{
//	SAFE_DELETE(internal);
//}
//bool FileBrowser::isExist() const
//{
//	return File::access(internal->path,File::accessExist);
//}
//uint64_t FileBrowser::fileTotalSize(const std::string& path)
//{
//	return FileBrowserInternal::getTotalFileSize(path);
//}
//
//bool FileBrowser::getFreeSpaceSize(const std::string& path,uint64_t& freeSize)
//{
//#ifdef WIN32
//	_ULARGE_INTEGER freespace,totalspace ,userspace;
//
//	BOOL ret = GetDiskFreeSpaceEx(path.c_str(),&freespace,&totalspace,&userspace);
//	if(!ret)
//	{
//		return false;
//	}
//
//	freeSize = freespace.QuadPart;
//
//	return true;
//#else
//	struct statfs diskstat;
//	if(statfs(path.c_str(),&diskstat) < 0)
//	{
//		return false;
//	}
//
//	uint64_t bsize = diskstat.f_bsize;
//	uint64_t bcount = diskstat.f_bfree;
//
//	freeSize = bsize * bcount;
//
//	return true;
//#endif
//}
//
//bool FileBrowser::read(Directory::Dirent& dir,uint32_t index) const
//{
//	if(index >= internal->direntList.size())
//	{
//		return false;
//	}
//
//	dir = internal->direntList[index];
//
//	return true;
//}
//bool FileBrowser::remove(const std::string& CompletePath)
//{
//	bool isdir = false;
//
//	std::vector<Directory::Dirent>::iterator iter;
//	for(iter = internal->direntList.begin();iter != internal->direntList.end();iter ++)
//	{
//		if(iter->CompletePath == CompletePath || iter->Path == CompletePath || iter->Name == CompletePath)
//		{
//			isdir = iter->Type == Directory::Dirent::DirentType_Dir;
//			break;
//		}
//	}
//
//	bool ret = false;
//	if(isdir)
//	{
//		ret = File::removeDirectory(CompletePath.c_str());
//	}
//	else
//	{
//		ret = File::remove(CompletePath.c_str());
//	}	
//
//	if(ret)
//	{
//		std::vector<Directory::Dirent>::iterator iter;
//		for(iter = internal->direntList.begin();iter != internal->direntList.end();iter ++)
//		{
//			if(iter->CompletePath == CompletePath || iter->Path == CompletePath || iter->Name == CompletePath)
//			{
//				internal->direntList.erase(iter);
//				break;
//			}
//		}
//	}
//	
//
//	return ret;
//}
//
//void FileBrowser::order(OrderType type,OrderMode mode)
//{
//	if(type == OrderType_Name)
//	{
//		std::vector<Directory::Dirent>::iterator iter;
//		std::list<OrderByNameItem> nameList;
//		for(iter = internal->direntList.begin();iter != internal->direntList.end();iter ++)
//		{
//			nameList.push_back(OrderByNameItem(*iter,mode));
//		}
//		nameList.sort();
//		internal->direntList.clear();
//		std::list<OrderByNameItem>::iterator oiter;
//		for(oiter = nameList.begin();oiter != nameList.end();oiter ++)
//		{
//			internal->direntList.push_back(oiter->getDirent());
//		}
//	}
//	else if(type == OrderType_Size)
//	{
//		std::vector<Directory::Dirent>::iterator iter;
//		std::list<OrderBySizeItem> nameList;
//		for(iter = internal->direntList.begin();iter != internal->direntList.end();iter ++)
//		{
//			nameList.push_back(OrderBySizeItem(*iter,mode));
//		}
//		nameList.sort();
//		internal->direntList.clear();
//		std::list<OrderBySizeItem>::iterator oiter;
//		for(oiter = nameList.begin();oiter != nameList.end();oiter ++)
//		{
//			internal->direntList.push_back(oiter->getDirent());
//		}
//	}
//	else
//	{
//		std::vector<Directory::Dirent>::iterator iter;
//		std::list<OrderByCteatTimeItem> nameList;
//		for(iter = internal->direntList.begin();iter != internal->direntList.end();iter ++)
//		{
//			nameList.push_back(OrderByCteatTimeItem(*iter,mode));
//		}
//		nameList.sort();
//		internal->direntList.clear();
//		std::list<OrderByCteatTimeItem>::iterator oiter;
//		for(oiter = nameList.begin();oiter != nameList.end();oiter ++)
//		{
//			internal->direntList.push_back(oiter->getDirent());
//		}
//	}
//}
} // namespace Base
} // namespace Public
