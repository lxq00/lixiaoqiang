/**
 * 协议控制类头文件
 *
 * 用来将文本解析成结构体
 * 或者把结构体转化成字符串
 *
 * 单个对象不是线程安全的
 */

#pragma once


#include "GBStack/GBTypes.h"
#include "GBStack/GBStructs.h"
#include "InterProtocol/InterProtocol_Defs.h"

#define CONTENT_TYPE_MANSCDP_RTSP		"Application/MANSRTSP"
#define CONTENT_TYPE_MANSCDP_XML		"Application/MANSCDP+xml" 
#define CONTENT_TYPE_MANSCDP_SDP		"application/sdp"

//获取字符串的类型(无法识别E_PT_FILE_QUERY E_PT_FILE_RESULT)
INTERPROTOCOL_API int GetMsgType(string strXml, EProtocolType& eType, int* pSn=NULL);
// 将xml字符串解析成结构体
INTERPROTOCOL_API int ParseMsg(string strXml, EProtocolType eType, void* pData);


//构造字符串
INTERPROTOCOL_API int BuildMsg(EProtocolType eType, const void* pData, string& strXml);


