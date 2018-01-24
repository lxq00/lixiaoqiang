#include "Base/URLEncoding.h"
#include "Base/String.h"
using namespace std;

namespace Xunmei{
namespace Base{

bool NeedTrans(char ch)
{
	const char notneedtrans[] = "~!*()_-'.";

	if(ch >= 'a' && ch <= 'z')
	{
		return false;
	}
	if(ch >= 'A' && ch <= 'Z')
	{
		return false;
	}

	if(ch >= '0' && ch <= '9')
	{
		return false;
	}

	const char* tmp = strchr(notneedtrans,ch);
	if(tmp == NULL)
	{
		return true;
	}

	return false;
}

std::string URLEncoding::encode(const std::string& url)
{
	if(url.length() <= 0)
	{
		return "";
	}

	int utf8bufsize = url.length()*2 + 100;
	char* utf8buf = new char[utf8bufsize + 10];

	utf8bufsize = ansi2utf8((char*)url.c_str(),url.length(),utf8buf,utf8bufsize);
	if(utf8bufsize < 0)
	{
		delete []utf8buf;
		return "";
	}

	char* enbuf = new char[utf8bufsize*3 + 100];
	int enbufpos = 0;
	for(int i = 0;i < utf8bufsize;i ++)
	{
		if(NeedTrans(utf8buf[i]))
		{
			sprintf(&enbuf[enbufpos],"%%%02X",utf8buf[i]&0xff);
			enbufpos += 3;
		}
		else
		{
			enbuf[enbufpos] = utf8buf[i];
			enbufpos ++;
		}
	}
	enbuf[enbufpos] = 0;

	std::string enstr(enbuf);

	delete []enbuf;
	delete []utf8buf;

	return enstr;
}
std::string URLEncoding::decode(const std::string& enurl)
{
	if(enurl.length() <= 0)
	{
		return "";
	}

	int urlbuflen = enurl.length() + 100;
	char* urlbuf = new char[urlbuflen + 10];
	char* asiibuf = new char[urlbuflen + 10];

	int urlbufpos = 0;
	for(unsigned int i = 0;i < enurl.length();urlbufpos ++)
	{
		if(enurl.c_str()[i] == '%' && enurl.length() - i >= 3)
		{
			int val;
			sscanf(&enurl.c_str()[i],"%%%02x",&val);

			urlbuf[urlbufpos] = val;
			i += 3;
		}
		else
		{
			urlbuf[urlbufpos] = enurl.c_str()[i];
			i += 1;
		}
	}
	
	urlbuf[urlbufpos] = 0;

	int len = utf82ansi(urlbuf,urlbufpos,asiibuf,urlbuflen);
	if(len < 0)
	{
		delete []urlbuf;
		delete []asiibuf;
		return "";
	}

	std::string url(asiibuf,len);

	delete []urlbuf;
	delete []asiibuf;

	return url;
}
	
}	
}


