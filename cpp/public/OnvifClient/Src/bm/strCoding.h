#pragma once

#include <iostream>
#include <string>
#ifdef WIN32
#include <windows.h>
#endif
using namespace std;

class strCoding
{
public:
	strCoding(void);
	~strCoding(void);

	void UTF_8ToGB2312(string &pOut, char *pText, int pLen);//utf_8转为gb2312
	void GB2312ToUTF_8(string& pOut,char *pText, int pLen); //gb2312 转utf_8
	string UrlGB2312(char * str);                           //urlgb2312编码
	string UrlUTF8(char * str);                             //urlutf8 编码
	string UrlUTF8Decode(string str);                  //urlutf8解码
	string UrlGB2312Decode(string str);                //urlgb2312解码

private:
	void UTF_8ToUnicode(wchar_t* pOut,char *pText);
	void UnicodeToUTF_8(char* pOut, wchar_t* pText);
	void UnicodeToGB2312(char* pOut, wchar_t uData);
	char CharToInt(char ch);
	char StrToBin(char *str);

};