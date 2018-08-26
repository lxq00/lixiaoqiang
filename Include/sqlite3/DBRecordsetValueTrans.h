#ifndef __DBREORDSETVAUETRANS_H__
#define __DBREORDSETVAUETRANS_H__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <map>
#include <set>
#include "Base/Base.h"
using namespace Public::Base;
using namespace std;

enum RecordType
{
	RecordType_Int,
	RecordType_String,
	RecordType_Float,
};

///数据库值的存储
struct FiledRecord
{
	FiledRecord():type(RecordType_Int)
	{
		memset(&value,0,sizeof(value));
	}
	FiledRecord(const FiledRecord& record)
	{
		type = record.type;
		memcpy(&value,&record.value,sizeof(value));
		if(type == RecordType_String)
		{
			if(record.value.sval != NULL)
			{
				value.sval = new char[strlen(record.value.sval) + 1];
				strcpy(value.sval,record.value.sval);
			}
		}
	}
	~FiledRecord()
	{
		if(type == RecordType_String)
		{
			SAFE_DELETEARRAY(value.sval);
		}
	}
	RecordType		type;		///数据库值本事的类型
	union{
		char*		sval;
		double		fval;
		long long	ival;
	}value;
};
template<typename T>
inline FiledRecord* putRecordTrans(T value)
{
	FiledRecord* val = new FiledRecord();
	
	return val;
}

template<>
inline FiledRecord* putRecordTrans(int64_t value)
{
	FiledRecord* val = new FiledRecord();

	val->type = RecordType_Int;
	val->value.ival = value;

	return val;
}

template<>
inline FiledRecord* putRecordTrans(uint64_t value)
{
	FiledRecord* val = new FiledRecord();

	val->type = RecordType_Int;
	val->value.ival = value;

	return val;
}

template<>
inline FiledRecord* putRecordTrans(uint32_t value)
{
	FiledRecord* val = new FiledRecord();

	val->type = RecordType_Int;
	val->value.ival = value;

	return val;
}

template<>
inline FiledRecord* putRecordTrans(int value)
{
	FiledRecord* val = new FiledRecord();

	val->type = RecordType_Int;
	val->value.ival = value;

	return val;
}

template<>
inline FiledRecord* putRecordTrans(float value)
{
	FiledRecord* val = new FiledRecord();


	val->type = RecordType_Float;
	val->value.fval = value;

	return val;
}

template<>
inline FiledRecord* putRecordTrans(double value)
{
	FiledRecord* val = new FiledRecord();

	val->type = RecordType_Float;
	val->value.fval = (float)value;

	return val;
}


template<>
inline FiledRecord* putRecordTrans(bool value)
{
	FiledRecord* val = new FiledRecord();

	val->type = RecordType_Int;
	val->value.ival = value;

	return val;
}

template<>
inline FiledRecord* putRecordTrans(std::string value)
{
	FiledRecord* val = new FiledRecord();

	val->type = RecordType_String;
	val->value.sval = new char[value.size() + 1];
	strcpy(val->value.sval,value.c_str());

	return val;
}

template<>
inline FiledRecord* putRecordTrans(const char* value)
{
	return putRecordTrans<std::string>(value);
}

template<>
inline FiledRecord* putRecordTrans(const unsigned char* value)
{
	return putRecordTrans<const char*>((const char*)value);
}
//template<>
//inline FiledRecord* putRecordTrans(Time::TM value)
//{
//	FiledRecord* val = new FiledRecord();
//			
//	val->type = RecordType_DateTime;
//	val->value.tval = Time::mktime(&value);
//
//	return val;
//}

template<typename T>
inline T getRecordTrans(FiledRecord* val)
{
	return T();
}

template<>
inline int64_t getRecordTrans(FiledRecord* val)
{
	if(val == NULL)
	{
		return 0;
	}
	if(val->type == RecordType_Int)
	{
		return val->value.ival;
	}
	else if(val->type == RecordType_Float)
	{
		return (uint64_t)val->value.fval;
	}
	else if(val->type == RecordType_String && val->value.sval != NULL)
	{
		uint64_t ival = 0;

		sscanf(val->value.sval,"%llu",(long long unsigned int*)&ival);

		return ival;
	}

	return 0;
}

template<>
inline uint64_t getRecordTrans(FiledRecord* val)
{
	if(val == NULL)
	{
		return 0;
	}
	if(val->type == RecordType_Int)
	{
		return val->value.ival;
	}
	else if(val->type == RecordType_Float)
	{
		return (uint64_t)val->value.fval;
	}
	else if(val->type == RecordType_String && val->value.sval != NULL)
	{
		uint64_t ival = 0;

		sscanf(val->value.sval,"%llu",(long long unsigned int*)&ival);

		return ival;
	}

	return 0;
}


template<>
inline int getRecordTrans(FiledRecord* val)
{
	if(val == NULL)
	{
		return 0;
	}
	if(val->type == RecordType_Int)
	{
		return (int)val->value.ival;
	}
	else if(val->type == RecordType_Float)
	{
		return (int)val->value.fval;
	}
	else if(val->type == RecordType_String && val->value.sval != NULL)
	{
		int ival = 0;

		sscanf(val->value.sval,"%d",&ival);

		return ival;
	}

	return 0;
}

template<>
inline uint32_t getRecordTrans(FiledRecord* val)
{
	if(val == NULL)
	{
		return 0;
	}
	if(val->type == RecordType_Int)
	{
		return (uint32_t)val->value.ival;
	}
	else if(val->type == RecordType_Float)
	{
		return (uint32_t)val->value.fval;
	}
	else if(val->type == RecordType_String && val->value.sval != NULL)
	{
		uint32_t ival = 0;

		sscanf(val->value.sval,"%u",&ival);

		return ival;
	}

	return 0;
}

template<>
inline float getRecordTrans(FiledRecord* val)
{
	if(val == NULL)
	{
		return 0;
	}
	if(val->type == RecordType_Int)
	{
		return (float)val->value.ival;
	}
	else if(val->type == RecordType_Float)
	{
		return (float)val->value.fval;
	}
	else if(val->type == RecordType_String && val->value.sval != NULL)
	{
		return (float)atof(val->value.sval);
	}
	
	return 0;
}

template<>
inline double getRecordTrans(FiledRecord* val)
{
	if(val == NULL)
	{
		return 0;
	}
	if(val->type == RecordType_Int)
	{
		return (double)val->value.ival;
	}
	else if(val->type == RecordType_Float)
	{
		return (double)val->value.fval;
	}
	else if(val->type == RecordType_String && val->value.sval != NULL)
	{
		return atof(val->value.sval);
	}
	
	return 0;
}

template<>
inline bool getRecordTrans(FiledRecord* val)
{
	if(val == NULL)
	{
		return false;
	}
	if(val->type == RecordType_Int)
	{
		return val->value.ival != 0;
	}
	else if(val->type == RecordType_Float)
	{
		return val->value.fval != 0;
	}
	else if(val->type == RecordType_String && val->value.sval != NULL)
	{
		return atof(val->value.sval) != 0;
	}

	return false;
}

template<>
inline std::string getRecordTrans(FiledRecord* val)
{
	if(val == NULL)
	{
		return "";
	}
	char tmp[64] = {0};
	if(val->type == RecordType_Int)
	{
		sprintf(tmp,"%llu",val->value.ival);
	}
	else if(val->type == RecordType_Float)
	{
		sprintf(tmp,"%f",val->value.fval);
	}
	else if(val->type == RecordType_String)
	{
		return val->value.sval;
	}
	
	return tmp;
}

//template<>
//inline Time::TM getRecordTrans(FiledRecord* val)
//{
//	Time::TM tm = {0};
//	if(val == NULL)
//	{
//		return tm;
//	}
//
//	if(val->type == RecordType_String)
//	{
//		if(val->value.sval != NULL)
//		{
//			sscanf(val->value.sval,"%04d-%02d-%02d %02d:%02d:%02d",&tm.year,&tm.mon,&tm.yday,&tm.hour,&tm.min,&tm.sec);
//		}
//	}
//	else if(val->type == RecordType_DateTime)
//	{
//		Time::TM* gmtime = Time::gmtime(val->value.tval);
//		if(gmtime != NULL)
//		{
//			tm = *gmtime;
//		}
//	}
//	
//	return tm;
//}


#endif //__DBREORDSETVAUETRANS_H__
