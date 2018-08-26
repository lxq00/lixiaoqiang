/**
 * 公共的基础头文件
 */
#pragma once
#include <sstream>
#include <stdio.h>
#include <assert.h>
#include <time.h>
#include "Format.h"
#include "GBStack/GBTypes.h"
#include "GBStack/GBStructs.h"

//#define TIXML_USE_STL
#include "XML/tinyxml.h"
#include "MsgBaseFun.h"

#ifndef __INTERPROTOCOL_PUBLIC_H__
#define __INTERPROTOCOL_PUBLIC_H__

//#define WRITE_ERROR		printf
#define WRITE_ERROR(...) printf("```````````` parse error:%s %s %d````````````\r\n",__FILE__,__FUNCTION__,__LINE__)


//定义NULL
#ifndef NULL
#	define NULL 0
#endif

#endif //__INTERPROTOCOL_PUBLIC_H__

