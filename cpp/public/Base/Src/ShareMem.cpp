//
//  Copyright (c)1998-2012,  Public Technology
//  All Rights Reserved.
//
//	Description:
//	$Id: ShareMem.cpp 55 2013-03-01 08:55:08Z  $
//

#include <stdexcept>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <new>

#if  defined( __i386__ ) || defined( __x86_64__ ) || defined(__linux__)
	#include <sys/mman.h>
	#include <unistd.h>
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <fcntl.h>	   
	#define INVALID_HANDLE_VALUE -1
#elif defined(_WIN32) || defined(WIN32)
	#include <windows.h>
	#include <stdio.h>
	#include <conio.h>  
#endif




#include "Base/ShareMem.h"
#include "Base/PrintLog.h"



namespace Public{
namespace Base{


struct ShareMem::Internal
{
public:
	Internal(const std::string &sharename, const size_t size, bool iscreate,void* startAddr):buf(NULL)
		, hMapFile(INVALID_HANDLE_VALUE)
	{
#if  defined( __i386__ ) || defined( __x86_64__ )
		if (iscreate)
		{
			int fd;
			if ( (fd = shm_open(sharename.c_str(), O_CREAT | O_RDWR | O_TRUNC,
				S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) < 0)
			{
				logerror("ShareMem Create Could not open file mapping:%s:%s \n", strerror(errno), sharename.c_str());
				return ;
			}
			if (ftruncate(fd, size) < 0)
			{
				logerror("ShareMem Create Could set length(%s, %d)\n", sharename.c_str(), size);
				return ;
			}
			if ((buf = (uint8_t *)mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0)) 
					== (uint8_t *) -1)
			{
				buf = NULL;
				logerror("ShareMem Create Could set mmap(%s, %d)\n", sharename.c_str(), size);
				return ;
				
			}
			close(fd);
		}
		else
		{
			int fd;
			if ( (fd = shm_open(sharename.c_str(), O_RDWR, 0)) < 0)
			{
				logerror("ShareMem Open Could not open file mapping:%s:(%s, %d)\n", strerror(errno), 
					sharename.c_str(), size);
				return ;
			}
			if (ftruncate(fd, size) < 0)
			{
				logerror("ShareMem Open Could set length(%s, %d)\n", sharename.c_str(), size);
				return ;
			}
			if ((buf = (uint8_t *)mmap(startAddr,size, 
	   				PROT_WRITE|PROT_READ,MAP_SHARED, fd, 0)) 
					== (uint8_t *) -1)
			{
				buf = NULL;
				logerror("ShareMem Open Could set mmap:%s (%s, %d)\n", strerror(errno),sharename.c_str(), size);

				return ;
				
			}
			close(fd);
		}
#elif defined(_WIN32) || defined(WIN32)
		if (iscreate)
		{

			

		    hMapFile = CreateFileMapping(
		                 INVALID_HANDLE_VALUE,    // use paging file
		                 NULL,                    // default security 
						 PAGE_READWRITE | SEC_COMMIT,          // read/write access
		                 0,                       // max. object size 
		                 size,                // buffer size  
		                sharename.c_str()); // name of mapping object
		 
		   if (hMapFile == NULL) 
		   { 
			  logerror("ShareMem Create Could not open file mapping object(%d)(%s, %d)\n", 
			  	 GetLastError(), sharename.c_str(), size);
		      return ;
		   } 
		 
		   buf = (uint8_t*)MapViewOfFileEx(hMapFile, // handle to map object
		               FILE_MAP_WRITE,  // read/write permission
		               0,                    
		               0,                    
		               size, NULL);  
		}
		else
		{

		    HANDLE hMapFile;
	  
	  		hMapFile = OpenFileMapping(
	                   FILE_MAP_WRITE,
	                   FALSE,                               // do not inherit the name
	                  sharename.c_str());    // name of mapping object 
	 
		   if (hMapFile == NULL) 
		   { 
			 logerror("PacketManager Could not open file mapping object (%d):(%s, %d)\n", GetLastError(), 
					sharename.c_str(), size);

		      return;
		   } 
		 
		   buf = (uint8_t*) MapViewOfFileEx(hMapFile, // handle to map object
		               FILE_MAP_WRITE,  // write permission
		               0,                    
		               0,
		               size, startAddr);                   
		 
		   if (buf == NULL) 
		   {
			   CloseHandle(hMapFile);
			   logerror("ShareMem not map view of file(%d)(%s, %d)\n",
				   GetLastError(), sharename.c_str(), size);
		      return;
		   }
		}
#endif
		len = size;
		name = sharename;
	}
   	~Internal()
   	{
	   	if(buf)
		{
#if defined( __i386__ ) || defined( __x86_64__ )
			::shm_unlink(name.c_str());
#elif defined(_WIN32) || defined(WIN32)
	 		UnmapViewOfFile((LPCTSTR)buf);
			CloseHandle(hMapFile);
#endif
		}
   	}
	bool isValid()
	{
		return (buf != NULL);
	}

	uint8_t *getbuffer()
	{
		return buf;
	}

	size_t getSize()
	{
		return len;
	}
	
	const std::string &getname() const
	{
		return name;
	}
	bool sync(const uint32_t offset, const uint32_t len)
	{
#ifndef WIN32
	return msync(buf + offset, len, MS_SYNC);
#else
	return true;
#endif		
	}

private:
	uint8_t *buf;
	size_t len;
	std::string name;
#ifdef WIN32
	HANDLE hMapFile;
#else
	int hMapFile;
#endif
};


ShareMem::ShareMem(const std::string &sharename, const size_t size, bool iscreat,void* startaddr)
{
	internal = new Internal(sharename, size, iscreat,startaddr);
}


shared_ptr<ShareMem> ShareMem::create(const std::string &sharename, const size_t size)
{
	shared_ptr<ShareMem> mem;
	mem  = new ShareMem(sharename, size, true,NULL);
	if (mem->internal->isValid())
		return mem;
	else
	{
		return shared_ptr<ShareMem>();
	}

}
shared_ptr<ShareMem> ShareMem::open(const std::string &sharename, const size_t size,void* startaddr)
{
	shared_ptr<ShareMem> p;
	p = new ShareMem(sharename, size, false,startaddr);
	if (p->internal->isValid())
		return p;
	else
	{
		return shared_ptr<ShareMem>();
	}

}
ShareMem::~ShareMem()
{
	delete internal;
}


uint8_t * ShareMem::getbuffer()
{
	return internal->getbuffer();
}

size_t ShareMem::getSize()
{
	return internal->getSize();
}


const std::string &ShareMem::getname() const
{
	return internal->getname();
}

bool ShareMem::sync(const uint32_t offset, const uint32_t len)
{
	return internal->sync(offset, len);
}
	

#if 0
class BASE_API ShareMem 
{
	ShareMem();
	ShareMem(ShareMem const&);
	ShareMem& operator=(ShareMem const&);

public:
	
	static ShareMem * create(const std::string &sharename, const size_t size);

	static ShareMem * open(const std::string &sharename, const size_t size);

	virtual ~ShareMem();

	/// 得到管理的内存总字节数,这个值就是缺省值或者config配置后的值.
	/// \retval 总大小
	uint8_t *getbuffer();

	/// 得到管理的内存剩余字节数,是按2^n页面来进行统计的,可能会比用户调用GetPacket
	/// 时传入的bytes值累加的结果要大.
	/// \retval 剩余字节数
	size_t getSize();

	/// 打印内存节点的状态,仅用于调试
	void dumpData(const uint32_t offset, const uint32_t len);

private:
	struct Internal;
	Internal *internal;
};

#endif



#if 0
///////////////////////////////////////////
/////////////////////////////PacketManagerInternal Impl
// 默认的配置参数

PacketManager::PoolParameter PacketManagerInternal::sm_param = {
	(128 * 1024 * 1024),
	1024,
	4, 
	-1,
	0
};

std::string PacketManagerInternal::sm_sharestring =
#if  defined( __i386__ ) || defined( __x86_64__ )
	"vgssharemem";
#elif defined(_WIN32) || defined(WIN32)
	"Globe\\vgssharemem";
#endif
	
/// 单件模式
PacketManagerInternal* PacketManagerInternal::instance()
{
	static PacketManagerInternal inst;
	
	return &inst;
}

PacketManagerInternal::PacketManagerInternal()
:originBuffer(NULL)
,chunkHead(NULL)
,bufferList(NULL)
{	
	
#if  defined( __i386__ ) || defined( __x86_64__ )
	if (sm_param.offset < 0)
	{
		alignSize = 1 << Base::log2i(sm_param.alignSize);
		chunkCount = sm_param.totalSize/sm_param.chunkSize;
		int fd;
		if ( (fd = shm_open(sm_sharestring.c_str(), O_CREAT | O_RDWR | O_TRUNC,
			S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) < 0)
		{
			logerror("Could not open file mapping:%s:%s \n", strerror(errno), sm_sharestring.c_str());
			return ;
		}
		if (ftruncate(fd, sm_param.totalSize + sm_param.chunkSize) < 0)
		{
			logerror("Could set length\n");
			return ;
		}
		if ((originBuffer = (uint8_t *)mmap(NULL, sm_param.totalSize + sm_param.chunkSize, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0)) 
				== (uint8_t *) -1)
		{
			originBuffer = NULL;
			logerror("Could set mmap\n");
			return ;
			
		}
		close(fd);
	}
	else
	{
		int fd;
		if ( (fd = shm_open(sm_sharestring.c_str(), O_RDWR, 0)) < 0)
		{
			logerror("PacketManager Could not open file mapping:%s:%s \n", strerror(errno), 
				sm_sharestring.c_str());
			return ;
		}
		if (ftruncate(fd, sm_param.allSize) < 0)
		{
			logerror("PacketManager Could set length\n");
			return ;
		}
		if ((originBuffer = (uint8_t *)mmap(NULL,sm_param.totalSize, 
   				PROT_WRITE|PROT_READ,MAP_SHARED, fd, sm_param.offset)) 
				== (uint8_t *) -1)
		{
			originBuffer = NULL;
			logerror("PacketManager Could set mmap:%s \n", strerror(errno));

			return ;
			
		}
		chunkCount = (sm_param.totalSize - sm_param.chunkSize + 1)/sm_param.chunkSize;
		alignSize = 1;
		close(fd);
	}
#elif defined(_WIN32) || defined(WIN32)
	if (sm_param.offset < 0)
	{
		alignSize = 1 << Base::log2i(sm_param.alignSize);
	
		chunkCount = sm_param.totalSize/sm_param.chunkSize;
		
		HANDLE hMapFile;

	    hMapFile = CreateFileMapping(
	                 INVALID_HANDLE_VALUE,    // use paging file
	                 NULL,                    // default security 
	                 PAGE_READWRITE,          // read/write access
	                 0,                       // max. object size 
	                 sm_param.totalSize + sm_param.chunkSize,                // buffer size  
	                 (LPCWSTR)sm_sharestring.c_str()); // name of mapping object
	 
	   if (hMapFile == NULL) 
	   { 
	      logerror("Could not open file mapping object (%d).\n", 
	             GetLastError());
	      return ;
	   } 
	 
	   originBuffer = (uint8_t*) MapViewOfFile(hMapFile, // handle to map object
	               FILE_MAP_ALL_ACCESS,  // read/write permission
	               0,                    
	               0,                    
	              sm_param.totalSize + sm_param.chunkSize);  
	}
	else
	{

	    HANDLE hMapFile;
  
  		hMapFile = OpenFileMapping(
                   FILE_MAP_WRITE,
                   FALSE,                               // do not inherit the name
                   (LPCWSTR)sm_sharestring.c_str());    // name of mapping object 
 
	   if (hMapFile == NULL) 
	   { 
	      printf("PacketManager Could not open file mapping object (%d).\n", 
	             GetLastError());
	      return;
	   } 
	 
	   originBuffer = (uint8_t*) MapViewOfFile(hMapFile, // handle to map object
	               FILE_MAP_WRITE,  // write permission
	               (DWORD) (sm_param.offset>>32),                    
	                (DWORD)(sm_param.offset&0xffffffff),                    
	               sm_param.totalSize);                   
	 
	   if (originBuffer == NULL) 
	   { 
	      printf("Could not map view of file (%d).\n", 
	             GetLastError()); 
	      return;
	   }
	   chunkCount = (sm_param.totalSize - sm_param.chunkSize + 1)/sm_param.chunkSize;
	   alignSize = 1;
	}
#endif
	chunkNodeLog2 = log2i(sm_param.chunkSize);
	bufferListCount = log2i(chunkCount) + 1;


	size_t addr = reinterpret_cast<size_t>(originBuffer);
	size_t offset = ((addr + sm_param.chunkSize - 1) & ~(sm_param.chunkSize - 1)) - addr;
	buffer = originBuffer + offset;

	chunkHead = reinterpret_cast<ChunkNode *>(::malloc(chunkCount * sizeof(ChunkNode)));
	bufferList = reinterpret_cast<BufferList *> (::malloc(bufferListCount*sizeof(BufferList)));

	for (int i = 0; i < chunkCount; i++) {
		chunkHead[i].order = 0;
		chunkHead[i].idx = i;
		chunkHead[i].buf = buffer + (i<<chunkNodeLog2);
		chunkHead[i].pre = NULL;
		chunkHead[i].next = NULL;
	}


	int count = chunkCount;
	int tmpidx = 0;

	for ( int j = bufferListCount - 1; j >= 0; j--)
	{
			int tmpcount = count/(1<<j);
			ChunkNode * tmp;
			bufferList[j].head.order = j+1;
			bufferList[j].head.buf = NULL;
			bufferList[j].head.pre = &(bufferList[j].head);
			bufferList[j].head.next = &(bufferList[j].head);
			count -= tmpcount * (1<<j);
			tmp  = &(bufferList[j].head);
			while (tmpcount > 0) {
				chunkHead[tmpidx].order = j + 1;
				chunkHead[tmpidx].pre = tmp;
				chunkHead[tmpidx].next = tmp->next;
				tmp->next->pre =  &(chunkHead[tmpidx]);
				tmp->next = &(chunkHead[tmpidx]);
				tmp = &(chunkHead[tmpidx]);
				tmpidx += (1<<j);

				tmpcount--;
			}
	}


	Base::loginfo("PacketManager: malloc total(%d) pagesize(%d)"\
		"blockCount (%d) ManagermentSize(%d)\n", sm_param.totalSize, sm_param.chunkSize, chunkCount,
		chunkCount * sizeof(ChunkNode)+ bufferListCount*sizeof(BufferList));
	Base::loginfo("                AddrRange(%#x-%#x)\n",
					buffer, buffer + sm_param.totalSize);
}

PacketManagerInternal::~PacketManagerInternal()
{
	if(originBuffer)
	{
#if defined( __i386__ ) || defined( __x86_64__ )
		::shm_unlink(sm_sharestring.c_str());
#elif defined(_WIN32) || defined(WIN32)
 		UnmapViewOfFile((LPCTSTR)originBuffer);   
#endif
	}
	
	if (bufferList)
		::free((char *) bufferList);
	if (chunkHead)
		::free((char *)chunkHead);

}

void PacketManagerInternal::PrintNodeInfo()
{
	mutex.enter();
	for (int i = 0; i < bufferListCount; i++) {
		Base::loginfo("BufferList id:%d, order(%d) head(%#x), headpre(%#x)next(%#x)\n",
			i, bufferList[i].head.order, &(bufferList[i].head),  bufferList[i].head.pre, bufferList[i].head.next);
		ChunkNode *p = bufferList[i].head.next;
		int j = 0;
		while (p != &(bufferList[i].head)) {
			Base::loginfo("bufflist(%d)id: %d, realIdx(%d)self(%#x)order(%d) pre(%#x), next(%#x) buf:(%#x)\n", i, j, p->idx,p,p->order, p->pre,
				p->next, p->buf);
			p = p->next;
		}
	}
	mutex.leave();


}

void PacketManagerInternal::config(PoolParameter* param, const char *sharememname)
{
	if (param)
	{
		assert(param->chunkSize >= 1024 && param->chunkSize > 0 && param->totalSize % param->chunkSize == 0);
		assert(param->alignSize >= 4);
		sm_param = *param;
	}
	if (sharememname)
	{
		sm_sharestring = sharememname;
	}
}


void PacketManagerInternal::deleteNode(ChunkNode *node)
{
	node->order = 0;
	node->pre->next = node->next;
	node->next->pre = node->pre;
}
void PacketManagerInternal::insertNode(ChunkNode *node, int idx)
{
	node->order = idx + 1;
	node->pre = &(bufferList[idx].head);
	node->next = bufferList[idx].head.next;
	bufferList[idx].head.next->pre = node;
	bufferList[idx].head.next = node;
}

int PacketManagerInternal::split(int startIdx, int endIdx)
{
	int idx = bufferList[startIdx].head.next->idx;
	deleteNode(bufferList[startIdx].head.next);
	if (startIdx > endIdx) {
		int size =  1 << (startIdx - 1);
		do
		{
			startIdx--;
			insertNode(&(chunkHead[idx]), startIdx);
			idx += size;
			size >>= 1;
		}while (startIdx > endIdx);
	}
	chunkHead[idx].order = -(endIdx + 1);

	return idx;
}

void PacketManagerInternal::merge(int id,int order11)
{
	mutex.enter();

	int order = -(chunkHead[id].order);
	int bubbyIdx  = 0;
	while (order < bufferListCount)
	{
		bubbyIdx = FindBubbyIdx(id, order);

		if ( bubbyIdx >= chunkCount || chunkHead[bubbyIdx].order != order)
			break;
		deleteNode(&chunkHead[bubbyIdx]);
		id = FindCombileIdx(id, order);
		order++;

	}
	insertNode(&chunkHead[id],order - 1);
	mutex.leave();

	return;

}


PacketInternal* PacketManagerInternal::getPacket(uint32_t length , int extraLen)
{
	size_t aligned_info_size = (extraLen + alignSize - 1) & ~(alignSize - 1);
	size_t len = length + aligned_info_size;
	uint8_t* p = reinterpret_cast<uint8_t*>(Malloc(&len));
	if (p)
	{
		PacketInternal *pPacket = new PacketInternal;
		pPacket->size = length;
		pPacket->buffer = p;
		pPacket->extraLen = extraLen;
		pPacket->length = 0;
		return pPacket;
	}
	else
	{
		Base::loginfo("PacketManager: NO enough, need size(%d)ext(%d)\n",
			length,extraLen);

		return NULL;
	}
}

void PacketManagerInternal::putPacket(PacketInternal* internal)
{
	void* p = internal->buffer;
	delete internal;
	Free(p);
}

/// 从管理的内存中申请内存块
/// \param size [in, out] 传入需要分配的大小，传出实际分配的大小
void* PacketManagerInternal::Malloc(size_t* size)
{
	int vecIdx = Base::log2i(((*size  - 1)>>(chunkNodeLog2 - 1)));

	int idx = vecIdx;
	mutex.enter();
	while(idx  < bufferListCount) {
		if (bufferList[idx].head.next != &(bufferList[idx].head)) {
			break;
		} else {
			idx++;
		}
	}
	if (idx >= bufferListCount) {

		mutex.leave();
		Base::loginfo("Packet No Free Space\n");
		return NULL;
	}

	idx = split(idx, vecIdx);

	mutex.leave();

	*size = ((1 << (vecIdx))<<chunkNodeLog2);
	return chunkHead[idx].buf;
}

/// 释放内存块
void PacketManagerInternal::Free(void* ptr)
{
	int id = (reinterpret_cast<uint8_t*>(ptr) - buffer)>>chunkNodeLog2;

	merge(id, chunkHead[id].order);

	return ;
}


//返回缓冲的大小,以Bytes为单位
uint32_t PacketManagerInternal::getBufferSize()
{
	return sm_param.totalSize;
}

uint32_t PacketManagerInternal::getFreeSize()
{
	mutex.enter();
	int size = 0;
	int i = 0;

	for (i = 0;i < bufferListCount; i++) {
		int tmp = 0;
		ChunkNode *p = bufferList[i].head.next;
		while ( p != &(bufferList[i].head))
		{
			tmp++;
			p = p->next;
		}

		size += (1<<i)*tmp;
	}
	mutex.leave();
	return size << chunkNodeLog2;
}

void PacketManagerInternal::dumpNodes()
{
	Base::loginfo("PacketManager: dump FreeList\n");
	PrintNodeInfo();
	return ;

}

#endif


} // namespace Base
} // namespace Public


