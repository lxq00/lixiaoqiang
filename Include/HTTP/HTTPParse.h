#pragma once
#include "Defs.h"
#include "Base/Base.h"
#include "Network/Network.h"
using namespace Public::Base;
using namespace Public::Network;

namespace Public {
namespace HTTP {

#define Content_Length	"Content-Length"
#define Content_Type	"Content-Type"

#define Transfer_Encoding "Transfer-Encoding"

#define CHUNKED					"chunked"
#define CONNECTION				"Connection"
#define CONNECTION_Close		"Close"
#define CONNECTION_KeepAlive	"keep-alive"
#define CONNECTION_Upgrade		"Upgrade"

#define HTTPSEPERATOR				"\r\n"

class HTTPParse
{
public:
	struct Header
	{
		std::string		method;
		URL				url;
		struct {
			std::string protocol;
			std::string	version;
		}verinfo;

		int				statuscode;
		std::string		statusmsg;

		std::map<std::string, Value> headers;

		Header()
		{
			statuscode = 200;
			headerIsOk = false;
		}
		Value header(const std::string& key)
		{
			for (std::map<std::string, Value>::iterator iter = headers.begin(); iter != headers.end(); iter++)
			{
				if (strcasecmp(key.c_str(), iter->first.c_str()) == 0)
				{
					return iter->second;
				}
			}
			return Value();
		}
	private:
		friend class HTTPParse;

		bool	headerIsOk;
	};

	HTTPParse(bool _isRequest):isRequest(_isRequest){}
	~HTTPParse() {}

	shared_ptr<Header> parse(const char* data,uint32_t datalen,uint32_t& useddata)
	{
		useddata = 0;

		while (datalen > 0)
		{
			if (content == NULL) content = make_shared<Header>();

			size_t pos = String::indexOf(data, datalen, HTTPSEPERATOR);
			if (pos == -1) break;

			const char* startlineaddr = data;
			uint32_t linedatalen = pos;

			data += pos + strlen(HTTPSEPERATOR);
			datalen -= pos + strlen(HTTPSEPERATOR);
			useddata += pos + strlen(HTTPSEPERATOR);

			//连续两个SEPERATOR表示header结束
			if (pos == 0)
			{
				content->headerIsOk = true;
				break;
			}
			else if (!content->headerIsOk && content->verinfo.protocol == "")
			{
				parseFirstLine(startlineaddr, linedatalen);
			}
			else if (!content->headerIsOk)
			{
				parseHeaderLine(startlineaddr, linedatalen);
			}
		}
		shared_ptr<Header> contenttmp = content;
		if (contenttmp != NULL && contenttmp->headerIsOk)
		{
			content = NULL;
			return contenttmp;
		}

		return shared_ptr<Header>();
	}

	bool isFindFirstLine()
	{
		if (content == NULL || content->verinfo.protocol == "") return false;

		return true;
	}
private:
	void parseFirstLine(const char* startlienaddr, uint32_t linedatalen)
	{
		std::string versionstr;
		if (isRequest)
		{
			std::vector<std::string> tmp = String::split(startlienaddr, linedatalen, " ");
			if (tmp.size() != 3) return;

			content->method = tmp[0];
			content->url = URL(tmp[1]);
			versionstr = tmp[2];
		}
		else
		{
			std::vector<std::string> tmp = String::split(startlienaddr, linedatalen, " ");
			if (tmp.size() < 2) return;

			versionstr = tmp[0];
			content->statuscode = Value(tmp[1]).readInt();

			std::string errstr;
			for (uint32_t i = 2; i < tmp.size(); i++)
			{
				errstr += (i == 2 ? "" : " ") + tmp[i];
			}
			content->statusmsg = errstr;
		}

		if (versionstr != "")
		{
			std::vector<std::string> tmp = String::split(versionstr, "/");
			if (tmp.size() >= 1) content->verinfo.protocol = tmp[0];
			if (tmp.size() >= 2) content->verinfo.version = tmp[1];
		}
	}
	void parseHeaderLine(const char* startlienaddr, uint32_t linedatalen)
	{
		std::vector<std::string> tmp = String::split(startlienaddr, linedatalen, ":");
		if (tmp.size() >= 1)
		{
			std::string strtmp;
			for (uint32_t i = 1; i < tmp.size(); i++)
			{
				strtmp += (i == 1 ? "" : "/") + tmp[i];
			}

			content->headers[String::strip(tmp[0])] = String::strip(strtmp);
		}
	}
private:
	bool isRequest;
	shared_ptr<Header> content;
};

}
}