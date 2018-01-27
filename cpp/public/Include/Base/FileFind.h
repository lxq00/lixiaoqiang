//
//  Copyright (c)1998-2012, Public Technology
//  All Rights Reserved.
//
//	Description:
//	$Id: File.h 252 2013-12-18 04:40:28Z  $
//

#ifndef __BASE_FILEFIND_H__
#define __BASE_FILEFIND_H__

#include <string>
#ifdef WIN32
#include <io.h>
#endif

#include "Base/Defs.h"
#include "Base/IntTypes.h"
#include "Base/Shared_ptr.h"

namespace Public {
namespace Base {

/// \class FileFind
/// \brief 文件查找类,支持'*','?'通配符查找
class BASE_API FileFind
{
	FileFind(FileFind const&);
	FileFind& operator=(FileFind const&);
public:
	class BASE_API FileFindInfo
	{
		friend class FileFind;
		FileFindInfo(FileFindInfo&);
		FileFindInfo& operator=(const FileFindInfo&);
		FileFindInfo(const std::string& path,void* findinfo);
	public:
		virtual ~FileFindInfo();

		/// 得到查找到的文件的长度
		/// \retval 文件长度
		virtual size_t getLength();

		/// 得到查找到的文件的文件名
		/// \retval 文件名称
		virtual std::string getFileName();

		/// 得到查找到的文件的全路径
		/// \retval 全路径
		virtual std::string getFilePath();

		/// 是否为只读文件
		/// \retval true 只读
		/// \retval false 不是只读
		virtual bool isReadOnly();

		/// 是否为目录文件
		/// \retval true 是目录
		/// \retval false 不是目录
		virtual bool isDirectory();

		/// 是否为隐藏文件
		/// \retval true 是
		/// \retval false 不是
		virtual bool isHidden();

		/// 是否为普通文件
		/// \retval true 是
		/// \retval false 不是	
		virtual bool isNormal();
	private:
		struct FileFindInfoInternal;
		FileFindInfoInternal* internal;
	};
public:
	/// 构造函数
	FileFind(const std::string& findString);

	/// 析构函数
	virtual ~FileFind();

	/// 查找第一个文件
	/// \retval object 找到
	/// \retval null 没有找到
	shared_ptr<FileFind::FileFindInfo>  find();
private:
	struct FileFindInternal;
	FileFindInternal* internal;
};

} // namespace Base
} // namespace Public

#endif //__BASE_FILE_H__

