#include "sqlite3/DBRecordsetSQLite.h"
#include "Base/Base.h"
#include "sqlite3.h"
using namespace Public::Base;

struct DBRecordset::DBRecordsetInternal
{
	sqlite3_stmt*	stmt3;
	Mutex*			mutex;

	DBRecordsetInternal():stmt3(NULL),mutex(NULL){}
	~DBRecordsetInternal()
	{
		if(stmt3 != NULL && mutex != NULL)
		{
			Guard locker(mutex);
			sqlite3_finalize(stmt3);
		}
	}
};

DBRecordset::DBRecordset()
{
	internal = new DBRecordsetInternal();
}
DBRecordset::~DBRecordset()
{
	SAFE_DELETE(internal);
}

shared_ptr<RowRecordSet> DBRecordset::getRow()
{
	Guard locker(internal->mutex);

	if(sqlite3_step(internal->stmt3) != SQLITE_ROW)
	{
		return shared_ptr<RowRecordSet>();
	}

	shared_ptr<RowRecordSet> set = make_shared<RowRecordSet>();

	int clouns = sqlite3_column_count(internal->stmt3);

	for(int i = 0;i < clouns;i ++)
	{
		int ret = sqlite3_column_type(internal->stmt3,i); 

		switch(ret)
		{
		case SQLITE_INTEGER:
			{
				uint64_t filedval = sqlite3_column_int64(internal->stmt3,i);
				set->putFiled<uint64_t>(i,filedval);
			}
			break;
		case SQLITE_TEXT:
			{						
				const unsigned char* filedval = sqlite3_column_text(internal->stmt3,i);

				if (filedval != NULL)
				{	
					set->putFiled<std::string>(i,(char*)filedval);
				}											
			}
			break;
		case SQLITE_FLOAT:
			{
				double  filedval = sqlite3_column_double(internal->stmt3,i);
				set->putFiled<double>(i,filedval);					
			}
			break;
		default:
			break;				
		}
	}

	return set;
}

struct DBConnectionSQLite::DBConnectionSQLiteInternal
{
	sqlite3*		  			sqlhand;
	Mutex						mutex;

	bool beginTransaction()
	{
		return exec("begin transaction");
	}
	bool commitTransaction()
	{
		return exec("commit transaction");
	}
	bool exec(const std::string& sql)
	{
		char*		errmsg;

		int ret = sqlite3_exec(sqlhand,sql.c_str(),NULL,NULL,&errmsg);
		if(ret != 0)
		{
			logerror("sqlite3_exec:%s error:%s\r\n",sql.c_str(),errmsg);
		}

		sqlite3_free(errmsg);


		return ret == 0;
	}
};

DBConnectionSQLite::DBConnectionSQLite()
{
	internal = new DBConnectionSQLite::DBConnectionSQLiteInternal;

	internal->sqlhand = NULL;
}

DBConnectionSQLite::~DBConnectionSQLite()
{
	disconnect();

	delete internal;
}

bool DBConnectionSQLite::connect(const std::string& dbname,bool create)
{
	Guard locker(internal->mutex);

	if(!create && !File::access(dbname.c_str(),File::accessExist))
	{
		return false;
	}
	if(internal->sqlhand != NULL)
	{
		return  false;
	}
	int	ret = sqlite3_open(String::ansi2utf8(dbname).c_str(),&internal->sqlhand);

	if(ret != 0)
	{
		std::string errstring = sqlite3_errmsg(internal->sqlhand);
		internal->sqlhand = NULL;
		logerror("sqlite3_open %s error %s\r\n",dbname.c_str(),errstring.c_str());
	}
	return ret == 0;
}

bool DBConnectionSQLite::disconnect()
{
	Guard locker(internal->mutex);

	if (internal->sqlhand)
	{
		sqlite3_close(internal->sqlhand);
		internal->sqlhand = NULL;
	}

	return true;
}

bool DBConnectionSQLite::exec(const std::string& sql)
{
	Guard locker(internal->mutex);

	if(internal->sqlhand == NULL)
	{
		return false;
	}

	return internal->exec(sql);
}

bool DBConnectionSQLite::exec(const std::vector<std::string>& sql)
{
	Guard locker(internal->mutex);

	if(internal->sqlhand == NULL)
	{
		return false;
	}

	if(!internal->beginTransaction())
	{
		return false;
	}

	for(unsigned int i = 0;i < sql.size();i ++)
	{
		internal->exec(sql[i]);
	}

	return internal->commitTransaction();
}


shared_ptr<DBRecordset> DBConnectionSQLite::query(const std::string& sql)
{
	Guard locker(internal->mutex);

	if(internal->sqlhand == NULL)
	{
		return shared_ptr<DBRecordset>();
	}
	
	shared_ptr<DBRecordset> set = make_shared<DBRecordset>();
	set->internal->mutex = &internal->mutex;


	if(sqlite3_prepare_v2(internal->sqlhand,/*ensql*/sql.c_str(),-1,&set->internal->stmt3,NULL) != 0)
	{
		return shared_ptr<DBRecordset>();
	}

	return set;
}

bool DBConnectionSQLite::exists(const std::string& sql)
{
	if(internal->sqlhand == NULL)
	{
		return false;
	}

	shared_ptr<DBRecordset> set = query(sql);

	if(set == NULL)
	{
		return false;
	}

	shared_ptr<RowRecordSet> rowset = set->getRow();
	if(rowset != NULL)
	{
		return true;
	}

	return false;
}

