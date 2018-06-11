#ifndef __DBRECORDSET_H__
#define __DBRECORDSET_H__
#include "DBRecordsetValueTrans.h"
#include "Base/Base.h"

using namespace Public::Base;

class RowRecordSet
{
public:
	RowRecordSet(){}
	~RowRecordSet(){}
	int getColumnCount()
	{
		return FiledIndexList.size();
	}
	template<typename T>
	void putFiled(int index,T val)
	{
		FiledRecord* rval = putRecordTrans<T>(val);

		FiledIndexList[index] = shared_ptr<FiledRecord>(rval);
	}
	template<typename T>
	T getField(int index)
	{
		shared_ptr<FiledRecord> val;

		std::map<int, shared_ptr<FiledRecord> >::iterator rIter = FiledIndexList.find(index);
		if(rIter != FiledIndexList.end())
		{
			val = rIter->second;
		}

		return getRecordTrans<T>((FiledRecord*)val.get());
	}
private:
	std::map<int,shared_ptr<FiledRecord> >		FiledIndexList;
};

class DBConnectionSQLite;

class DBRecordset
{
	friend class DBConnectionSQLite;
	struct DBRecordsetInternal;
public:
	DBRecordset();
	~DBRecordset();
	shared_ptr<RowRecordSet> getRow();
private:
	DBRecordsetInternal* internal;
};


#endif //__DBRECORDSET_H__
