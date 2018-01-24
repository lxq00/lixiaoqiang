//
//  Copyright (c)1998-2012, Xunmei Technology
//  All Rights Reserved.
//
//	Description:
//	$Id: TimeRecord.h 3 2013-01-21 06:57:38Z jiangwei $


#ifndef __BASE_TIME_RECORD_H__
#define __BASE_TIME_RECORD_H__

#include <stdio.h>
#include "Base/IntTypes.h"
#include "Base/Time.h"


namespace Xunmei{
namespace Base {


/// \class TimeRecord
/// \brief ʱ���¼�࣬���ڲ���ͳ�ƶ��������֮���ʱ���
class TimeRecord
{
	/// ʱ�������ṹ
	typedef struct
	{
		const char *name;
		uint64_t us;
	} TimePoint;

public:

	/// ���캯��
	/// \parma name [in] ����
	/// \param size	[in] Ԥ�Ʋ����������ʵ�ʲ�������ӦС��Ԥ��ֵ
	TimeRecord(const char* name, int size) : name(name), size(size), pos(0)
	{
		points = new TimePoint[size];
	};

	/// ��������
	~TimeRecord()
	{
		delete []points;
	};

	/// ��ղ�����¼
	void reset()
	{
		pos = 0;
	};

	/// ִ�в���
	/// \param name [in]���������Ʊ��
	void sample(const char* name)
	{
		points[pos].name = name;
		points[pos].us = Base::Time::getCurrentMicroSecond();
		pos++;
	}

	/// ִ�����һ�β�������ͳ�Ʋ������
	/// \param timeout [in] ��ʱ���ӡ����λ΢�0��ʾ���Ǵ�ӡ
	void stat(uint32_t timeout = 0);

private:
	TimePoint* points;
	const char* name;
	int size;
	int pos;
};

/// ͳ�ƴ�ӡ����Ľӿ�
inline void TimeRecord::stat(uint32_t timeout)
{
	sample(NULL);
	if(!timeout || points[pos - 1].us - points[0].us >= timeout)
	{
		for(int i = 1; i < pos; i++)
		{
			printf("%s-Per-%s-%s : %d us\n", name, points[i - 1].name, points[i].name, (i == 0) ? 0 : (int)(points[i].us - points[i-1].us));
		}
	}
};


} // namespace Base
} // namespace Xunmei

#endif // __BASE_TIME_RECORD_H__


