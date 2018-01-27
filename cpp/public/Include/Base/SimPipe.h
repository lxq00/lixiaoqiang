//
//  Copyright (c)1998-2012, Public Technology
//  All Rights Reserved.
//
//	Description:
//	$Id: SimPipe.h 140 2013-06-27 07:55:35Z jiangwei $

#ifndef __BASE_SIMPIPE_H__
#define __BASE_SIMPIPE_H__

#include <string>	
#include <stddef.h>
#include "Base/IntTypes.h"
#include "Defs.h"

namespace Public {
namespace Base {

////////////////////////////////////////////////////////////////////////////////
/// \class SimPipe
/// \brief 仿真管道类

class BASE_API SimPipe 
{
	SimPipe(const std::string &);
	SimPipe(SimPipe const&);
	SimPipe& operator=(SimPipe const&);

public:
	SimPipe();
	/// 析构函数
	virtual ~SimPipe();

	/// 创建Pipe通道
	/// \param name [in] Pipe的名称
	/// \return != NULL SimPipe实例指针
	///          == NULL 失败
	bool creat(const std::string &name);

	/// 关闭pipe通道
	/// save 保存该pipe
	bool destory(bool save = false);

	/// 写数据
	/// \param buf [in] 数据地址
	/// \param len [in] 数据长度
	/// \retval  > 0 写的数据长度
	///          = 0 未写入
	///          < 0 失败
	int write(const void *buf, size_t len);

	/// 读数据
	/// \param buf [out] 读数据地址
	/// \param len [in] 数据长度
	/// \retval  > 0 读的数据长度
	///          = 0 没有数据
	///          < 0 失败
	/// \note  read 和readEx两个的调用 是线程不安全的,
	int read(void *buf, size_t len);

	typedef enum {
		seekPos_Cur,
		seekPos_Head,
	} SeekPos;
	
	/// 读位置Seek
	/// \param offset [in] 偏移
	/// \param pos [in]位置
	/// \retval true 成功
	///         false 失败
	bool readSeek(const int64_t &offset, SeekPos pos);

	/// 写位置Seek
	/// \param offset [in] 偏移
	/// \param pos [in]位置
	/// \retval true 成功
	///         false 失败
	bool writeSeek(const int64_t &offset, SeekPos pos);	

	/// 获得读的位置
	/// \retval 读的位置
	uint64_t getReadPos();

	/// 获得写的位置
	/// \retval 写的位置
	uint64_t getWritePos();

	/// 获得Pipe文件的名称
	/// \retval 返回文件名称的引用
	const std::string getname() const;
	
private:
	struct Internal;
	Internal *internal;
};


} // namespace Base
} // namespace Public

#endif	// __BASE_SIMPIPE_H__



