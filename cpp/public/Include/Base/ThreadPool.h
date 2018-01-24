//
//  Copyright (c)1998-2012, Public Technology
//  All Rights Reserved.
//
//	Description:
//	$Id: ThreadPool.h 3 2013��8��21��10:18:30 lixiaoqiang $

#ifndef __BASE_THREADPOOL_H__
#define __BASE_THREADPOOL_H__

#include "Defs.h"
#include "Base/Mutex.h"
#include "Base/Thread.h"
#include "Base/Callback.h"

#include <list>
#include <map>

using namespace std;

namespace Public{
namespace Base{

///�̳߳�ִ�лص�����
typedef Function1<void, void*> ThreadPoolHandler;

/// \class TreadPool
/// \brief ��ƽ̨�̳߳���
class BASE_API ThreadPool
{
public:
	struct ThreadPoolInternal;
	typedef enum 
	{
		Type_Manager,	//����ģʽ���ڲ�������dispathcer�أ�Dispatch��������Ϣ������У����̻߳�ȡ��Ϣ�����̹߳���
		Type_Normal,	//��ͨģʽ��Dispatch�ӿڴ����߳�ִ���߳�
	}Type;
public:
	/// ���������������������̳߳�
	/// param[in] type    �̳߳ع���ģʽ
	/// param[in] maxDiapathcerSize ��type == Type_Manager��dispathcer������Ÿ���
	/// param[in] liveTime �߳�ִ�к���д��ʱ��/0��ʾִ�к��Զ��رգ�< 0��ʾһֱ����һֱ����  ��λ��
	ThreadPool(Type type = Type_Normal,uint32_t maxDiapathcerSize = 64,uint32_t liveTime = 60);
	~ThreadPool(); 

	///�̳߳�ִ���º����ӿ�
	/// param func[in] ��Ҫִ�еĺ���ָ��
	/// param param[n] ��Ҫִ�иú���ָ��Ĳ���
	/// param [out] 0�ɹ���-1ʧ��
	bool Dispatch(const ThreadPoolHandler& func,void* param);

	void uninit();
private:
	ThreadPoolInternal* internal;
};

#define gThreadPool (*(ThreadPool::instance()))

};//Base
};//Public



#endif //__BASE_THREADPOOL_H__

