#pragma once

#include "Base/Defs.h"
#include "Base/IntTypes.h"
#include "Base/Time.h"
#include "Base/Func.h"
#include "Base/Value.h"
#include <string>
using namespace std;

namespace Public {
namespace Base {

class BASE_API SqlDB_Row
{
public:
	SqlDB_Row();
	~SqlDB_Row();
	int getColumnCount();
	void putFiled(int index, const Value& val);
	const Value getField(int index) const;
private:
	struct SqlDB_RowInternal;
	SqlDB_RowInternal* internal;
};

class BASE_API SqlDB_Recordset
{
public:
	SqlDB_Recordset() {}
	~SqlDB_Recordset() {}
	virtual shared_ptr<SqlDB_Row> getRow() = 0;
};

class BASE_API SqlDB_Connection
{
public:
	SqlDB_Connection() {}
	virtual ~SqlDB_Connection() {}

	virtual bool connect() = 0;
	virtual bool disconnect() = 0;
	virtual bool exec(const std::string& sql,uint32_t* affectedrow = NULL) = 0;
	virtual bool exec(const std::vector<std::string>& sql, uint32_t* affectedrow = NULL) = 0;

	virtual shared_ptr<SqlDB_Recordset> query(const std::string& sql) = 0;
	virtual uint32_t count(const std::string& sql) = 0;
};


} // namespace Base
} // namespace Public

