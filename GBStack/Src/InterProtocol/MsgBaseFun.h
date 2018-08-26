#pragma once

#include <vector>
#include <map>
#include <string>
#include <sstream>

using namespace std;

#define IMPLEMENT_BASE_FUN(type,FunName, CmdName) \
	inline EProtocolType GetProType(){return type;} \
	inline const char* GetFunName(){return FunName;} \
	inline const char* GetCmdName(){return CmdName;} \


//构造扩展信息
void MsgBuildExtendInfo(const std::vector<std::string>& vExtendInfo, ostringstream& oss);

void MsgBuildExtendInfo(const std::map<std::string,std::string>& vExtendInfo, ostringstream& oss);

//解析基础信息
int MsgParse(TiXmlDocument& xml, string& strXml, int &nSn, string& strDeviceId);

//解析基础信息
int MsgParse(TiXmlDocument& xml, string& strXml, int &nSn, string& strDeviceId, vector<string>* pExtendInfo);

int MsgParse(TiXmlDocument& xml, string& strXml, int &nSn, string& strDeviceId, map<string,string>* pExtendInfo);
//解析结果
int MsgParseResult(TiXmlElement* pElement, bool& bResult);
//解析状态
#define MsgParseStatus MsgParseResult


//十六进制字符转换成数字
char HexChar2Num(char cHex);
//数字转换成十六进制字符
char Num2HexChar(char cNum);

