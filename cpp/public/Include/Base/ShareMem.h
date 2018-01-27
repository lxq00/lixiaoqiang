//
//  Copyright (c)1998-2012, Public Technology
//  All Rights Reserved.
//
//	Description:
//	$Id: ShareMem.h 3 2013-01-21 06:57:38Z jiangwei $

#ifndef __BASE_SHAREMEM_H__
#define __BASE_SHAREMEM_H__

#include <stddef.h>		
#include "Base/IntTypes.h"
#include "Defs.h"

namespace Public {
namespace Base {

////////////////////////////////////////////////////////////////////////////////
/// \class ShareMem
/// \brief 共享内存类

class BASE_API ShareMem 
{
	
	ShareMem();
	ShareMem(const std::string &, const size_t, bool ,void*);
	ShareMem(ShareMem const&);
	ShareMem& operator=(ShareMem const&);

public:

	/// 创建共享内存通道
	/// \param sharename [in] 共享内存的名称
	/// \param size [in] 共享内存的大小
	/// \return != NULL ShareMem实例指针
	///          == NULL 失败
	static shared_ptr<ShareMem> create(const std::string &sharename, const size_t size);

	/// 打开共享内存通道
	/// \param sharename [in] 共享内存的名称
	/// \param size [in] 共享内存的大小
	/// \param startAddr[in] 内存映射的起始地址
	/// \return != NULL ShareMem实例指针
	///          == NULL 失败
	static shared_ptr<ShareMem> open(const std::string &sharename, const size_t size,void* startAddr = NULL);

	/// 析构函数
	virtual ~ShareMem();

	/// 获得buffer的地址指针
	/// \retval 地址
	uint8_t *getbuffer();

	/// 获得buf的大小
	/// \retval buf大小
	size_t getSize();

	/// 打印共享内存内容,主要用于测试
	/// \param offset [in] 打印的内存相对起始位置的偏移量
	/// \param len [in] 长度
	void dumpData(const uint32_t offset, const uint32_t len);

	/// 获得共享内存的名称
	/// \retval 返回共享内存名称的引用
	const std::string &getname() const;

	/// 同步内存。保证写入内存
	/// \param offset [in] 内存相对起始位置的偏移量
	/// \param len [in] 长度
	/// \retval true 成功
	///         false 失败
	bool sync(const uint32_t offset, const uint32_t len);
	
private:
	struct Internal;
	Internal *internal;
};


} // namespace Base
} // namespace Public

#endif	// __BASE_SHAREMEM_H__


