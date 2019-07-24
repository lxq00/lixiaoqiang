#include "Base/URLEncoding.h"
#include "Base/String.h"
using namespace std;

namespace Public{
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

	std::string utf8buf = String::ansi2utf8(url);

	char* enbuf = new char[utf8buf.length() *3 + 100];
	int enbufpos = 0;
	for(uint32_t i = 0;i < utf8buf.length();i ++)
	{
		if(NeedTrans(utf8buf.c_str()[i]))
		{
			sprintf(&enbuf[enbufpos],"%%%02X",utf8buf.c_str()[i]&0xff);
			enbufpos += 3;
		}
		else
		{
			enbuf[enbufpos] = utf8buf.c_str()[i];
			enbufpos ++;
		}
	}
	enbuf[enbufpos] = 0;

	std::string enstr(enbuf);

	delete []enbuf;

	return enstr;
}
std::string URLEncoding::decode(const std::string& enurl)
{
	if(enurl.length() <= 0)
	{
		return "";
	}

	size_t urlbuflen = enurl.length() + 100;
	char* urlbuf = new char[urlbuflen + 10];

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

	std::string url = String::utf82ansi(urlbuf);

	delete []urlbuf;

	return url;
}
	
}	
}


