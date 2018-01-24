//
//  Copyright (c)1998-2014, Public Technology
//  All Rights Reserved.
//
//
//	Description:
//	$Id: Time.h 226 2013-10-29 09:10:03Z linyang $


#ifndef __BASE_TIME_H__
#define __BASE_TIME_H__

#include <string>
#include "Defs.h"
#include "IntTypes.h"


namespace Public {
namespace Base {



/// 时间结构体
typedef struct
{
	int  year;		///< 年。
	int  month;		///< 月，January = 1, February = 2, and so on.
	int  day;		///< 日。
	int  wday;		///< 星期，Sunday = 0, Monday = 1, and so on
	int  hour;		///< 时。
	int  minute;	///< 分。
	int  second;	///< 秒。
	int  timeZone;	 //表示本地时间与UTC时间的差、单位分钟，如中国+08:00区 为 +480
}SystemTime;

/// \class Time
/// \brief 时间类
class BASE_API Time : public SystemTime
{
public:
	/// 日期顺序格式
	enum DateFormat
	{
		ymd,
		mdy,
		dmy
	};

	/// 格式化模式选项
	enum FormatMask
	{
		fmGeneral = 0,		///< 只指定是否显示以及起始值
		fmSeparator = 1,	///< 指定分隔符
		fmDateFormat = 2,	///< 指定年月日顺序
		fmHourFormat = 4,	///< 指定小时制式

		fmAll = fmSeparator | fmDateFormat | fmHourFormat	///< 指定所有格式选项
	};

	/// 缺省构造函数
	Time();

	/// 相对时间构造函数
	/// \param time [in] 相对时间
	Time(uint64_t time);

	/// 普通时间构造函数
	/// \param vyear 	[in] 年
	/// \param vmonth [in] 月
	/// \param vday 	[in]日期
	/// \param vhour 	[in] 小时
	/// \param vmin [in] 分
	/// \param vsec [in] 秒
	/// \param tzdiff[in] 与UTC产生的时间差、默认为正八区故为-8*60
	Time(int vyear, int vmonth, int vday, int vhour, int vmin, int vsec);

	/// 得到UTC时间，相对时间指从GMT 1970-1-1 00:00:00 到某个时刻经过的秒数
	/// \retval 相对时间
	uint64_t makeTime() const;

	/// 分解UTC时间
	/// \param time [in] 相对时间
	void breakTime(uint64_t time);

	/// 时间调节
	/// \param seconds [in] 调节的秒数
	/// \retval 调节后新的时间对象
	Time operator +( int64_t seconds ) const;

	/// 时间调节
	/// \param seconds [in] 调节的秒数
	/// \retval 调节后新的时间对象
	Time operator -( int64_t seconds ) const;

	/// 时间差运算
	/// \param time [in] 相比较的时间
	/// \retval 比较的结果，为(*this - time)得到的秒数
	int64_t operator -( const Time& time ) const;

	/// 时间调节
	/// \param seconds [in] 调节的秒数
	/// \retval 对象本身
	Time& operator +=( int64_t seconds );

	/// 时间调节
	/// \param seconds [in] 调节的秒数
	/// \retval 对象本身
	Time& operator -=( int64_t seconds );

	/// 时间比较
	/// \param time  [in]相比较的时间
	/// \retval true 相等，
	/// \retval false 不等
	bool operator == (const Time& time) const;

	/// 时间比较
	/// \param time  [in]相比较的时间
	/// \retval true 不等
	/// \retval false 相等
	bool operator != (const Time& time) const;

	/// 时间比较
	/// \param time  [in] 相比较的时间
	/// \retval true 小于 
	/// \retval false 不小于
	bool operator < (const Time& time) const;

	/// 时间比较
	/// \param time  [in] 相比较的时间
	/// \retval true 小于等于
	/// \retval false 大于
	bool operator <= (const Time& time) const;

	/// 时间比较
	/// \param time  [in] 相比较的时间
	/// \retval true 大于
	/// \retval false 不大于
	bool operator > (const Time& time) const;

	/// 时间比较
	/// \param time [in] 相比较的时间
	/// \retval true 大于等于
	/// \retval false 小于
	bool operator >= (const Time& time) const;

	/// 时间格式化
	/// \param buf  	[in]字符串缓冲，一定要足够大
	/// \param format [in] 格式化字符串，如"yyyy-MM-dd HH:mm:ss tt"
	///									yy = 年，不带世纪 yyyy = 年，带世纪
	/// 								M = 非0起始月 MM = 0起始月 MMMM = 月名称
	/// 								d = 非0起始日 dd = 0起始日
	/// 								H = 非0起始24小时 HH = 0起始24小时 h = 非0起始12小时 hh = 0起始12小时
	/// 								tt = 显示上午或下午
	/// \param mask 	[in] 格式选项，指定日期分隔符，年月日顺序，小时制式是否由统一的格
	///									式决定。相应位置0，表示使用统一格式，置1，表示使用format指定的格式
	void format(char* buf, const char *format = "yyyy-MM-dd HH:mm:ss", int mask = fmGeneral) const;

	/// 时间字符串解析
	/// \param buf   [out] 输入的字符串缓冲
	/// \param format [in] 格式化字符串，暂时只支持默认的"yyyy-MM-dd HH:mm:ss"
	/// \param mask [in] 格式选项，指定日期分隔符，年月日顺序，小时制式是否由统一的格
	///        					式决定。相应位置0，表示使用统一格式，置1，表示使用format指定的格式
	/// \retval  true 成功
	/// \retval  false 失败
	bool parse(const char* buf, const char *format = "yyyy-MM-dd HH:mm:ss", int mask = fmGeneral);


	//解析VGSII uri 中定义的时间格式
	bool parseVGSIITimeString(const std::string& timeStr);
	//生成VGSII uri中定义的格式时间
	std::string buildVGSIITimeString() const;
	
	/// 得到本地当前系统时间
	/// \retval Time对象
	static Time getCurrentTime();

	/// 设置本地当前系统时间
	/// \param time [in] 新的时间
	/// \param toleranceSeconds [in] 容差，表示容许设置时间和当前差多少秒内不做修改
	static void setCurrentTime(const Time& time, int toleranceSeconds = 0);

	/// 得到从系统启动到现在的毫秒数
	/// \retval 当前的毫秒时间
	/// \notice 如果比较时间，不能把getCurrentMilliSecond和 getCurrentMicroSecond的值进行对比
	static uint64_t getCurrentMilliSecond();

	/// 得到从系统启动到现在的微秒数
	/// \retval  	当前的微秒时间
	/// \notice 	如果比较时间，不能把getCurrentMilliSecond和 getCurrentMicroSecond的值进行对比
	static uint64_t getCurrentMicroSecond();

	/// 设置时间格式，会影响Format的输出的字符串格式，如"yyyy-MM-dd HH:mm:ss"
	///  \param format [in] 格式
	static void setFormat(const std::string& format);

	/// 获取时间格式
	///  \retval 时间格式的字符串
	static const std::string getFormat();

	/// 获取日期格式
	/// \retval 日期格式
	static DateFormat getDateFormat();

	/// 获取当前是否是12小时制
	/// \retval true 是12小时制
	/// \retval false 不是12小时制
	static bool get12Hour();

	/// 获取日期分割符
	/// \retval 分割符
	static char getSeparator();

	static void convertSystemTimeToTime(SystemTime &sys, Time& tm);
};

} // namespace Base
} // namespace Public

#endif // __BASE_TIME_H__


