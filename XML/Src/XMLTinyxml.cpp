#include "XMLTinyxml.h"
#ifdef WIN32
#include <windows.h>
#else
#include <errno.h>
#include <string.h>
#include <iconv.h>
#endif
#include <stdarg.h>
#include <stdio.h>
#include <map>
#ifdef WIN32
#define strcasecmp _stricmp
#endif
namespace Public{
namespace XML{

// void  printXMLLibVersion()
// {
// 	printf("*************************************************\r\n");
// 	printf("%s Version:%d.%d.%d svn:%s Built in %s\r\n", "XML Lib", r_major, r_minor, r_build, versionalias, __DATE__);
// 	printf("*************************************************\r\n\r\n");
// }
void parseTiXmlElementAttribute(TiXmlElement* pElement,XMLObject::Child& child)
{
	if(pElement == NULL)
	{
		return;
	}
	
	TiXmlAttribute* attribute = pElement->FirstAttribute();
	while(attribute != NULL)
	{
		child.setAttribute(attribute->Name(),attribute->Value());

		attribute = attribute->Next();
	}
}

void parseTiXmlElementAndAddChild(TiXmlNode* pNode,XMLObject::Child& child)
{
	if(pNode == NULL)
	{
		return;
	}

	TiXmlNode* pChildNode = pNode->FirstChild();
	while(pChildNode != NULL)
	{
		if(pChildNode->Type() == TiXmlNode::TEXT)
		{
			child.setValue(pChildNode->Value());
		}
		else if(pChildNode->Type() == TiXmlNode::ELEMENT)
		{
			XMLObject::Child& subchild = child.addChild(pChildNode->Value());
			parseTiXmlElementAttribute(pChildNode->ToElement(),subchild);
			parseTiXmlElementAndAddChild(pChildNode,subchild);
		}
		pChildNode = pChildNode->NextSibling();
	}
}

void buildTiXmlElementFormChild(XMLObject::Child& child,TiXmlElement* pElement,XMLObject::Encoding old,XMLObject::Encoding encode)
{
	if(child.isEmpty())
	{
		return;
	}
	if(child.attributeCount() == 0 && child.childCount() == 0)
	{
		TiXmlText* childElement = new TiXmlText(child.getValue().toString().c_str());

		pElement->LinkEndChild(childElement);
		return;
	}

	XMLObject::Attribute atti = child.firstAttribute();
	while(!atti.isEmpty())
	{
		pElement->SetAttribute(buildVaildXmlString(atti.name,old,encode).c_str(),buildVaildXmlString(atti.value.toString(),old,encode).c_str());

		atti = child.nextAttribute();
	}

	XMLObject::Child subchild = child.firstChild();
	while(!subchild.isEmpty())
	{
		TiXmlElement* childElement = new TiXmlElement(buildVaildXmlString(subchild.getName(),old,encode).c_str());
		if(childElement == NULL)
		{
			return;
		}

		buildTiXmlElementFormChild(subchild,childElement,old,encode);

		pElement->LinkEndChild(childElement);
		subchild = child.nextChild();
	}
}

void findVersionAndEncoding(const std::string& xml,std::string& ver,XMLObject::Encoding& encode)
{
	const char* tmpbuf = xml.c_str();
	do
	{
		const char* tmp = strchr(tmpbuf,'\r');
		if(tmp == NULL)
		{
			break;
		}

		std::string buffer(tmpbuf,tmp-tmpbuf);
		const char* tmpstart = strstr(buffer.c_str(),"<?");
		if(tmpstart == NULL)
		{
			tmpbuf = tmp + 1;
			continue;
		}
		const char*tmpver = strstr(buffer.c_str(),"version");
		if(tmpver == NULL)
		{
			break;
		}
		const char* verstart = strchr(tmpver,'"');
		if(verstart == NULL)
		{
			break;
		}
		verstart += 1;
		char verbuf[256];
		int index = 0;
		while(*verstart != '"' && index < 256)
		{
			verbuf[index] = *verstart;
			index ++;
			verstart ++;
		}
		verbuf[index] = 0;
		ver = verbuf;

		
		const char*tmpcoding = strstr(buffer.c_str(),"encoding");
		if(tmpcoding == NULL)
		{
			break;
		}
		const char* codingstart = strchr(tmpcoding,'"');
		if(codingstart == NULL)
		{
			break;
		}
		codingstart += 1;
		char codingbuf[256];
		int codingindex = 0;
		while(*codingstart != '"' && codingindex < 256)
		{
			codingbuf[codingindex] = *codingstart;
			codingindex ++;
			codingstart ++;
		}
		codingbuf[codingindex] = 0;
		if(strcasecmp(codingbuf,"gb2312") == 0 || strcasecmp(codingbuf,"gbk") == 0)
		{
			encode = XMLObject::Encoding_GBK;
		}
		else if(strcasecmp(codingbuf,"utf-8") == 0)
		{
			encode = XMLObject::Encoding_UTF8;
		}
		break;
	}while(1);
}


int ansi2utf8(const char *inbuf,size_t inlen,char *outbuf,size_t outlen)
{
#ifdef WIN32
	int len = MultiByteToWideChar(CP_ACP, 0, inbuf, inlen, NULL, 0);
	wchar_t *wstr = new wchar_t[len + 1];
	int tmplen  = MultiByteToWideChar(CP_ACP, 0,inbuf, inlen, wstr, len);
	len = WideCharToMultiByte(CP_UTF8, 0, wstr, tmplen, NULL, 0, NULL, NULL);
	if ((int)outlen < len)
	{
		delete []wstr;
		return -1;
	}
	WideCharToMultiByte(CP_UTF8, 0, wstr, tmplen,outbuf, len, NULL, NULL);
	delete []wstr;
	return len;
#else
	iconv_t cd;
	char **pin = (char **)&inbuf;
	char **pout = &outbuf;
	cd = iconv_open("utf-8","gb2312");
	if (cd == 0) return -1;
	size_t tmp = outlen;
	if (iconv(cd,pin,&inlen,pout,&outlen) == (size_t)-1) 
	{
		iconv_close(cd);
		return -1;
	}
	iconv_close(cd);
	return (tmp - outlen);
#endif
}

int utf82ansi(const char *inbuf,size_t inlen,char *outbuf,size_t outlen)
{
#ifdef WIN32
	int len = MultiByteToWideChar(CP_UTF8, 0, inbuf, inlen, NULL, 0);
	wchar_t *wstr = new wchar_t[len + 1];
	
	int lentmp = MultiByteToWideChar(CP_UTF8, 0,inbuf, inlen, wstr, len);
	len = WideCharToMultiByte(CP_ACP, 0, wstr, lentmp, NULL, 0, NULL, NULL);
	if ((int)outlen < len)
	{
		delete []wstr;
		return -1;
	}
	WideCharToMultiByte(CP_ACP, 0, wstr, lentmp,outbuf, len, NULL, NULL);
	delete []wstr;
	return len;
#else
	iconv_t cd;
	char **pin = (char **)&inbuf;
	char **pout = &outbuf;
	
	cd = iconv_open("gb2312","utf-8");
	if (cd == 0) return -1;
	size_t tmp = outlen;
	if (iconv(cd,pin,&inlen,pout,&outlen) == (size_t)-1) 
	{
		iconv_close(cd);
		return -1;
	}
	iconv_close(cd);
	return (tmp - outlen);
#endif

}

std::string buildVaildXmlString(const std::string& val,XMLObject::Encoding old,XMLObject::Encoding encode)
{
	if(old == XMLObject::Encoding_Unknown)
	{
		old = XMLObject::Encoding_GBK;
	}
	if(encode == XMLObject::Encoding_Unknown)
	{
		encode = old;
	}

	if(old == encode)
	{
		return val;
	}

	if(old == XMLObject::Encoding_UTF8)
	{
		int bufferlen = val.length() * 3 + 100;
		char* buffer = new char[bufferlen];

		int len = utf82ansi(val.c_str(),val.length(),buffer,bufferlen);
		buffer[len] = 0;

		std::string newstr(buffer);

		delete []buffer;

		return newstr;
	}
	else
	{
		int bufferlen = val.length()  + 100;
		char* buffer = new char[bufferlen];

		int len = ansi2utf8(val.c_str(),val.length(),buffer,bufferlen);
		buffer[len] = 0;

		std::string newstr(buffer);

		delete []buffer;

		return newstr;
	}
}

}
}
