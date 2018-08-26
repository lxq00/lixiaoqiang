#include "strCoding.h"
#include "Base/Base.h"


strCoding::strCoding(void)
{
}

strCoding::~strCoding(void)
{
}

void strCoding::UTF_8ToUnicode(wchar_t* pOut,char *pText)
{
	char* uchar = (char *)pOut;

	uchar[1] = ((pText[0] & 0x0F) << 4) + ((pText[1] >> 2) & 0x0F);
	uchar[0] = ((pText[1] & 0x03) << 6) + (pText[2] & 0x3F);

	return;
}

void strCoding::UnicodeToUTF_8(char* pOut, wchar_t* pText)
{
	// 注意 WCHAR高低字的顺序,低字节在前，高字节在后
	char* pchar = (char *)pText;

	pOut[0] = (0xE0 | ((pchar[1] & 0xF0) >> 4));
	pOut[1] = (0x80 | ((pchar[1] & 0x0F) << 2)) + ((pchar[0] & 0xC0) >> 6);
	pOut[2] = (0x80 | (pchar[0] & 0x3F));

	return;
}

//做为解Url使用
char strCoding:: CharToInt(char ch){
	if(ch>='0' && ch<='9')return (char)(ch-'0');
	if(ch>='a' && ch<='f')return (char)(ch-'a'+10);
	if(ch>='A' && ch<='F')return (char)(ch-'A'+10);
	return -1;
}
char strCoding::StrToBin(char *str){
	char tempWord[2];
	char chn;

	tempWord[0] = CharToInt(str[0]);                         //make the B to 11 -- 00001011
	tempWord[1] = CharToInt(str[1]);                         //make the 0 to 0 -- 00000000

	chn = (tempWord[0] << 4) | tempWord[1];                //to change the BO to 10110000

	return chn;
}


//UTF_8 转gb2312
void strCoding::UTF_8ToGB2312(string &pOut, char *pText, int pLen)
{
	pOut = utf82ansiEx(pText);
}

//GB2312 转为 UTF-8
void strCoding::GB2312ToUTF_8(string& pOut,char *pText, int pLen)
{
	pOut = ansi2utf8Ex(pText);
	return;
}
//把str编码为网页中的 GB2312 url encode ,英文不变，汉字双字节 如%3D%AE%88
string strCoding::UrlGB2312(char * str)
{
	string dd;
	size_t len = strlen(str);
	for (size_t i=0;i<len;i++)
	{
		if(isalnum((unsigned char)str[i]))
		{
			char tempbuff[2];
			sprintf(tempbuff,"%c",str[i]);
			dd.append(tempbuff);
		}
		else if (isspace((unsigned char)str[i]))
		{
			dd.append("+");
		}
		else
		{
			char tempbuff[4];
			sprintf(tempbuff,"%%%X%X",((unsigned char*)str)[i] >>4,((unsigned char*)str)[i] %16);
			dd.append(tempbuff);
		}

	}
	return dd;
}

//把str编码为网页中的 UTF-8 url encode ,英文不变，汉字三字节 如%3D%AE%88

string strCoding::UrlUTF8(char * str)
{
	string tt;
	string dd;
	GB2312ToUTF_8(tt,str,(int)strlen(str));

	size_t len=tt.length();
	for (size_t i=0;i<len;i++)
	{
		if(isalnum((unsigned char)tt.at(i)))
		{
			char tempbuff[2]={0};
			sprintf(tempbuff,"%c",(unsigned char)tt.at(i));
			dd.append(tempbuff);
		}
		else if (isspace((unsigned char)tt.at(i)))
		{
			dd.append("+");
		}
		else
		{
			char tempbuff[4];
			sprintf(tempbuff,"%%%X%X",((unsigned char)tt.at(i)) >>4,((unsigned char)tt.at(i)) %16);
			dd.append(tempbuff);
		}

	}
	return dd;
}
//把url GB2312解码
string strCoding::UrlGB2312Decode(string str)
{
	string output="";
	char tmp[2];
	int i=0,len=str.length();

	while(i<len){
		if(str[i]=='%'){
			tmp[0]=str[i+1];
			tmp[1]=str[i+2];
			output += StrToBin(tmp);
			i=i+3;
		}
		else if(str[i]=='+'){
			output+=' ';
			i++;
		}
		else{
			output+=str[i];
			i++;
		}
	}

	return output;
}
//把url utf8解码
string strCoding::UrlUTF8Decode(string str)
{
	string output="";

	string temp =UrlGB2312Decode(str);//

	UTF_8ToGB2312(output,(char *)temp.data(),strlen(temp.data()));

	return output;

}