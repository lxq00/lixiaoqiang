#pragma once
#include "Defs.h"
#include "Base/Base.h"
#include "Network/Network.h"
using namespace Public::Base;
using namespace Public::Network;

namespace Public {
namespace HTTP {

#define Content_Length			"Content-Length"
#define Content_Type			"Content-Type"

#define Transfer_Encoding		"Transfer-Encoding"

#define CHUNKED					"chunked"
#define CONNECTION				"Connection"
#define CONNECTION_Close		"Close"
#define CONNECTION_KeepAlive	"keep-alive"
#define CONNECTION_Upgrade		"Upgrade"

#define HTTPSEPERATOR			"\r\n"


struct HTTPHeader
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

	HTTPHeader()
	{
		statuscode = 200;
		statusmsg = "OK";
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
};

class HTTPParse
{
public:
	HTTPParse(bool _isRequest):isRequest(_isRequest){}
	~HTTPParse() {}

	shared_ptr<HTTPHeader> parse(CircleBuffer& buffer,std::string* usedstr = NULL)
	{
		if (content == NULL) content = make_shared<HTTPHeader>();

		bool contentIsOk = false;
		while (!contentIsOk)
		{
			std::string linestr;
			if (!buffer.consumeLine(linestr)) break;

			if (usedstr != NULL)
			{
				*usedstr += linestr + "\r\n";
			}

			contentIsOk = parseLine(linestr.c_str(), linestr.length());
		}

		shared_ptr<HTTPHeader> contenttmp = content;
		if (contenttmp != NULL && contentIsOk)
		{
			content = NULL;
			return contenttmp;
		}

		return shared_ptr<HTTPHeader>();
	}
	shared_ptr<HTTPHeader> parse(const char* data,uint32_t datalen,uint32_t& useddata)
	{
		if (content == NULL) content = make_shared<HTTPHeader>();

		useddata = 0;
		bool contentIsOk = false;
		while (!contentIsOk)
		{
			size_t pos = String::indexOf(data, datalen, HTTPSEPERATOR);
			if (pos == -1) break;


			{
				uint32_t poslen = pos + strlen(HTTPSEPERATOR);

				data += poslen;
				datalen -= poslen;
				useddata += poslen;
			}

			contentIsOk = parseLine(data, pos);
		}
		shared_ptr<HTTPHeader> contenttmp = content;
		if (contenttmp != NULL && contentIsOk)
		{
			content = NULL;
			return contenttmp;
		}

		return shared_ptr<HTTPHeader>();
	}

	bool isFindFirstLine()
	{
		if (content == NULL || content->verinfo.protocol == "") return false;

		return true;
	}
private:
	bool parseLine(const char* startlineaddr, uint32_t linedatalen)
	{
		//连续两个SEPERATOR表示header结束
		if (linedatalen == 0)
		{
			return true;
		}
		else if (content->verinfo.protocol == "")
		{
			parseFirstLine(startlineaddr, linedatalen);
		}
		else
		{
			parseHeaderLine(startlineaddr, linedatalen);
		}

		return false;
	}
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
	bool					isRequest;
	shared_ptr<HTTPHeader>	content;
};

class HTTPBuild
{
public:
	static std::string build(bool isRequest,const HTTPHeader& headertmp)
	{
		HTTPHeader header = headertmp;
		
		{
			header.verinfo.protocol = header.url.protocol;
			if (header.verinfo.protocol.length() == 0) header.verinfo.protocol = "HTTP";

			if (strcasecmp(header.verinfo.protocol.c_str(), "http") == 0) header.verinfo.version = "1.1";
			else if (strcasecmp(header.verinfo.protocol.c_str(), "rtsp") == 0) header.verinfo.version = "1.0";
		}
		std::string cmdstr;

		if (isRequest)
		{
			std::string requrl = header.url.href();
			if (strcasecmp(header.verinfo.protocol.c_str(), "http") == 0)
			{
				requrl = header.url.getPath();
			}

			cmdstr = header.method + " " + requrl + " "+header.verinfo.protocol+"/"+header.verinfo.version + HTTPSEPERATOR;
		}
		else
		{
			cmdstr = header.verinfo.protocol + "/" + header.verinfo.version + Value(header.statuscode).readString() + " " + (header.statuscode == 200 ? "OK" : header.statusmsg) + HTTPSEPERATOR;
		}
		for (std::map<std::string, Value>::const_iterator iter = header.headers.begin(); iter != header.headers.end(); iter++)
		{
			cmdstr += iter->first + ": " + iter->second.readString() + HTTPSEPERATOR;
		}
		cmdstr += HTTPSEPERATOR;

		return cmdstr;
	}
};


}
}