//
//  Copyright (c)1998-2012,  Public Technology
//  All Rights Reserved.
//
//	Description:
//	$Id: Time.cpp 226 2013-10-29 09:10:03Z  $
//

#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <assert.h>
#include <string.h>
#include <algorithm>
#include "Base/PrintLog.h"
#include "Base/System.h"
#include "Base/Time.h"

#ifdef WIN32
	#include <Windows.h>
	#include <stdio.h>
#ifndef snprintf
#define snprintf _snprintf
#endif
#elif defined(__linux__)
	#include <sys/time.h>
	#include <sys/times.h>
	#include <sys/select.h>
    #include <linux/unistd.h>
#ifndef _SC_CLK_TCK
	#define sysconf
	#define _SC_CLK_TCK 100
#endif

#elif defined(__TCS__)
	#include <psos.h>
	#include <pna.h>
	#define KC_TICKS2SEC 100
#else
	#error "Unknown Platform"
#endif



namespace
{

#ifdef WIN32
static double ddFreq;
static double ddBaseTime;
class WinTimeStarter // make something happen for windows
{
public:
	WinTimeStarter()
	{
		LARGE_INTEGER c, f;
		QueryPerformanceFrequency(&f);
		ddFreq = (double)f.QuadPart / 1000000;
		QueryPerformanceCounter(&c);
		ddBaseTime = (double)c.QuadPart / ddFreq;
	}
}starter;
#endif

using namespace Public;
using namespace Public::Base;

static std::string s_format = "yyyy-MM-dd HH:mm:ss";

static Time::DateFormat s_dateFormat = Time::ymd;
static char s_dateSeparator = '-';
static bool s_12hour = false;


/// 每月的天数
static int mday[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

/// 从1月1日起至本月1日的天数
static int monthdays[12] =
{
	0,
	31, //1
	31+28, //2
	31+28+31, //3
	31+28+31+30, //4
	31+28+31+30+31, //5
	31+28+31+30+31+30, //6
	31+28+31+30+31+30+31, //7
	31+28+31+30+31+30+31+31, //8
	31+28+31+30+31+30+31+31+30, //9
	31+28+31+30+31+30+31+31+30+31, //10
	31+28+31+30+31+30+31+31+30+31+30, //11
};

/// 是否闰年
inline bool is_leap_year(int year)
{
	return ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0);
}

/// 日期规范化
inline void normalize_date(int& year, int& month, int& day)
{
	if (year < 1970)
	{
		year = 1970;
	}
	else if (year > 2038)
	{
		year = 2038;
	}

	if (month <= 0)
	{
		month = 1;
	}
	else if (month > 12)
	{
		month = 12;
	}

	if (day <= 0)
	{
		day = 1;
	}
	else if (day > mday[month - 1])
	{
		if (!(month == 2 && day == 29 && is_leap_year(year)))
		{
			day = mday[month - 1];
		}
	}
}

/// 时间规范化
inline void normalize_time1(int& hour, int& minute, int& second,int& tzdiff)
{
	do 
	{
		//仅允许24:00:00的特殊时间
		if (hour == 24 && minute == 0 && second == 0)
		{
			break;
		}

		if (hour < 0)
		{
			hour = 0;
		}
		else if (hour > 23)
		{
			hour = 23;
		}

		if (minute < 0)
		{
			minute = 0;
		}
		else if (minute > 59)
		{
			minute = 59;
		}

		if (second < 0)
		{
			second = 0;
		}
		else if (second > 59)
		{
			second = 59;
		}
	} while (false);

	if(tzdiff < -24*60)
	{
		tzdiff = -24*60;
	}
	else if(tzdiff > 24*60)
	{
		tzdiff = 24*60;
	}
}

/// 得到自1970-1-1起的总天数
inline int date_to_days(int year, int month, int day)
{
	// 每年365天 + 润年增加的天数 + 当年天数
	int leap_days = (year - 1968) / 4;
	int days = (year - 1970) * 365 + leap_days + monthdays[month - 1] + (day - 1);
	if (is_leap_year(year) && month < 3) //还没到闰月
	{
		--days;
	}

	return days;
}

/// 计算星期数
inline int date_to_weekday(int year, int month, int day)
{
	int leap_days = (year - 1968) / 4;
	int wdays = (year - 1970) + leap_days + monthdays[month - 1] + (day - 1) + 4;
	if (is_leap_year(year) && month < 3) //还没到闰月
	{
		--wdays;
	}

	return wdays % 7;
}


/// 把自1970-1-1起的天数转换成日期
inline void days_to_date(int days, int& year, int& month, int& day)
{
	year = days / 365 + 1970;
	days %= 365;

	int leap_days = (year - 1 - 1968) / 4;
	bool is_leap = false;
	if (days < leap_days)
	{
		--year;
		is_leap = is_leap_year(year);
		days += is_leap ? 366 : 365;
	}
	else
	{
		is_leap = is_leap_year(year);
	}

	days -= leap_days;

	for (month = 1; month <= 11; ++month)
	{
		if (month == 3)
		{
			if (is_leap)
			{
				if (days == monthdays[2])
				{
					month = 2;
					break;
				}
				days--;
			}
		}
		if (days < monthdays[month])
		{
			break;
		}
	}

	day = days - monthdays[month - 1] + 1;
}

/// 返回本地时间与 GMT 时间相差的秒数; 本地时间 + 相差的秒数 = GMT时间
inline int get_timezone()
{
	{
#if defined(_MSC_VER)
		TIME_ZONE_INFORMATION wtz;
		::GetTimeZoneInformation(&wtz);

		return -wtz.Bias;
#else
		struct timeval tv;
		struct timezone tz;
		if(::gettimeofday(&tv,&tz) < 0)
		{
			return 0;
		}

		return -tz.tz_minuteswest;
#endif
	}
}

} // namespace noname


namespace Public {
namespace Base {

/// UTC 转换为 SystemTime
void utc_to_time(uint64_t utc, SystemTime& time)
{
	uint64_t seconds = utc + time.timeZone*60;

	enum {DAY_SECONDS = 24 * 60 * 60};
	int days = int(seconds / DAY_SECONDS);
	days_to_date(days, time.year, time.month, time.day);

	int sec = int(seconds % DAY_SECONDS);
	time.hour = sec / 3600;
	sec %= 3600;
	time.minute = sec / 60;
	time.second = sec % 60;
	time.wday = date_to_weekday(time.year, time.month, time.day);
}

/// SystemTime 转换为 UTC
uint64_t time_to_utc(SystemTime const& time)
{
	int year = time.year, month = time.month, day = time.day, tzdif = time.timeZone;
	normalize_date(year, month, day);
	int days = date_to_days(year, month, day);

	int hour = time.hour, minute = time.minute, second = time.second;
	normalize_time1(hour, minute, second, tzdif);

	enum {DAY_SECONDS = 24 * 60 * 60};
	uint64_t seconds = uint64_t(days) * DAY_SECONDS + hour * 3600 + minute * 60 + second;
	return seconds - tzdif*60;
}

/// SystemTime 规范化 (值校验，生成星期)
void normalize_time(SystemTime& time)
{
	normalize_date(time.year, time.month, time.day);
	time.wday = date_to_weekday(time.year, time.month, time.day);
	normalize_time1(time.hour, time.minute, time.second,time.timeZone);
}


Time::Time()
{
}

Time::Time(uint64_t time)
{
	breakTime(time);
}

Time::Time(const SystemTime& sys) 
{
	*(SystemTime*)this = sys;
}

Time::Time( int vyear, int vmonth, int vday, int vhour, int vmin, int vsec)
{
	struct tm t;

	year = vyear;
	t.tm_year = year - 1900;
	month = vmonth;
	t.tm_mon = vmonth - 1;
	day = t.tm_wday = vday;
	hour = t.tm_hour = vhour;
	minute = t.tm_min = vmin;
	second = t.tm_sec = vsec;
	timeZone = get_timezone();
	
	// 值校验，规范化，生成星期
	normalize_time(*this);
}

Time Time::getCurrentTime()
{
	time_t localTime = ::time(NULL);
	
	return Time((uint64_t)localTime);
}

void Time::setCurrentTime(const Time& time, int toleranceSeconds)
{
	Time temp = time;
	static Time maxTimeSet(2037, 1, 1, 0, 0, 0);

	if(temp > maxTimeSet)
	{
		temp = maxTimeSet;
	}

	if(toleranceSeconds)
	{
		if(abs((int)(getCurrentTime() - temp)) <= toleranceSeconds)
		{
			return;
		}
	}
#ifdef WIN32
	SYSTEMTIME t;
	t.wMilliseconds = 0;
	t.wDayOfWeek = -1;

	t.wYear = temp.year;
	t.wMonth = temp.month;
	t.wDay = temp.day;
	t.wHour = temp.hour;
	t.wMinute = temp.minute;
	t.wSecond = temp.second;
	SetLocalTime(&t);

#elif defined(__linux__)
	struct timeval tv;

	tv.tv_sec = temp.makeTime();
	tv.tv_usec = 0;

	settimeofday(&tv, 0);

	// 同步到rtc
//	infof("Time::setCurrentTime to %04d-%02d-%02d %02d:%02d:%02d\n",
//			temp.year, temp.month, temp.day,
//			temp.hour, temp.minute, temp.second);
	if (SystemCall("hwclock -w") != 0)
	{
	//	warnf("Time::setCurrentTime(): hwclock -w failed\n");
	}
#endif
}

uint64_t Time::getCurrentMilliSecond()
{
#ifdef WIN32
	LARGE_INTEGER c;
	QueryPerformanceCounter(&c);
	return (int64_t)((((double)c.QuadPart / ddFreq) - ddBaseTime) / 1000);

//	return GetTickCount64(); //xp下不支持该函数
//	return GetTickCount(); //xp下要越界，不用
#elif defined(__linux__)
	struct tms t;
	static uint64_t curtick = 0x7fffffff; //当前tick计数，初始化为一个中间值，处理第一次溢出
	uint64_t tmptick = curtick;	// 先备份一下，避免多线程访问出错
	uint64_t tick = (uint)times(&t); // times必须使用非空参数，以兼容更多的平台
	if(tick == (uint)-1) // 溢出，取errno的值
	{
		tick = (uint)-errno;
	}

	if((uint)tmptick != (uint)tick) // 低32位变化说明tick变化
	{
		while(tick < tmptick) // 溢出处理
		{
			tick += 0xffffffff;
			tick += 1;
		}

		if(curtick < tick) // 提前处理多线程引起的时间倒流的问题，提高效率
		{
			curtick = tick;
		}
	}

	return curtick * (1000 / sysconf(_SC_CLK_TCK));
#else
#error "Unknown Platform"
#endif
}

uint64_t Time::getCurrentMicroSecond()
{
#ifdef WIN32
	LARGE_INTEGER c;
	QueryPerformanceCounter(&c);
	return (int64_t)(((double)c.QuadPart / ddFreq) - ddBaseTime);

#elif defined(__linux__)
	struct timeval tv;
	struct timezone	tz;
	gettimeofday(&tv, &tz);
	return ((int64_t)tv.tv_sec * 1000 * 1000 + tv.tv_usec);
#else
#error "Unknown Platform"
#endif
}

void Time::setFormat(const std::string& format)
{
	s_format = format;

	// 设置日期分隔符
	if(s_format.find('.') != s_format.npos)
	{
		s_dateSeparator = '.';
	}
	else if(s_format.find('/') != s_format.npos)
	{
		s_dateSeparator = '/';
	}
	else if(s_format.find('-') != s_format.npos)
	{
		s_dateSeparator = '-';
	}
	else
	{
		assert(0);
	}

	// 设置小时格式
	s_12hour = (s_format.find('h') != s_format.npos);

	// 设置日期格式
	size_t pos1 = s_format.find('y');
	size_t pos2 = s_format.find('M');
	size_t pos3 = s_format.find('d');
	if(pos1 < pos2 && pos2 < pos3)
	{
		s_dateFormat = ymd;
	}
	else if(pos2 < pos3 && pos3 < pos1)
	{
		s_dateFormat = mdy;
	}
	else if(pos3 < pos2 && pos2 < pos1)
	{
		s_dateFormat = dmy;
	}
	else
	{
		assert(0);
	}
}

const std::string Time::getFormat()
{
	return s_format;
}

Time::DateFormat Time::getDateFormat()
{
	return s_dateFormat;
}

bool Time::get12Hour()
{
	return s_12hour;
}

char Time::getSeparator()
{
	return s_dateSeparator;
}

uint64_t Time::makeTime() const
{
	return time_to_utc(*this);
}

void Time::breakTime(uint64_t time)
{
	time_t tt = (time_t)time; // linux下可能是32位，会溢出
	if(time != (uint64_t)tt)
	{
		logerror("Time::breakTime overflowed!\n");
	}
	timeZone = get_timezone();
	utc_to_time(time, *this);
	// 值校验，规范化，生成星期
	normalize_time(*this);
}

Time Time::operator +( int64_t seconds ) const
{
	Time time;
	time.breakTime(makeTime() + seconds);

	return time;
}

Time Time::operator -( int64_t seconds ) const
{
	Time time;
	time.breakTime(makeTime() - seconds);

	return time;
}

int64_t Time::operator -(const Time& time ) const
{
	return ((int64_t)makeTime() - (int64_t)time.makeTime());
}

Time& Time::operator += (int64_t seconds)
{
	breakTime(uint64_t((int64_t)makeTime() + seconds));

	return *this;
}

Time& Time::operator -= (int64_t seconds)
{
	breakTime(uint64_t((int64_t)makeTime() - seconds));

	return *this;
}

bool Time::operator == (const Time& time) const
{
	return (second == time.second) && (minute == time.minute) && (hour == time.hour)
		&& (day == time.day) && (month == time.month) && (year == time.year);
}

bool Time::operator != (const Time& time) const
{
	return !operator==(time);
}

bool Time::operator < (const Time& time) const
{
	return year < time.year ||
		(year == time.year && month < time.month) ||
		(year == time.year && month == time.month && day < time.day) ||
		(year == time.year && month == time.month && day == time.day && hour < time.hour) ||
		(year == time.year && month == time.month && day == time.day && hour == time.hour && minute < time.minute) ||
		(year == time.year && month == time.month && day == time.day && hour == time.hour && minute == time.minute && second < time.second);
}

bool Time::operator > (const Time& time) const
{
	return time.operator<(*this);
}

bool Time::operator >= (const Time& time) const
{
	return !operator<(time);
}

bool Time::operator <= (const Time& time) const
{
	return !operator>(time);
}

std::string Time::format(const std::string& format, int mask) const
{
	char temp[8] = {0};
	char tempYear[8] = {0};
	char tempMonth[8] = {0};
	char tempDay[8] = {0};
	const char* str[3] = {0};
	size_t size = format.length();

	char buf[512] = { 0 };

	// 日期字符串格式化
	int ny = 0, nm = 0, nd = 0;
	const char *p = format.c_str();
	while(*p)
	{
		if(*p == 'y')
		{
			ny++;
		}
		else if(*p == 'M')
		{
			nm++;
		}
		else if(*p == 'd')
		{
			nd++;
		}
		p++;
	}
	if(ny > 2)
	{
		//snprintf(tempYear, sizeof(tempYear) - 1, "%04d", year);
		tempYear[0] = year / 1000 + '0';
		tempYear[1] = year % 1000 / 100 + '0';
		tempYear[2] = year % 100 / 10 + '0';
		tempYear[3] = year % 10 + '0';
		tempYear[4] = 0;
	}
	else if(ny > 0)
	{
		//snprintf(tempYear, sizeof(tempYear) - 1, "%02d", year % 100);
		tempYear[0] = year % 100 / 10 + '0';
		tempYear[1] = year % 10 + '0';
		tempYear[2] = 0;
	}
	else
	{
		tempYear[0] = '\0';
	}

	if(nm > 0)
	{
		//snprintf(tempMonth, sizeof(tempMonth) - 1, (nm == 1) ? "%d" : "%02d", month);
		int si = 0;
		if (month >= 10 || nm > 1)
		{
			tempMonth[si++] = month / 10 + '0';
		}
		tempMonth[si++] = month % 10 + '0';
		tempMonth[si] = 0;
	}
	else
	{
		tempMonth[0] = '\0';
	}

	if(nd > 0)
	{
		//snprintf(tempDay, sizeof(tempDay) - 1, (nd == 1) ? "%d" : "%02d", day);
		int si = 0;
		if (day >= 10 || nd > 1)
		{
			tempDay[si++] = day / 10 + '0';
		}
		tempDay[si++] = day % 10 + '0';
		tempDay[si] = 0;
	}
	else
	{
		tempDay[0] = '\0';
	}

	// 日期字符串排序
	DateFormat df = s_dateFormat;
	if (mask & fmDateFormat)
	{
		char const* pos1 = std::find(format.c_str(), format.c_str() + size, 'y');
		char const* pos2 = std::find(format.c_str(), format.c_str() + size, 'M');
		char const* pos3 = std::find(format.c_str(), format.c_str() + size, 'd');

		if(pos1 < pos2 && pos2 < pos3)
		{
			df = ymd;
		}
		else if(pos2 < pos3 && pos3 < pos1)
		{
			df = mdy;
		}
		else if(pos3 < pos2 && pos2 < pos1)
		{
			df = dmy;
		}
		else
		{
			assert(0);
		}
	}

	if(df == ymd)
	{
		str[0] = tempYear;
		str[1] = tempMonth;
		str[2] = tempDay;
	}
	else if(df == mdy)
	{
		str[0] = tempMonth;
		str[1] = tempDay;
		str[2] = tempYear;
	}
	else if(df == dmy)
	{
		str[0] = tempDay;
		str[1] = tempMonth;
		str[2] = tempYear;
	}

	// 去掉无效的日期字符串
	if(str[0][0] == '\0')
	{
		str[0] = str[1];
		str[1] = str[2];
	}
	else if(str[1][0] == '\0')
	{
		str[1] = str[2];
	}

	// 12小时换算
	int is_12hour = (mask & fmHourFormat) ? (std::find(format.c_str(), format.c_str() + size, 'h') != format.c_str() + size) : s_12hour;
	int h = this->hour;
	if (is_12hour)
	{
		if(h > 12)
		{
			h -= 12;
		}
		else if(h == 0)
		{
			h = 12;
		}
	}

	// 扫描格式字符串，逐步格式化
	int istr = 0;
	for(size_t i = 0; i < size; i++)
	{
		size_t n = i;
		switch(format[i])
		{
		case '-':
		case '.':
		case '/':
			{
				char ds = (mask & fmSeparator) ? format[i] : s_dateSeparator;
				strncat(buf, &ds, 1);
			}
			break;

		case 'y':
		case 'M':
		case 'd':
			while(n < size && format[++n] == format[i]);
			strncat(buf, str[istr++], 4);
			i = n - 1;
			break;

		case 'H':
		case 'h':
			{
				while(n < size && (format[n] == 'h' || format[n] == 'H'))
					++n;
				//snprintf(temp, sizeof(temp) - 1, (n == i + 1) ? "%d" : "%02d", h);
				int si = 0;
				if (h >= 10 || n > i + 1)
				{
					temp[si++] = h / 10 + '0';
				}
				temp[si++] = h % 10 + '0';
				temp[si] = 0;
				strncat(buf, temp, 4);
				i = n - 1;

			}
			break;

		case 'm':
			{
				while(n < size && format[++n] == 'm');
				//snprintf(temp, sizeof(temp) - 1, (n == i + 1) ? "%d" : "%02d", minute);
				int si = 0;
				if (minute >= 10 || n > i + 1)
				{
					temp[si++] = minute / 10 + '0';
				}
				temp[si++] = minute % 10 + '0';
				temp[si] = 0;
				strncat(buf, temp, 4);
				i = n - 1;
			}
			break;

		case 's':
			{
				while(n < size && format[++n] == 's');
				//snprintf(temp, sizeof(temp) - 1, (n == i + 1) ? "%d" : "%02d", second);
				int si = 0;
				if (second >= 10 || n > i + 1)
				{
					temp[si++] = second / 10 + '0';
				}
				temp[si++] = second % 10 + '0';
				temp[si] = 0;
				strncat(buf, temp, 4);
				i = n - 1;
			}
			break;

		case 't':
			if (is_12hour)
			{
				while(n < size && format[++n] == 't');
				strncat(buf,hour/12?"PM":"AM", 2);
				i = n - 1;
			}
			else
			{
				n = strlen(buf);
				while(n > 0 && buf[n - 1] == ' ')// 删除多余的空格
				{
					buf[n - 1] = '\0';
					n--;
				}
			}
			break;

		default:
			strncat(buf, format.c_str() + i, 1);
			break;
		}
	}

	return buf;
}

bool Time::parse(const std::string& buf, const std::string& format, int mask)
{
	int n[6] = {0};	// 存放解析出的数字
	int count = 0;	// 解析出的数字个数
	char const* p = buf.c_str();
	while (count < 6)
	{
		while (*p >= '0' && *p <= '9')
		{
			n[count] = n[count] * 10 + (*p - '0');
			++p;
		}

		++count;

		while (*p != 0 && (*p < '0' || *p > '9'))
		{
			++p;
		}

		if (*p == 0)
		{
			break;
		}
	}

	//if (count == 6)
	{
		year = n[0];
		month = n[1];
		day = n[2];
		hour = n[3];
		minute = n[4];
		second = n[5];
		normalize_time(*this);
		return true;
	}

	return false;
}

} // namespace Base
} // namespace Public

