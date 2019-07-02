#pragma once

#include "Base/Base.h"
using namespace Public::Base;

// WIN32 Dynamic Link Library
#ifdef _MSC_VER

#ifdef ZIP_DLL_BUILD
#define  ZIP_API _declspec(dllexport)
#else
#define  ZIP_API _declspec(dllimport)
#endif

#else

#define ZIP_API
#endif

namespace Public {
namespace Zip {

//解压对象
class ZIP_API UnzipObject
{
public:
	struct ZipItem
	{
		int			index;
		std::string	filename;
	};
public:
	UnzipObject();
	~UnzipObject();

	bool openFromFile(const std::string& filename, const std::string& passwd = "");
	bool openFromMem(const std::string& data, const std::string& passwd = "");
	bool close();

	bool getItems(std::map<int, ZipItem>& items);
	bool unzipToMem(const ZipItem& item, std::string& data);
	bool unzipToFile(const ZipItem& item, const std::string& filename);
	bool unzipAll(const std::string& dirName);
private:
	struct UnzipObjectInternal;
	UnzipObjectInternal* internal;
};

//压缩对象
class ZIP_API ZipObject
{
public:
	ZipObject();
	~ZipObject();

	bool create(const std::string& passwd = "");
	bool saveToMem(std::string& data);
	bool saveToFile(const std::string& filename);

	bool addItemFromMem(const std::string& itemname, const std::string& dat);
	bool addItemFromFile(const std::string& itemname, const std::string& filename);
private:
	struct ZipObjectInternal;
	ZipObjectInternal* internal;
};

}
}