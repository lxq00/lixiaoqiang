//
//  Copyright (c)1998-2012, Public Technology
//  All Rights Reserved.
//
//	Description:
//	$Id: MemPool.h 255 2014-01-23 03:28:32Z  $
//
#ifndef __DYNAMIC_MEMORYPOOL_H__
#define __DYNAMIC_MEMORYPOOL_H__
#include "Base/Defs.h"
#include "Base/IntTypes.h"
namespace Public{
namespace Base{

///内存池设定、用于Publicmedia、也可以其他地方使用
class BASE_API DynamicMemPool :public IMempoolInterface
{
	class MemPoolInternal;
public:
	DynamicMemPool();
	~DynamicMemPool();

	/// 内存分配、失败返回NULL
	void* Malloc(uint32_t size,uint32_t& realsize);
	
	/// 内存释放
	bool Free(void*);

	uint32_t usedBufferSize();

	/// 内存使用打印
//	void dump();

	///配置内存池总大小、默认为128M
//	static void config(uint32_t allSize);
private:
	MemPoolInternal*	internal;
};


} // namespace Base
} // namespace Public


#endif //__DYNAMIC_MEMORYPOOL_H__
