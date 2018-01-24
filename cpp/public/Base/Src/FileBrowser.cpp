#include "Base/FileBrowser.h"
#include "Base/BaseTemplate.h"
#include "Base/File.h"
#include "Base/String.h"
#include "Base/ReadWriteMutex.h"
#include "Base/Guard.h"
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

namespace Xunmei {
namespace Base {

struct OrderByItem:public FileBrowser::Dirent
{
	OrderByItem(const FileBrowser::Dirent& dirent,FileBrowser::OrderMode _mode):mode(_mode)
	{
		Name = dirent.Name;
		CompletePath = dirent.CompletePath;
		Path = dirent.Path;
		SuffixName = dirent.SuffixName;
		FileSize = dirent.FileSize;
		CreatTime = dirent.CreatTime;
		Type= dirent.Type;
	}
	FileBrowser::Dirent getDirent()
	{
		FileBrowser::Dirent dirent;

		dirent.Name = Name;
		dirent.CompletePath = CompletePath;
		dirent.Path = Path;
		dirent.SuffixName = SuffixName;
		dirent.FileSize = FileSize;
		dirent.CreatTime = CreatTime;
		dirent.Type= Type;

		return dirent;
	}

	FileBrowser::OrderMode mode;
};

struct OrderByNameItem:public OrderByItem
{
	OrderByNameItem(const FileBrowser::Dirent& dirent,FileBrowser::OrderMode mode):OrderByItem(dirent,mode)
	{
	}
	
	bool operator <(const OrderByNameItem& item) const
	{
		if(mode == FileBrowser::OrderMode_Asc)
		{
			return strcmp(Name.c_str(),item.Name.c_str()) < 0;
		}
		else
		{
			return strcmp(Name.c_str(),item.Name.c_str()) > 0;
		}
	}	
};
struct OrderBySizeItem:public OrderByItem
{
	OrderBySizeItem(const FileBrowser::Dirent& dirent,FileBrowser::OrderMode mode):OrderByItem(dirent,mode)
	{
	}

	bool operator <(const OrderBySizeItem& item) const
	{
		if(mode == FileBrowser::OrderMode_Asc)
		{
			return FileSize < item.FileSize;
		}
		else
		{
			return FileSize > item.FileSize;
		}
	}
};
struct OrderByCteatTimeItem:public OrderByItem
{
	OrderByCteatTimeItem(const FileBrowser::Dirent& dirent,FileBrowser::OrderMode mode):OrderByItem(dirent,mode)
	{
	}

	bool operator <(const OrderByCteatTimeItem& item) const
	{
		if(mode == FileBrowser::OrderMode_Asc)
		{
			return CreatTime.makeTime() < item.CreatTime.makeTime();
		}
		else
		{
			return CreatTime.makeTime() > item.CreatTime.makeTime();
		}
	}
};

struct FileBrowser::FileBrowserInternal
{
	FileBrowserInternal(const std::string& dirpath):status(false),path(dirpath)
	{
		load(dirpath);
	}
#ifdef WIN32
	static uint64_t getTotalFileSize(const std::string& dirpath)
	{
		uint64_t totalSize = 0;
		
		WIN32_FIND_DATA findData;
		std::string path = dirpath + PATH_SEPARATOR + "*.*";	
		HANDLE hFile = FindFirstFile(path.c_str(),&findData);
		if(hFile == INVALID_HANDLE_VALUE)
		{
			return 0;
		}
		
		do{
			if(strcmp(findData.cFileName,".") != 0 && strcmp(findData.cFileName,"..") != 0)
			{
				std::string suffixName;
				const char* tmp = strrchr(findData.cFileName,'.');
				if(tmp != NULL)
				{
					suffixName = tmp + 1;
				}
				if((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && strcasecmp(suffixName.c_str(),"lnk") != 0)
				{
					std::string completePath = dirpath + std::string(std::string("")+PATH_SEPARATOR) + findData.cFileName;

					totalSize += getTotalFileSize(completePath);
				}
				else
				{
					uint64_t high = findData.nFileSizeHigh;
					totalSize += (high << 32) | findData.nFileSizeLow;
				}
			}
		}while(FindNextFile(hFile,&findData));

		FindClose(hFile);

		return totalSize;
	}
	void load(const std::string& dirpath)
	{
		GuardWriteMutex guard(direntMutex);
		WIN32_FIND_DATA findData;

		std::string path = dirpath + PATH_SEPARATOR + "*.*";
		
		HANDLE hFile = FindFirstFile(path.c_str(),&findData);
		if(hFile == INVALID_HANDLE_VALUE)
		{
			int a = GetLastError();
			return;
		}
		
		do{
			if(strcmp(findData.cFileName,".") != 0 && strcmp(findData.cFileName,"..") != 0)
			{
				Dirent dir;
				
				dir.Type = (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? Dirent::DirentType_Dir : Dirent::DirentType_File;
				dir.Name = findData.cFileName;
				dir.CompletePath = dirpath + std::string(std::string("")+PATH_SEPARATOR) + dir.Name;
				dir.Path = dirpath;				
				const char* tmp = strrchr(findData.cFileName,'.');
				if(tmp != NULL)
				{
					dir.SuffixName = tmp + 1;
				}

				SYSTEMTIME systime;
				FileTimeToSystemTime(&findData.ftLastAccessTime,&systime);

				dir.CreatTime = Time(systime.wYear,systime.wMonth,systime.wDay,systime.wHour,systime.wMinute,systime.wSecond);

				if(dir.Type == Dirent::DirentType_File)
				{
					uint64_t high = findData.nFileSizeHigh;
					dir.FileSize = (high << 32) | findData.nFileSizeLow;
				}
				else
				{	
					dir.FileSize = 0;
				}				
				
				direntList.push_back(dir);
			}
		}while(FindNextFile(hFile,&findData));

		FindClose(hFile);
		
		status = true;
	}	
#else
	static uint64_t getTotalFileSize(const std::string& dirpath)
	{
		uint64_t totalSize = 0;

		DIR* pdir = opendir(dirpath.c_str());
		if(pdir == NULL)
		{
			return 0;
		}

		dirent* pdirent;

		while((pdirent = readdir(pdir)) != NULL)
		{
			if(strcmp(pdirent->d_name,".") == 0 || strcmp(pdirent->d_name,"..") == 0)
			{
				continue;
			}
			if(pdirent->d_type != DT_DIR && pdirent->d_type != DT_REG)
			{
				continue;
			}
			std::string completePath = dirpath + PATH_SEPARATOR + pdirent->d_name;
			if((pdirent->d_type == DT_DIR))
			{
				totalSize += getTotalFileSize(completePath);
			}
			else
			{
				struct stat filestat;
				stat(completePath.c_str(),&filestat);
				totalSize += filestat.st_size;
			}
		}
		closedir(pdir);

		return totalSize;
	}

	void load(const std::string& dirpath)
	{
		GuardWriteMutex guard(direntMutex);
		DIR* pdir = opendir(dirpath.c_str());
		if(pdir == NULL)
		{
			return;
		}

		dirent* pdirent;

		while((pdirent = readdir(pdir)) != NULL)
		{
			if(strcmp(pdirent->d_name,".") == 0 || strcmp(pdirent->d_name,"..") == 0)
			{
				continue;
			}

			Dirent dir;
			dir.Type = (pdirent->d_type == DT_DIR) ? Dirent::DirentType_Dir : Dirent::DirentType_File;
			dir.Name = pdirent->d_name;
			dir.CompletePath = dirpath + PATH_SEPARATOR + dir.Name;
			dir.Path = dirpath;
			const char* tmp = strrchr(pdirent->d_name,'.');
			if(tmp != NULL)
			{
				dir.SuffixName = tmp + 1;
			}

			struct stat filestat;
			stat(dir.CompletePath.c_str(),&filestat);

			Time creattime;
			creattime.breakTime(filestat.st_atime);
			dir.CreatTime = creattime;

			if(dir.Type == Dirent::DirentType_File && !(pdirent->d_type == DT_LNK))
			{
				dir.FileSize = filestat.st_size;
			}
			else
			{	
				dir.FileSize = 0;
			}				

			direntList.push_back(dir);	
		}
		closedir(pdir);
		
		status = true;
	}
#endif
public:
	bool							status;
	ReadWriteMutex					direntMutex;
	std::vector<Dirent>				direntList;
	std::string						path;
};

FileBrowser::FileBrowser(const std::string& dirpath)
{
	internal = new(std::nothrow) FileBrowserInternal(dirpath);
}
FileBrowser::~FileBrowser()
{
	SAFE_DELETE(internal);
}
bool FileBrowser::isExist() const
{
	return internal->status;
}
uint64_t FileBrowser::fileTotalSize(const std::string& path)
{
	return FileBrowserInternal::getTotalFileSize(path);
}

bool FileBrowser::getFreeSpaceSize(const std::string& path,uint64_t& freeSize)
{
#ifdef WIN32
	_ULARGE_INTEGER freespace,totalspace ,userspace;

	BOOL ret = GetDiskFreeSpaceEx(path.c_str(),&freespace,&totalspace,&userspace);
	if(!ret)
	{
		return false;
	}

	freeSize = freespace.QuadPart;

	return true;
#else
	struct statfs diskstat;
	if(statfs(path.c_str(),&diskstat) < 0)
	{
		return false;
	}

	uint64_t bsize = diskstat.f_bsize;
	uint64_t bcount = diskstat.f_bfree;

	freeSize = bsize * bcount;

	return true;
#endif
}

bool FileBrowser::read(Dirent& dir,uint32_t index) const
{
	GuardReadMutex guard(internal->direntMutex);
	if(index >= internal->direntList.size())
	{
		return false;
	}

	dir = internal->direntList[index];

	return true;
}
bool FileBrowser::remove(const std::string& CompletePath)
{
	bool isdir = false;

	GuardWriteMutex guard(internal->direntMutex);
	std::vector<Dirent>::iterator iter;
	for(iter = internal->direntList.begin();iter != internal->direntList.end();iter ++)
	{
		if(iter->CompletePath == CompletePath || iter->Path == CompletePath || iter->Name == CompletePath)
		{
			isdir = iter->Type == Dirent::DirentType_Dir;
			break;
		}
	}

	bool ret = false;
	if(isdir)
	{
		ret = File::removeDirectory(CompletePath.c_str());
	}
	else
	{
		ret = File::remove(CompletePath.c_str());
	}	

	if(ret)
	{
		std::vector<Dirent>::iterator iter;
		for(iter = internal->direntList.begin();iter != internal->direntList.end();iter ++)
		{
			if(iter->CompletePath == CompletePath || iter->Path == CompletePath || iter->Name == CompletePath)
			{
				internal->direntList.erase(iter);
				break;
			}
		}
	}
	

	return ret;
}

void FileBrowser::order(OrderType type,OrderMode mode)
{
	GuardWriteMutex guard(internal->direntMutex);
	if(type == OrderType_Name)
	{
		std::vector<Dirent>::iterator iter;
		std::list<OrderByNameItem> nameList;
		for(iter = internal->direntList.begin();iter != internal->direntList.end();iter ++)
		{
			nameList.push_back(OrderByNameItem(*iter,mode));
		}
		nameList.sort();
		internal->direntList.clear();
		std::list<OrderByNameItem>::iterator oiter;
		for(oiter = nameList.begin();oiter != nameList.end();oiter ++)
		{
			internal->direntList.push_back(oiter->getDirent());
		}
	}
	else if(type == OrderType_Size)
	{
		std::vector<Dirent>::iterator iter;
		std::list<OrderBySizeItem> nameList;
		for(iter = internal->direntList.begin();iter != internal->direntList.end();iter ++)
		{
			nameList.push_back(OrderBySizeItem(*iter,mode));
		}
		nameList.sort();
		internal->direntList.clear();
		std::list<OrderBySizeItem>::iterator oiter;
		for(oiter = nameList.begin();oiter != nameList.end();oiter ++)
		{
			internal->direntList.push_back(oiter->getDirent());
		}
	}
	else
	{
		std::vector<Dirent>::iterator iter;
		std::list<OrderByCteatTimeItem> nameList;
		for(iter = internal->direntList.begin();iter != internal->direntList.end();iter ++)
		{
			nameList.push_back(OrderByCteatTimeItem(*iter,mode));
		}
		nameList.sort();
		internal->direntList.clear();
		std::list<OrderByCteatTimeItem>::iterator oiter;
		for(oiter = nameList.begin();oiter != nameList.end();oiter ++)
		{
			internal->direntList.push_back(oiter->getDirent());
		}
	}
}
} // namespace Base
} // namespace Xunmei
