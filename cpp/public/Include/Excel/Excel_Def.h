#ifndef __XUNMEI_EXCEL_DEFINE_H__
#define __XUNMEI_EXCEL_DEFINE_H__
#include "Base/Base.h"

using namespace Xunmei::Base;

// WIN32 Dynamic Link Library
#ifdef _MSC_VER

#ifdef EXCEL_DLL_BUILD
#define  EXCEL_API _declspec(dllexport)
#else
#define  EXCEL_API _declspec(dllimport)
#endif

#else
#define EXCEL_API
#endif

namespace Xunmei {
namespace Excel {

//内容显示格式
typedef enum
{
	FomatType_GENERAL = 0,
	FomatType_NUMBER1,            // 0
	FomatType_NUMBER2,            // 0.00
	FomatType_NUMBER3,            // #,##0
	FomatType_NUMBER4,            // #,##0.00
	FomatType_CURRENCY1,          // "$"#,##0_);("$"#,##0)
	FomatType_CURRENCY2,          // "$"#,##0_);[Red]("$"#,##0)
	FomatType_CURRENCY3,          // "$"#,##0.00_);("$"#,##0.00)
	FomatType_CURRENCY4,          // "$"#,##0.00_);[Red]("$"#,##0.00)
	FomatType_PERCENT1,           // 0%
	FomatType_PERCENT2,           // 0.00%
	FomatType_SCIENTIFIC1,        // 0.00E+00
	FomatType_FRACTION1,          // # ?/?
	FomatType_FRACTION2,          // # ??/??
	FomatType_DATE1,              // M/D/YY
	FomatType_DATE2,              // D-MMM-YY
	FomatType_DATE3,              // D-MMM
	FomatType_DATE4,              // MMM-YY
	FomatType_TIME1,              // h:mm AM/PM
	FomatType_TIME2,              // h:mm:ss AM/PM
	FomatType_TIME3,              // h:mm
	FomatType_TIME4,              // h:mm:ss
	FomatType_DATETIME,           // M/D/YY h:mm
	FomatType_ACCOUNTING1,        // _(#,##0_);(#,##0)
	FomatType_ACCOUNTING2,        // _(#,##0_);[Red](#,##0)
	FomatType_ACCOUNTING3,        // _(#,##0.00_);(#,##0.00)
	FomatType_ACCOUNTING4,        // _(#,##0.00_);[Red](#,##0.00)
	FomatType_CURRENCY5,          // _("$"* #,##0_);_("$"* (#,##0);_("$"* "-"_);_(@_)
	FomatType_CURRENCY6,          // _(* #,##0_);_(* (#,##0);_(* "-"_);_(@_)
	FomatType_CURRENCY7,          // _("$"* #,##0.00_);_("$"* (#,##0.00);_("$"* "-"??_);_(@_)
	FomatType_CURRENCY8,          // _(* #,##0.00_);_(* (#,##0.00);_(* "-"??_);_(@_)
	FomatType_TIME5,              // mm:ss
	FomatType_TIME6,              // [h]:mm:ss
	FomatType_TIME7,              // mm:ss.0
	FomatType_SCIENTIFIC2,        // ##0.0E+0
	FomatType_TEXT                // @
} FomatType;


//定义对齐方式,X和Y可以用于|
typedef enum 
{
	ALIGN_X_NONE = 0,//初始化
	ALIGN_X_LEFT,//左对齐
	ALIGN_X_CENTER ,//居中对齐
	ALIGN_X_RIGHT ,//右对齐
}ALIGN_X_Type;

typedef enum {
	ALIGN_Y_TOP,	//向上对齐
	ALIGN_Y_CENTER,		//居中对齐
	ALIGN_Y_BOTTOM,		//向下对齐	
}ALIGN_Y_Type;


//字体显示方式
typedef enum
{
	TxtoriType_NONE = 0,			//正常模式
	TxtoriType_TOPBOTTOMTXT,		//竖行，从上到下显示
	TxtoriType_90NOCLOCKTXT,		//竖行，向右显示
	TxtoriType_90CLOCKTXT			//竖行，向左显示
} TxtoriType;


//边框显示方式，可以|的方式
#define SIDE_BOTTOM		1			//底部边框
#define SIDE_TOP		2			//上部边框 
#define SIDE_LEFT		4			//左部边框
#define SIDE_RIGHT		8			//右部边框


}
}

#ifdef WIN32
#pragma warning(disable:4200)
#endif

#endif //__XUNMEI_EXCEL_H__
