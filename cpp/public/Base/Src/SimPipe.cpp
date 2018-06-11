//
//  Copyright (c)1998-2012,  Public Technology
//  All Rights Reserved.
//
//	Description:
//	$Id: SimPipe.cpp 141 2013-06-27 09:08:03Z  $

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#ifndef WIN32
#include <unistd.h>
#endif
#include "Base/SimPipe.h"
#include "Base/Guard.h"
#include "Base/File.h"
#include "Base/BaseTemplate.h"

namespace Public {
namespace Base {

struct SimPipe::Internal
{
	Internal(const std::string &name_):status(false), file(NULL), readpos(0), writepos(0),isSave(false)
	{
		file = ::fopen(name_.c_str(),"rb");
		if(file == NULL)
		{
			file = ::fopen(name_.c_str(), "wb+");
		}		
		if (file != NULL)
		{
			status = true;
			name = name_;
			::fseek(file,0,SEEK_END);
			writepos = ::ftell(file);
			::fseek(file,0,SEEK_SET);
		}
	}
	~Internal()
	{
		if (file != NULL )
		{
			fclose(file);
		}
		if(!isSave)
		{
			File::remove(name.c_str());
		}
	}
	int write(const void *buf, size_t len)
	{
		Guard guard(mutex);
		::fseek(file, (long)writepos, SEEK_SET);
		int wlen = ::fwrite(buf,1,len, file);
		if (wlen > 0)
		{
			writepos += wlen;

			::fflush(file);

		}
		return wlen;
	}
	int read(void *buf, size_t len)
	{
		Guard guard(mutex);
		::fseek(file,  (long)readpos, SEEK_SET);
		int rlen = ::fread(buf,1,  len, file);

		if (rlen >= 0)
		{
			readpos += rlen;
		}
		return rlen;
	}
	
	bool readSeek(const int64_t &offset, SimPipe::SeekPos pos)
	{
		Guard guard(mutex);
		if (pos == SimPipe::seekPos_Cur)
		{
			uint64_t off = (uint64_t)((int64_t)readpos + offset);
			if (off >= 0 && off <= writepos)
			{
				readpos = off;
				return true;
			}
			else return false;
		}
		else if (offset >= 0 && offset <= (int64_t)writepos)
		{
			readpos = (uint64_t)offset;
			return true;
		}
		return false;
	}

	/// 写位置Seek
	/// \param offset [in] 偏移
	/// \param pos [in]位置
	/// \retval true 成功
	///         false 失败
	bool writeSeek(const int64_t &offset, SeekPos pos)
	{
		Guard guard(mutex);
		if (pos == SimPipe::seekPos_Cur)
		{
			uint64_t off = (uint64_t)((int64_t)writepos + offset);
			if (off >= readpos && off <= writepos)
			{
				writepos = off;
				return true;
			}
			else return false;
		}
		else if (offset >= (int64_t)readpos && offset <= (int64_t)writepos)
		{
			writepos = (uint64_t)offset;
			return true;
		}
		return false;	
	}
	uint64_t getReadPos()
	{
		Guard gurad(mutex);
		return readpos;
	}

	/// 获得写的位置
	/// \retval 写的位置
	uint64_t getWritePos()
	{
		Guard gurad(mutex);
		return writepos;
	}
	const std::string &getname() const { return name;}

	bool status;
	std::string name;
	FILE *file;
	uint64_t readpos;
	uint64_t writepos;
	Mutex mutex;
	bool isSave;
};

SimPipe::SimPipe():internal(NULL)
{
}

/// 创建Pipe通道
/// \param name [in] Pipe的名称
/// \return != NULL SimPipe实例指针
	///          == NULL 失败
bool SimPipe::creat(const std::string &name)
{
	if(internal != NULL)
	{
		SAFE_DELETE(internal);
	}
	internal = new Internal(name);

	return internal != NULL && internal->status;
}

bool SimPipe::destory(bool save)
{
	if(internal != NULL)
	{
		internal->isSave = save;
		SAFE_DELETE(internal);
	}

	return true;
}
	/// 析构函数

SimPipe::~SimPipe()
{
	SAFE_DELETE(internal);
}

/// 写数据
/// \param buf [in] 数据地址
/// \param len [in] 数据长度
/// \retval  > 0 写的数据长度
///          = 0 未写入
///          < 0 失败
int SimPipe::write(const void *buf, size_t len)
{
	if(internal == NULL)
	{
		return -1;
	}
	return internal->write(buf, len);
}

/// 读数据
/// \param buf [out] 读数据地址
/// \param len [in] 数据长度
/// \retval  > 0 读的数据长度
///          = 0 没有数据
///          < 0 失败
int SimPipe::read(void *buf, size_t len)
{
	if(internal == NULL)
	{
		return -1;
	}
	return internal->read(buf, len);
}


/// 读位置Seek
/// \param offset [in] 偏移
/// \param pos [in]位置
/// \retval true 成功
///         false 失败
bool SimPipe::readSeek(const int64_t &offset, SeekPos pos)
{
	if(internal == NULL)
	{
		return false;
	}
	return internal->readSeek(offset, pos);
}

/// 写位置Seek
/// \param offset [in] 偏移
/// \param pos [in]位置
/// \retval true 成功
///         false 失败
bool SimPipe::writeSeek(const int64_t &offset, SeekPos pos)
{
	if(internal == NULL)
	{
		return false;
	}
	return internal->writeSeek(offset, pos);
}

/// 获得读的位置
/// \retval 读的位置
uint64_t SimPipe::getReadPos()
{
	if(internal == NULL)
	{
		return -1;
	}
	return internal->getReadPos();
}

/// 获得写的位置
/// \retval 写的位置
uint64_t SimPipe::getWritePos()
{
	if(internal == NULL)
	{
		return -1;
	}
	return internal->getWritePos();
}

/// 获得Pipe文件的名称
/// \retval 返回文件名称的引用
const std::string SimPipe::getname() const
{
	if(internal == NULL)
	{
		return "";
	}
	return internal->getname();
}


} // namespace Base
} // namespace Public

