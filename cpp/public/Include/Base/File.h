//
//  Copyright (c)1998-2012, Public Technology
//  All Rights Reserved.
//
//	Description:
//	$Id: File.h 252 2013-12-18 04:40:28Z linyang $
//

#ifndef __BASE_FILE_H__
#define __BASE_FILE_H__

#include <string>
#ifdef WIN32
#include <io.h>
#endif

#include "Base/Defs.h"
#include "Base/IntTypes.h"

namespace Public {
namespace Base {

/// \struct FileInfo
/// \brief 文件信息结构
struct FileInfo
{
	enum {maxPathSize = 256};
	char		name[maxPathSize];	///< 文件名
	uint16_t	attrib;			///< 文件属性
	uint64_t	timeCreate;	///< 文件创建时间
	uint64_t	timeAccess;	///< 文件访问时间
	uint64_t	timeWrite;		///< 文件修改时间
	uint64_t	size;			///< 文件大小
};

#ifdef WIN32
#define PATH_SEPARATOR '\\'
#else
#define PATH_SEPARATOR '/'
#endif // WIN32

/// \class File
/// \brief 文件服务封装
class BASE_API File
{
	File(File const&);
	File& operator=(File const&);

public:

	/// 文件打开标志
	enum OpenFlags {
		modeRead =         (int) 0x00000, ///< 为读打开一个文件,如果文件不存在或无法打开,Open调用失败.
		modeWrite =        (int) 0x00001, ///< 为写创建一个空文件,如果文件存在,其内容将被销毁.
		modeReadWrite =    (int) 0x00002, ///< 为读写打开一个文件,如果文件不存在或无法打开,Open调用失败.
		shareCompat =      (int) 0x00000,
		shareExclusive =   (int) 0x00010,
		shareDenyWrite =   (int) 0x00020,
		shareDenyRead =    (int) 0x00030,
		shareDenyNone =    (int) 0x00040,
		modeNoInherit =    (int) 0x00080,
		modeCreate =       (int) 0x01000, ///< 如果文件不存在,自动创建文件,和modeReadWrite配合使用.
		modeNoTruncate =   (int) 0x02000, ///< 和modeCreate一起使用,如果要创建的文件已经存在,原来的内容保留.
		typeText =         (int) 0x04000, // typeText and typeBinary are
		typeBinary =       (int) 0x08000, // used in derived classes only
		osNoBuffer =       (int) 0x10000, ///< 上层处理缓冲,此时读写的偏移和长度都需要按页面大小对齐.
		osWriteThrough =   (int) 0x20000,
		osRandomAccess =   (int) 0x40000,
		osSequentialScan = (int) 0x80000,
	};

	/// 文件属性
	enum Attribute {
		normal =    0x00,
		readOnly =  0x01,
		hidden =    0x02,
		system =    0x04,
		volume =    0x08,
		directory = 0x10,
		archive =   0x20
	};

	/// 文件定位参考位置
	enum SeekPosition
	{
		begin = 0x0,
		current = 0x1,
		end = 0x2
	};

	/// 文件访问方式
	enum AccessMode
	{
		accessExist = 0,
		accessWrite = 2,
		accessRead = 4,
		accessReadWrite = 6,
	};

	/// 构造函数.
	File();

	/// 析构函数.如果数据缓冲还有效,会被释放.
	virtual ~File();

	/// 打开文件.打开后文件指针偏移在0处,而以modeAppend标志打开的文件文件指针在文件末尾.
	/// \param  pFileName [in]文件名.
	/// \param  dwFlags [in]打开标志,默认为modeReadWrite.
	/// \retval true  打开成功
	/// \retval false  打开失败,文件不存在或无法打开.
	virtual bool open(const char* pFileName, uint32_t dwFlags = modeReadWrite);

	/// 关闭文件.
	virtual void close();

	/// 装载数据,申请和文件长度一样大小的缓冲,将文件内容读到该缓冲,返回缓冲指针.
	/// 和unload函数一起提供方便
	/// \param  pFileName [in] 文件名.
	/// \retval NULL  装载失败
	/// \retval !NULL  数据缓冲指针.
	virtual uint8_t* load(const char* pFileName);

	/// 释放数据缓冲
	virtual void unload();

	/// 读文件数据.读操作后文件指针自动累加.
	/// \param pBuffer [out]  数据缓冲的指针.
	/// \param dwCount [in]  要读出的字节数
	/// \retval >=0  读出的字节数
	/// \retval <0 读失败
	virtual long read(void* pBuffer, size_t dwCount);

	/// 写文件数据.读操作后文件指针自动累加.
	/// \param pBuffer [in]  数据缓冲的指针.
	/// \param dwCount [in]  要写入的字节数
	/// \retval >=0  写入的字节数
	/// \retval <0 写失败
	virtual long write(void *pBuffer, size_t dwCount);

	/// 同步文件底层缓冲,在写操作后调用,确保写入的数据已经传给操作系统.
	virtual void flush();

	/// 文件定位.(Todo:Unix不支持4G以上文件)
	/// \param lOff [in]  偏移量,字节为单位.
	/// \param nFrom [in]  偏移相对位置,最后得到的偏移为lOff+nFrom.
	/// \retval 偏移后文件的指针位置.
	virtual uint64_t seek(int64_t lOff, SeekPosition nFrom);

	/// 获得当前的文件指针位置
	/// \retval 文件指针位置.
	virtual uint64_t getPosition();

	/// 获得文件长度
	/// \retval 文件长度
	virtual uint64_t getLength();

	/// 从文本文件当前偏移处读取一行字符串.读操作后文件指针自动累加.
	/// \param s [out]  数据缓冲.
	/// \param size [in]  需要读取的字符串长度.
	/// \retval NULL  读取失败
	/// \retval !NULL  字符串指针.
	virtual char* gets(char* s, size_t size);

	/// 从文本文件当前偏移处写入一行字符串.写操作后文件指针自动累加.
	/// \param s [in] 数据缓冲.
	/// \retval  实际写入字符串长度.
	virtual long puts(const char* s);

	/// 判断文件是否打开
	/// \retval true 已经打开
	/// \retval false 没有打开
	virtual bool isOpen();

	/// 获取当前可执行程序的文件名
	/// \param s [out]  数据缓冲.
	/// \param size [in]  需要读取的字符串长度.
	/// \retval 实际得到的字符串长度,失败返回-1
	static size_t getExcutableFileName(char* s, size_t size);

	/// 获取当前可执行程序文件的绝对路径
	/// \param s [out]  数据缓冲.
	/// \param size [in]  需要读取的字符串长度.
	/// \retval 实际得到的字符串长度,失败返回-1
	static size_t getExcutableFileFullPath(char* s, size_t size);

	/// 重命名文件
	/// \param oldName [in] 旧的文件名
	/// \param oldName [in] 新的文件名
	/// \retval true  成功
	/// \retval false 失败
	static bool rename(const char* oldName, const char* newName);

	/// 删除文件
	/// \param fileName [in] 文件名
	/// \retval true  成功
	/// \retval false 失败
	static bool remove(const char* fileName);

	/// 创建目录
	/// \param dirName [in] 目录名
	/// \retval true  成功
	/// \retval false 失败
	static bool makeDirectory(const char* dirName);

	/// 删除目录
	/// \param dirName [in] 目录名
	/// \retval true  成功
	/// \retval false 失败
	static bool removeDirectory(const char* dirName);

	/// 文件系统统计
	/// \param path [in]任意路径,不一定是分区根目录.
	/// \param userFreeBytes [out]目录所在文件系统分配给该用户的剩余空间字节数
	/// \param totalBytes [out]目录所在文件系统总的字节数
	/// \param totalFreeBytes [out]目录所在文件系统总的剩余空间字节数,如果使用了硬盘
	///        空间配额,userFreeBytes可能会比totalFreeBytes小
	/// \retval true  成功
	/// \retval false 失败
	static bool statFS(const char* path,
		uint64_t& userFreeBytes,
		uint64_t& totalBytes,
		uint64_t& totalFreeBytes);

	/// 判断文件或目录访问权限
	/// \param path [in]文件或目录的路径.
	/// \param mode [in]访问权限,\see AccessMode
	/// \retval true  有权限
	/// \retval false 没有权限
	static bool access(const char* path, int mode);

	/// 根据路径获取文件信息
	/// \param path [out]文件或目录的路径.
	/// \param info [out]文件信息,\see FileInfo
	/// \retval true  成功
	/// \retval false 失败
	static bool stat(const char* path, FileInfo& info);

protected:
	struct FileInternal;
	FileInternal* internal;
};

////////////////////////////////////////////////////////////////////////////////

/// \class FileFind
/// \brief 文件查找类,支持'*','?'通配符查找
class BASE_API FileFind
{
	FileFind(FileFind const&);
	FileFind& operator=(FileFind const&);

public:
	/// 构造函数
	FileFind();

	/// 析构函数
	virtual ~FileFind();

	/// 查找第一个文件
	/// \param fileName [in] 包含通配符的路径名
	/// \retval ture 找到
	/// \retval false 没有找到
	virtual bool findFile(const char* fileName);

	/// 查找下一个文件,使用和上次findFile相同的条件,必须在findFile之后调用(Linux端有问题)
	/// \retval ture 找到
	/// \retval false 没有找到
	virtual bool findNextFile();

	/// 关闭查找,关闭后可以再次调用findFile
	virtual void close();

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

protected:
	struct FileFindInternal;
	FileFindInternal* internal;
};

} // namespace Base
} // namespace Public

#endif //__BASE_FILE_H__

