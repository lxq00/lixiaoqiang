#ifndef __DBRECORDSETSQLITE_H__
#define __DBRECORDSETSQLITE_H__
#include "DBRecordset.h"

class DBConnectionSQLite
{
	struct DBConnectionSQLiteInternal;
public:
	DBConnectionSQLite();
	virtual ~DBConnectionSQLite();

	bool connect(const std::string& dbname,bool create);
	bool disconnect();
	bool exec(const std::string& sql,bool transaction = false);
	bool exec(const std::vector<std::string>& sql);

	shared_ptr<DBRecordset> query(const std::string& sql);
	bool exists(const std::string& sql);
private:
	DBConnectionSQLiteInternal* internal;
};


#endif //__DBRECORDSET_H__
