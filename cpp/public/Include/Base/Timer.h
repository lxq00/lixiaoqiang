//
//  Copyright (c)1998-2012, Xunmei Technology
//  All Rights Reserved.
//
//
//	Description:
//	$Id: Timer.h 3 2013-01-21 06:57:38Z jiangwei $


#ifndef _BASE_TIMER_H_
#define _BASE_TIMER_H_

#include "Base/IntTypes.h"
#include "Defs.h"
#include "Func.h"

namespace Xunmei{
namespace Base{


/// \class Timer
/// \brief ��ƽ̨��ʱ����֧����ʱ�����ڡ��첽�ȶ��ֹ���ִ�з�ʽ

struct TimerInternal;

class BASE_API Timer
{
	/// ��ֹ�������캯���͸�ֵ����
	Timer(Timer const&);
	Timer& operator=(Timer const&);

public:
	typedef Function1<void, unsigned long> Proc;

	/// ���캯��
	/// \param name [in] ��ʱ������
	Timer(const char * name);

	/// ��������
	virtual ~Timer();

	/// ������ʱ��
	/// \param fun 		[in]	��ʱ���ص�����
	/// \param delay 	[in]	ָ����������ʱ����ʱ�����,��λΪ����,���Ϊ0��ʾ������ʼ����
	/// \param period [in]	��ʱ��������,ָ�����ϴε��ö���ʱ����ٴε���,��λΪ����,
	///		   ���Ϊ0��ʾ�Ƿ����ڶ�ʱ��,��һ�ε�����Ϻ���Զ�ֹͣ
	/// \param param 	[in]	�ص���������,�ڻص��������ǻᴫ���ص�����
	/// \param timeout �ص�����ִ�еĳ�ʱʱ��,���ʱ��ᱻ���ø��ص�ʱ�Ķ�ʱ���߳�,
	///        ����Ϊ��λ. 0��ʾ������ʱ,Ĭ��ֵΪ1����Ҳ����60��
	/// \reval false �ڷ���ʱ�Ķ�ʱ�����ϴε��û�û����ɵ������
	/// \reval true  �������
	/// \note ����ʱ���������ڲ���������Ϊ0,����ʵ���첽����
	bool start(const Proc& fun, uint32_t delay, uint32_t period, unsigned long param = 0, uint32_t timeout = 60000);

	/// �رն�ʱ��
	/// \param callNow [in]	��ʱ��ֹͣʱͬʱ�ٵ���һ�»ص�������ֻ�Դ���ʱ�ķ����ڶ�ʱ����Ч��
	/// \retval false ��ʱ��û�п���������µ���
	/// \retval ture  �ɹ�
	bool stop(bool callNow = false);

	/// �õ���ʱ������
	/// \retval ��ʱ������
	const char* getName();

	/// ���ö�ʱ������
	/// \param name [in]	�µĶ�ʱ������
	void setName(const char* name);

	/// �ж϶�ʱ���Ƿ���
	/// \reval true �Ѿ�����
	/// \reval false û�п���
	/// \note �����ڶ�ʱ���ڵ��ù�����Զ���״̬�ر�
	bool isStarted();

	/// �жϷ����ڶ�ʱ���Ƿ��Ѿ����ù�
	/// \reval true �Ѿ�����
	/// \reval false û�е���
	bool isCalled();

	/// �жϻص������Ƿ�����ִ��
	/// \retval true ����ִ��
	/// \retval false û��ִ��
	bool isRunning();

	/// �رն�ʱ�����ȴ�,ֱ���ص����������ŷ���
	/// \retval true �ɹ�
	/// \retval false ʧ��
	/// \note һ�����û���������ʱ�����,����Ҫ�ر�С��,��ֹ����
	bool stopAndWait();

private:
	TimerInternal* internal;
};


////////////////////////////////////////////////////////////////////////////////


/// \class TimerManager
/// \brief ��ʱ�������࣬ʹ�ø߾���ϵͳ��ʱ��������Ӧ�ö�ʱ������
struct TimerManagerInternal;

class BASE_API TimerManager
{
	TimerManager(TimerManager const&);
	TimerManager& operator=(TimerManager const&);

public:
	TimerManager();
	/// ������ʱ���������
	static TimerManager* instance();
	
	/// ��������
	~TimerManager();

	/// ��ӡ�����߳���Ϣ
	void dumpTimers();


	void uninit();

	TimerManagerInternal* internal;
};

#define gTimerManager (*TimerManager::instance())

} // namespace Base
} // namespace Xunmei

#endif


