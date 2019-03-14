#ifndef __DBRECORDSETMYSQL_H__
#define __DBRECORDSETMYSQL_H__
#include "Defs.h"
#include "Base/Base.h"
using namespace Public::Base;

namespace Public {
namespace MySql {

class MYSQL_API DBRecordsetMySql:public SqlDB_Recordset
{
	friend class DBConnectionMySql;
	struct DBRecordsetMySqlInternal;
public:
	DBRecordsetMySql();
	~DBRecordsetMySql();
	shared_ptr<SqlDB_Row> getRow();
private:
	DBRecordsetMySqlInternal* internal;
};
class MYSQL_API DBConnectionMySql :public SqlDB_Connection
{
	struct DBConnectionMySqlInternal;
public:
	DBConnectionMySql(const std::string& hosts, unsigned int port, const std::string& userName, const std::string& password, const std::string& dbName);
	virtual ~DBConnectionMySql();

	bool connect();
	bool disconnect();
	bool exec(const std::string& sql, uint32_t* affectedrow = NULL);
	bool exec(const std::vector<std::string>& sql, uint32_t* affectedrow = NULL);

	shared_ptr<SqlDB_Recordset> query(const std::string& sql);
	uint32_t count(const std::string& sql);
private:
	DBConnectionMySqlInternal* internal;
};

}
}

#endif //__DBRECORDSET_H__
