#include "Base/SqlDataBase.h"
#include "Base/Value.h"
#include "Base/BaseTemplate.h"
namespace Public {
namespace Base {

struct SqlDB_Row::SqlDB_RowInternal
{
	std::map<int,Value>		FiledIndexList;
};

SqlDB_Row::SqlDB_Row()
{
	internal = new SqlDB_RowInternal();
}
SqlDB_Row::~SqlDB_Row()
{
	SAFE_DELETE(internal);
}
int SqlDB_Row::getColumnCount()
{
	return internal->FiledIndexList.size();
}
void SqlDB_Row::putFiled(int index, const Value& val)
{
	internal->FiledIndexList[index] = val;
}
const Value SqlDB_Row::getField(int index) const
{
	static Value tmp;
	std::map<int, Value>::const_iterator rIter = internal->FiledIndexList.find(index);
	if (rIter != internal->FiledIndexList.end())
	{
		return rIter->second;
	}

	return tmp;
}

} // namespace Base
} // namespace Public

