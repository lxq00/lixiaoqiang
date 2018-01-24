#ifndef __FILEBROWSER_H__
#define __FILEBROWSER_H__
#include "Base/Defs.h"
#include "Base/IntTypes.h"
#include "Base/Time.h"
#include <string>
using namespace std;

namespace Xunmei {
namespace Base {

class BASE_API FileBrowser
{
	struct FileBrowserInternal;
public:
	struct BASE_API Dirent
	{
		std::string		Name;
		std::string		CompletePath;
		std::string		Path;
		std::string		SuffixName;
		uint64_t		FileSize;
		Time			CreatTime;
		enum{
			DirentType_Dir,
			DirentType_File,
		}				Type;

	};
	enum OrderType
	{
		OrderType_Name,
		OrderType_Size,
		OrderType_CrteatTime,
	};
	enum OrderMode
	{
		OrderMode_Asc,
		OrderMode_Desc,
	};
public:
	FileBrowser(const std::string& dirpath);
	~FileBrowser();

	bool isExist() const;
	static uint64_t fileTotalSize(const std::string& path);
	static bool getFreeSpaceSize(const std::string& path,uint64_t& freeSize);

	void order(OrderType type,OrderMode mode);

	bool read(Dirent& dir,uint32_t index) const;
	bool remove(const std::string& CompletePath);
private:
	FileBrowserInternal* internal;
};

} // namespace Base
} // namespace Xunmei


#endif //__FILEBROWSER_H__
