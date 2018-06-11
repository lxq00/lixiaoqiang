/**
 * 格式语句定义文件
 */
#pragma once



//xml头
#define XML_HEAD \
"<?xml version=\"1.0\" encoding=\"gb2312\" ?>\r\n"

//一级简易元素
#define XML_ELEMENT1(name,value) \
	"<"#name">" << (value) << "</"#name">\r\n"

//二级简易元素
#define XML_ELEMENT2(name,value) \
	"<"#name">" << (value) << "</"#name">\r\n"

//三级简易元素
#define XML_ELEMENT3(name,value) \
	"<"#name">" << (value) << "</"#name">\r\n"

//命令类型
#define XML_CMD_TYPE(ct)		XML_ELEMENT1(CmdType, ct)
//sn元素
#define XML_SN(sn)				XML_ELEMENT1(SN,sn)
//设备id元素
#define XML_DEVICE_ID(id)		XML_ELEMENT1(DeviceID,id)

//结果转换成字符串
#define RESULT_TO_STR(s)		((s)?"OK":"ERROR")
//状态转换成字符串
#define STATUS_TO_STR(s)		((s)?"ON":"OFF")
//#define STATUS_TO_STR RESULT_TO_STR
