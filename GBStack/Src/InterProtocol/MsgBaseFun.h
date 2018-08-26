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


//������չ��Ϣ
void MsgBuildExtendInfo(const std::vector<std::string>& vExtendInfo, ostringstream& oss);

void MsgBuildExtendInfo(const std::map<std::string,std::string>& vExtendInfo, ostringstream& oss);

//����������Ϣ
int MsgParse(TiXmlDocument& xml, string& strXml, int &nSn, string& strDeviceId);

//����������Ϣ
int MsgParse(TiXmlDocument& xml, string& strXml, int &nSn, string& strDeviceId, vector<string>* pExtendInfo);

int MsgParse(TiXmlDocument& xml, string& strXml, int &nSn, string& strDeviceId, map<string,string>* pExtendInfo);
//�������
int MsgParseResult(TiXmlElement* pElement, bool& bResult);
//����״̬
#define MsgParseStatus MsgParseResult


//ʮ�������ַ�ת��������
char HexChar2Num(char cHex);
//����ת����ʮ�������ַ�
char Num2HexChar(char cNum);

