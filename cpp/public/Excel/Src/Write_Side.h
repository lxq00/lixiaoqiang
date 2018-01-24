#ifndef __WRITE_SIDE_H__
#define __WRITE_SIDE_H__
//#include "Write_Workbook.h"
#include "Excel/Excel.h"

namespace Public {
namespace Excel {

struct XM_Excel::Side::LineInternal
{
	LineInternal():side(0) {}
	int				  side;
	shared_ptr<Color> color;
};

}
}
#endif //__WRITE_SIDE_H__