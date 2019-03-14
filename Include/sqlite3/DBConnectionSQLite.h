#ifndef __DBRECORDSETSQLITE_H__
#define __DBRECORDSETSQLITE_H__
#include "Defs.h"
#include "Base/Base.h"
using namespace Public::Base;

namespace Public {
namespace Sqlite {

class SQLITE_API DBRecordsetSQLite :public SqlDB_Recordset
{
	friend class DBConnectionSQLite;
	struct DBRecordsetSQLiteInternal;
public:
	DBRecordsetSQLite();
	~DBRecordsetSQLite();
	shared_ptr<SqlDB_Row> getRow();
private:
	DBRecordsetSQLiteInternal* internal;
};
class SQLITE_API DBConnectionSQLite:public SqlDB_Connection
{
	struct DBConnectionSQLiteInternal;
public:
	DBConnectionSQLite(const std::string& dbname, bool create);
	virtual ~DBConnectionSQLite();

	bool connect();
	bool disconnect();
	bool exec(const std::string& sql, uint32_t* affectedrow = NULL);
	bool exec(const std::vector<std::string>& sql, uint32_t* affectedrow = NULL);

	shared_ptr<SqlDB_Recordset> query(const std::string& sql);
	uint32_t count(const std::string& sql);
private:
	DBConnectionSQLiteInternal* internal;
};

}
}

#endif //__DBRECORDSET_H__
