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
/// \brief 时间记录类，用于测试统计多个采样点之间的时间差
class TimeRecord
{
	/// 时间采样点结构
	typedef struct
	{
		const char *name;
		uint64_t us;
	} TimePoint;

public:

	/// 构造函数
	/// \parma name [in] 名称
	/// \param size	[in] 预计采样点个数，实际采样次数应小于预设值
	TimeRecord(const char* name, int size) : name(name), size(size), pos(0)
	{
		points = new TimePoint[size];
	};

	/// 析构函数
	~TimeRecord()
	{
		delete []points;
	};

	/// 清空采样记录
	void reset()
	{
		pos = 0;
	};

	/// 执行采样
	/// \param name [in]采样点名称标记
	void sample(const char* name)
	{
		points[pos].name = name;
		points[pos].us = Base::Time::getCurrentMicroSecond();
		pos++;
	}

	/// 执行最后一次采样，并统计采样结果
	/// \param timeout [in] 超时后打印，单位微妙，0表示总是打印
	void stat(uint32_t timeout = 0);

private:
	TimePoint* points;
	const char* name;
	int size;
	int pos;
};

/// 统计打印结果的接口
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


