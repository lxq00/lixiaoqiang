#include "MySql/DBConnectionMySql.h"
#include "Base/Base.h"
#include "include/mysql.h"
#include "include/errmsg.h"
using namespace Public::Base;
using namespace Public::MySql;

class SQLITEInitObject
{
public:
	SQLITEInitObject()
	{
		//初始化数据库  
		if (0 != mysql_library_init(0, NULL, NULL)) 
		{
			logwarn("mysql_library_init()failed");
		}
	}
	~SQLITEInitObject() 
	{
		mysql_library_end();
	}

	static void instance()
	{
		static SQLITEInitObject init;
	}
};

struct DBRecordsetMySql::DBRecordsetMySqlInternal
{
	MYSQL_RES*		res;
	Mutex*			mutex;

	DBRecordsetMySqlInternal():res(NULL){}
	~DBRecordsetMySqlInternal()
	{
		Guard locker(mutex);
		if(res != NULL)
		{
			mysql_free_result(res);
		}
	}
};

DBRecordsetMySql::DBRecordsetMySql()
{
	internal = new DBRecordsetMySqlInternal();
}
DBRecordsetMySql::~DBRecordsetMySql()
{
	SAFE_DELETE(internal);
}

shared_ptr<SqlDB_Row> DBRecordsetMySql::getRow()
{
	Guard locker(internal->mutex);
	if (internal->res == NULL)
	{
		return shared_ptr<SqlDB_Row>();
	}
	int nCol = mysql_num_fields(internal->res);
	MYSQL_ROW column = mysql_fetch_row(internal->res);
	if(column == NULL) 
	{
		return shared_ptr<SqlDB_Row>();
	}

	shared_ptr<SqlDB_Row> row = make_shared<SqlDB_Row>();
	for(int i = 0;column != NULL && i < nCol;i ++)
	{
		if (column[i] != NULL)
		{
			row->putFiled(i, column[i]);
		}
	}

	return row;
}

struct DBConnectionMySql::DBConnectionMySqlInternal
{
	MYSQL		  							mysql;
	Mutex									mutex;
	bool									connectstatus;

	std::string		hosts;
	unsigned int	port;
	std::string		userName;
	std::string		password;
	std::string		dbName;

	DBConnectionMySqlInternal():connectstatus(false){}
	~DBConnectionMySqlInternal()
	{
		_close();
	}
	bool _close()
	{
		if (connectstatus)
		{
			mysql_close(&mysql);
		}
		connectstatus = false;

		return true;
	}
	bool _connect()
	{
		_close();

		mysql_init(&mysql);
		if (mysql_real_connect(&mysql, hosts.c_str(), userName.c_str(), password.c_str(), dbName.c_str(), port, NULL, 0) == NULL)
		{
			mysql_close(&mysql);
			return false;
		}

		connectstatus = true;

		return true;
	}

	bool _exec(const std::string& sql)
	{
		int connecttimes = 0;

		while (1)
		{
			if (!connectstatus && connecttimes == 0)
			{
				_connect();
				connecttimes++;
			}
			if (!connectstatus && connecttimes != 0)
			{
				return false;
			}

			int ret = mysql_real_query(&mysql, sql.c_str(),sql.length());
			int eno = mysql_errno(&mysql);
			const char* tmp = mysql_error(&mysql);
			if (ret == 0)
			{
				break;
			}
			if (eno == CR_SERVER_GONE_ERROR || eno == CR_SERVER_LOST)
			{
				_close();
				continue;
			}
			else
			{
				return false;
			}
		}

		return true;
	}
};

DBConnectionMySql::DBConnectionMySql(const std::string& hosts, unsigned int port, const std::string& userName, const std::string& password, const std::string& dbName)
{
	SQLITEInitObject::instance();

	internal = new DBConnectionMySql::DBConnectionMySqlInternal;

	internal->hosts = hosts;
	internal->port = port;
	internal->userName = userName;
	internal->password = password;
	internal->dbName = dbName;
}

DBConnectionMySql::~DBConnectionMySql()
{
	disconnect();

	SAFE_DELETE(internal);
}

bool DBConnectionMySql::connect()
{
	Guard locker(internal->mutex);

	return internal->_connect();
}

bool DBConnectionMySql::disconnect()
{
	Guard locker(internal->mutex);

	return internal->_close();
}

bool DBConnectionMySql::exec(const std::string& sql, uint32_t* affectedrow)
{
	Guard locker(internal->mutex);

	bool ret = internal->_exec(sql);
	if (!ret) return false;

	if(affectedrow != NULL)
		*affectedrow = mysql_affected_rows(&internal->mysql);

	return ret;
}

bool DBConnectionMySql::exec(const std::vector<std::string>& sql, uint32_t* affectedrow)
{
	Guard locker(internal->mutex);

	if (!internal->connectstatus)
	{
		internal->_connect();
	}
	if (!internal->connectstatus)
	{
		return false;
	}
	
	if (!mysql_autocommit(&internal->mysql, false))
	{
		return false;
	}


	for(unsigned int i = 0;i < sql.size();i ++)
	{
		internal->_exec(sql[i]);
	}

	mysql_commit(&internal->mysql);
	mysql_autocommit(&internal->mysql, true);

	if (affectedrow != NULL)
		*affectedrow = mysql_affected_rows(&internal->mysql);

	return true;
}


shared_ptr<SqlDB_Recordset> DBConnectionMySql::query(const std::string& sql)
{
	Guard locker(internal->mutex);

	if (!internal->_exec(sql))
	{
		return shared_ptr<SqlDB_Recordset>();
	}

	shared_ptr<DBRecordsetMySql> set = make_shared<DBRecordsetMySql>();
	set->internal->mutex = &internal->mutex;
	set->internal->res = mysql_store_result(&internal->mysql);
	if (set->internal->res == NULL)
	{
		return shared_ptr<SqlDB_Recordset>();
	}

	return set;
}

uint32_t DBConnectionMySql::count(const std::string& sql)
{
	shared_ptr<SqlDB_Recordset> set = query(sql);

	if (set == NULL)
	{
		return 0;
	}

	shared_ptr<SqlDB_Row> rowset = set->getRow();
	if (rowset != NULL)
	{
		return 0;
	}

	return rowset->getField(0).readUint32();
}

