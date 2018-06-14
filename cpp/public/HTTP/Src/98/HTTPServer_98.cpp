#include "Base/Defs.h"
#include "HTTP/Defs.h"

#ifndef GCCSUPORTC11
#include "shttpd/shttpd.h"
#include "HTTP/HTTPServer.h"
#include "HTTP/HTTPClient.h"
#include "JSON/json.h"
#include <time.h>
#include <sstream>

#include "boost/regex.hpp"

namespace Public {
namespace HTTP {


struct HTTPDataInfo
{
	shared_ptr<HTTPRequest>		request;
	shared_ptr<HTTPResponse>	response;
	uint64_t								prevTime;
	struct shttpd_arg *						arg;
	uint32_t								totalsize;
	bool									isRequest;

	std::string  queryHeaderCallback(const std::string& key)
	{
		return shttpd_get_header(arg,key.c_str());
	}

	static void parseHeader(std::map<std::string, URI::Value>& headers, struct shttpd_arg *arg)
	{
		const char* startptr = NULL;
		const char* endptr = NULL;
		shttpd_get_headeraddr(arg, &startptr, &endptr);

		std::string headerstr(startptr, endptr);
		const char* headerstrtmp = headerstr.c_str();

		while (1)
		{
			bool endoffile = false;
			
			std::string headerline;
			{
				const char* endtmp = strstr(headerstrtmp, "\r\n");
				if (endtmp == NULL)
				{
					endoffile = true;
					headerline = headerstrtmp;
				}
				else
				{
					headerline = std::string(headerstrtmp, endtmp);
					headerstrtmp = endtmp += 2;
				}
			}
			
			const char* valpos = strchr(headerline.c_str(), ':');
			if (valpos == NULL) break;

			std::string key = stip(std::string(headerline.c_str(), valpos - headerline.c_str()));
			std::string val = stip(valpos + 1);

			headers[key] = val;
			if (endoffile) break;
		}
	}
	static std::string stip(const std::string& tmp)
	{
		const char* starttmp = tmp.c_str();
		while (*starttmp == ' ') starttmp++;

		const char* endtmp = starttmp + strlen(starttmp);
		while (endtmp > starttmp && (*endtmp == ' ' || *endtmp == '\r' || *endtmp == '\n')) endtmp--;

		return std::string(starttmp, endtmp - starttmp);
	}
};



#define  HTTPTIMEOUT		60*1000

struct HTTPServer::HTTPServrInternal:public Thread
{
	shared_ptr<Timer>						pooltimer;

	Mutex									mutex;
	std::map<std::string,std::map<std::string, HTTPServer::HttpListenCallback> >	callbackList;
	std::map<std::string, HTTPServer::HttpListenCallback>							defaultCallbackList;

	std::map<void*,shared_ptr<HTTPDataInfo> >	dataList;

	uint32_t								listenport;


	HTTPServrInternal():Thread("HTTPServrInternal")
	{
		pooltimer = make_shared<Timer>("HTTPServer");
		pooltimer->start(Timer::Proc(&HTTPServrInternal::poolTimerProc, this), 0, 5000);
	}

	~HTTPServrInternal()
	{
		destroyThread();
		pooltimer = NULL;
	}

	void threadProc()
	{
		char portbuffer[32];
		sprintf(portbuffer,"%d",listenport);

		const char *argv_Complete[6] = {0};
		argv_Complete[0] = "StartService";
		argv_Complete[1] = "-ports";
		argv_Complete[2] = portbuffer;
		argv_Complete[3] = "-root";
		argv_Complete[4] = "default";
		argv_Complete[5] = NULL;
		shttpd_ctx * pCtx = shttpd_init(5, (char **)argv_Complete);
		if (NULL == pCtx)
		{
			return;
		}

		shttpd_register_uri(pCtx,"*", RequestAvg, this);

		while(looping())
		{
			shttpd_poll(pCtx, 1000);
		}

		shttpd_fini(pCtx);
	}

	void poolTimerProc(unsigned long param)
	{
		std::list<shared_ptr<HTTPDataInfo> >		freeList;
		{
			Guard locker(mutex);
			for(std::map<void*,shared_ptr<HTTPDataInfo> >::iterator iter = dataList.begin();iter != dataList.end();)
			{
				if(Time::getCurrentMilliSecond() - iter->second->prevTime >= HTTPTIMEOUT)
				{
					freeList.push_back(iter->second);
					dataList.erase(iter++);
				}
				else
				{
					iter++;
				}
			}
		}		
	}

	std::string GetResponseHeader(const shared_ptr<HTTPResponse>& response)
	{
		std::string datestr;
		{
			static Mutex	responsemutex;

			Guard locker(responsemutex);

			time_t ltime;
			time(&ltime);
			char *Data = ctime(&ltime);
			datestr = std::string(Data,strlen(Data) - 1);
		}

		bool isFindContentType = false;
		for (std::map<std::string, URI::Value>::iterator iter = response->headers.begin(); iter != response->headers.end(); iter++)
		{
			if (strcasecmp(iter->first.c_str(), "Content-Type") == 0)
			{
				isFindContentType = true;
				break;
			}
		}

		if (!isFindContentType)
		{
			response->headers["Content-Type"] = URI::Value("text/html; charset=gb2312");
		}

		std::ostringstream stream;
		stream << "HTTP/1.1 " << response->statusCode << " " << response->errorMessage << "\r\n";

		for (std::map<std::string, URI::Value>::iterator iter = response->headers.begin(); iter != response->headers.end(); iter++)
		{
			stream << iter->first << ":" << iter->second.readString() << "\r\n";
		}
		stream << "Content-Length:" << response->buf.size() << "\r\n";
		stream << "Data:" << datestr << "\r\n";
		stream << "\r\n";

		return stream.str();
	}

	void doRequestAvrData(struct shttpd_arg * arg, shared_ptr<HTTPDataInfo>& info, HTTPServer::HttpListenCallback& callback)
	{
		info->prevTime = Time::getCurrentMilliSecond();

		if(arg->flags & SHTTPD_CONNECTION_ERROR)
		{
			Guard locker(mutex);
			dataList.erase(info.get());

			arg->flags |= SHTTPD_END_OF_OUTPUT;
			return;
		}

		if(info->isRequest)
		{
			{
				Guard locker(mutex);
				info->request->buf.write(arg->in.buf, arg->in.len);
				arg->in.num_bytes = arg->in.len;
				if (info->request->buf.size() < info->totalsize)
				{
					return;
				}
			}			

			info->isRequest = false;
			callback(info->request,info->response);

			std::string header = GetResponseHeader(info->response);

			shttpd_printf(arg,header.c_str());
			arg->flags |= SHTTPD_MORE_POST_DATA;
			arg->flags |= SHTTPD_POST_BUFFER_FULL;
			return;
		}
		else
		{
			char *buf = arg->out.buf + arg->out.num_bytes;
			int bufSize = arg->out.len - arg->out.num_bytes;

			if(bufSize == 0)
			{
				arg->flags |= SHTTPD_MORE_POST_DATA;
				arg->flags |= SHTTPD_POST_BUFFER_FULL;
				return;
			}
			int readlen = 0;
			{
				readlen = info->response->buf.read(buf, bufSize);
			}
			if(readlen <= 0)
			{
				arg->flags |= SHTTPD_END_OF_OUTPUT;
				
				Guard locker(mutex);
				dataList.erase(info.get());

				return;
			}

			arg->out.num_bytes += readlen;
		}
		
	}
	void doRequestAvg(struct shttpd_arg * arg)
	{
		shared_ptr<HTTPDataInfo> info;
		HTTPServer::HttpListenCallback callback;
		if (arg->state == NULL)
		{
			const char* url = shttpd_get_env(arg, "REQUEST_URI");
			if (url == NULL)
			{
				shttpd_printf(arg, "HTTP /1.0 511 Bad Required No Url");
				arg->flags |= SHTTPD_END_OF_OUTPUT;
				return;
			}

			const char* method = shttpd_get_env(arg, "REQUEST_METHOD");
			if (method == NULL)
			{
				shttpd_printf(arg, "HTTP /1.0 511 Bad Required No Method");
				arg->flags |= SHTTPD_END_OF_OUTPUT;
				return;
			}

			{
				std::map<std::string, std::map<std::string, HTTPServer::HttpListenCallback> > list;
				{
					Guard locker(mutex);
					list = callbackList;
				}
				for (std::map<std::string, std::map<std::string, HTTPServer::HttpListenCallback> >::iterator iter = list.begin(); iter != list.end()&& callback != NULL; iter++)
				{
					boost::regex  regex(iter->first);

					if (!boost::regex_match(String::tolower(url), regex))
					{
						continue;
					}
					for (std::map<std::string, HTTPServer::HttpListenCallback>::iterator miter = iter->second.begin(); miter != iter->second.end(); miter++)
					{
						if (String::tolower(method) == miter->first)
						{
							callback = miter->second;
							break;
						}
					}
				}
				if (callback == NULL)
				{
					for (std::map<std::string, HTTPServer::HttpListenCallback>::iterator miter = defaultCallbackList.begin(); miter != defaultCallbackList.end(); miter++)
					{
						if (String::tolower(method) == miter->first)
						{
							callback = miter->second;
							break;
						}
					}
				}
			}

			if (callback == NULL)
			{
				shttpd_printf(arg, "HTTP /1.0 511 invild Required url");
				arg->flags |= SHTTPD_END_OF_OUTPUT;
				return;
			}

			const char* contnetlen = shttpd_get_header(arg, "Content-Length");
			if (contnetlen == NULL)
			{
				contnetlen = "0";
			}

			std::string strUrl = "http://";
			strUrl += shttpd_get_header(arg, "Host");
			strUrl += url;
			const char* request = shttpd_get_env(arg, "QUERY_STRING");
			if (request != NULL)
			{
				strUrl += "?";
				strUrl += request;
			}
			

			info = make_shared<HTTPDataInfo>();
			info->arg = arg;
			info->totalsize = atoi(contnetlen);
			info->prevTime = Time::getCurrentMilliSecond();
			info->isRequest = true;

			info->request = make_shared<HTTPRequest>();
			info->request->url.href(strUrl);
			HTTPDataInfo::parseHeader(info->request->headers, arg);
			info->request->method = method;

			info->response = make_shared<HTTPResponse>();
			arg->state = info.get();

			{
				Guard locker(mutex);
				dataList[info.get()] = info;
			}
		}
		else
		{
			Guard locker(mutex);
			std::map<void*, shared_ptr<HTTPDataInfo> >::iterator iter = dataList.find(arg->state);
			if (iter != dataList.end())
			{
				info = iter->second;
			}
		}

		if (info == NULL)
		{
			arg->flags |= SHTTPD_END_OF_OUTPUT;
			return;
		}

		doRequestAvrData(arg, info, callback);
	}

	static void RequestAvg(struct shttpd_arg * arg)
	{
		HTTPServrInternal* internal = (HTTPServrInternal*)arg->user_data;
		if(internal == NULL)
		{
			arg->flags |= SHTTPD_END_OF_OUTPUT;
			return;
		}
		internal->doRequestAvg(arg);
	}
};

HTTPServer::HTTPServer()
{
	internal = new HTTPServrInternal();
}
HTTPServer::~HTTPServer()
{
	SAFE_DELETE(internal);
}

bool HTTPServer::listen(const std::string& path, const std::string& method, const HttpListenCallback& callback)
{
	Guard locker(internal->mutex);
	if(path == "*" || path == "")
	{
		internal->defaultCallbackList[String::tolower(method)] = callback;
	}
	else
	{
		internal->callbackList[String::tolower(path)][String::tolower(method)] = callback;
	}

	return true;
}

//Òì²½¼àÌý
bool HTTPServer::run(uint32_t httpport, uint32_t threadNum)
{
	for (std::map<std::string, std::map<std::string, HTTPServer::HttpListenCallback> >::iterator iter = resource.begin(); iter != resource.end(); iter++)
	{
		for (std::map<std::string, HTTPServer::HttpListenCallback>::iterator miter = iter->second.begin(); miter != iter->second.end(); miter++)
		{
			listen(iter->first, miter->first, miter->second);
		}
	}

	internal->listenport = httpport;

	internal->createThread();

	return true;
}

}
}


#endif
