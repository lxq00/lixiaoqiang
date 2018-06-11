#include "Base/Base.h"
#include "HTTP/Defs.h"
#ifndef GCCSUPORTC11

#include "HTTPRequest_98.h"
#include "HTTPResponse_98.h"
#include "shttpd/shttpd.h"
#include "HTTP/HTTPServer.h"
#include "HTTP/HTTPClient.h"
#include "JSON/json.h"
#include <time.h>
#include <sstream>

namespace Public {
namespace HTTP {

struct HTTPDataInfo
{
	shared_ptr<HTTPServer::HTTPRequest>		request;
	shared_ptr<HTTPServer::HTTPResponse>	response;
	uint64_t								prevTime;
	struct shttpd_arg *						arg;
	uint32_t								totalsize;
	bool									isRequest;

	std::string  queryHeaderCallback(const std::string& key)
	{
		return shttpd_get_header(arg,key.c_str());
	}
};

struct HTTPUserCallbackInfo
{
	void*							internal;
	void*							listen;
};

struct HTTPListenInfo
{
	std::string						listenUrl;
	std::string						method;
	HTTPServer::HttpListenCallback	callback;
	shared_ptr<HTTPUserCallbackInfo> userData;
};



#define  HTTPTIMEOUT		60*1000

struct HTTPServer::HTTPServrInternal:public Thread
{
	shared_ptr<Timer>						pooltimer;
	std::string								cachediv;

	Mutex									mutex;
	
	HttpListenCallback						defaultCallback;
	shared_ptr<HTTPUserCallbackInfo>		defaultCallbackInfo;
	
	std::map<void*,shared_ptr<HTTPDataInfo> >	dataList;

	std::map<const HTTPListenInfo*,shared_ptr<HTTPListenInfo> >	callbackList;

	uint32_t								listenport;
	HTTPBufferCacheType						cachetype;


	HTTPServrInternal():Thread("HTTPServrInternal"){}

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

		{
			Guard locker(mutex);
			for (std::map<const HTTPListenInfo*, shared_ptr<HTTPListenInfo> >::iterator iter = callbackList.begin(); iter != callbackList.end(); iter++)
			{
				shttpd_register_uri(pCtx, iter->second->listenUrl.c_str(), RequestAvg, iter->second->userData.get());
			}
		}

		shttpd_register_uri(pCtx,"*", RequestAvg, defaultCallbackInfo.get());

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
		for (std::map<std::string, URI::Value>::iterator iter = response->internal->header.begin(); iter != response->internal->header.end(); iter++)
		{
			if (strcasecmp(iter->first.c_str(), "Content-Type") == 0)
			{
				isFindContentType = true;
				break;
			}
		}

		if (!isFindContentType)
		{
			response->internal->header["Content-Type"] = URI::Value("text/html; charset=gb2312");
		}

		std::ostringstream stream;
		stream << "HTTP/1.1 " << response->internal->code << " " << response->internal->error << "\r\n";

		for (std::map<std::string, URI::Value>::iterator iter = response->internal->header.begin(); iter != response->internal->header.end(); iter++)
		{
			stream << iter->first << ":" << iter->second.readString() << "\r\n";
		}
		stream << "Content-Length:" << response->internal->buffer->totalSize() << "\r\n";
		stream << "Data:" << datestr << "\r\n";
		stream << "\r\n";

		return stream.str();
	}

	void doRequestAvr(struct shttpd_arg * arg,const HTTPListenInfo* listenInfo)
	{
		shared_ptr<HTTPDataInfo> info;
		
		if(arg->state == NULL)
		{
			const char* contnetlen = shttpd_get_header(arg,"Content-Length");
			if(contnetlen == NULL)
			{
				contnetlen = "0";
			}

			std::string strUrl = "http://";
			strUrl += shttpd_get_header(arg, "Host");
			const char* url = shttpd_get_env(arg,"REQUEST_URI");
			if(url == NULL)
			{
				shttpd_printf(arg,"HTTP /1.0 511 Bad Required No Url");
				arg->flags |= SHTTPD_END_OF_OUTPUT;
				return;
			}
			strUrl += url;
			const char* request = shttpd_get_env(arg, "QUERY_STRING");
			if (request != NULL)
			{
				strUrl += "?";
				strUrl += request;
			}

			const char* method = shttpd_get_env(arg, "REQUEST_METHOD");
			if (method == NULL)
			{
				shttpd_printf(arg, "HTTP /1.0 511 Bad Required No Method");
				arg->flags |= SHTTPD_END_OF_OUTPUT;
				return;
			}

			info = make_shared<HTTPDataInfo>();
			info->arg = arg;
			info->totalsize = atoi(contnetlen);
			info->prevTime = Time::getCurrentMilliSecond();
			info->isRequest = true;

			info->request = make_shared<HTTPServer::HTTPRequest>();
			info->request->internal->init(strUrl,cachediv,QueryCallback(&HTTPDataInfo::queryHeaderCallback,info.get()));
			info->request->internal->method = method;
			
			info->response = make_shared<HTTPServer::HTTPResponse>();

			arg->state = info.get();

			{
				Guard locker(mutex);
				dataList[info.get()] = info;
			}			
		}
		else
		{
			Guard locker(mutex);
			std::map<void*,shared_ptr<HTTPDataInfo> >::iterator iter = dataList.find(arg->state);
			if(iter != dataList.end())
			{
				info = iter->second;
			}
		}

		if(info == NULL)
		{
			arg->flags |= SHTTPD_END_OF_OUTPUT;
			return;
		}

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
				info->request->buffer()->write(arg->in.buf, arg->in.len);
				arg->in.num_bytes = arg->in.len;
				if (info->request->buffer()->totalSize() < info->totalsize)
				{
					return;
				}
			}			

			HttpListenCallback callback = defaultCallback;

			{
				Guard locker(mutex);
				std::map<const HTTPListenInfo*, shared_ptr<HTTPListenInfo> >::iterator iter = callbackList.find(listenInfo);
				if (iter != callbackList.end())
				{
					callback = iter->second->callback;
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
				Guard locker(mutex);
				readlen = info->response->internal->buffer->read(buf, bufSize);
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

	static void RequestAvg(struct shttpd_arg * arg)
	{
		HTTPUserCallbackInfo* callbackinfo = (HTTPUserCallbackInfo*)arg->user_data;
		if(callbackinfo == NULL || callbackinfo->internal == NULL)
		{
			arg->flags |= SHTTPD_END_OF_OUTPUT;
			return;
		}
		HTTPServrInternal* internal = (HTTPServrInternal*)callbackinfo->internal;
		if (NULL == internal)
		{
			arg->flags |= SHTTPD_END_OF_OUTPUT;
			return;
		}

		internal->doRequestAvr(arg,(const HTTPListenInfo*)callbackinfo->listen);
	}
};

HTTPServer::HTTPServer(HTTPBufferCacheType type)
{
	internal = new HTTPServrInternal();
	internal->cachediv = File::getExcutableFileFullPath() + "/._httpcache";
	internal->defaultCallbackInfo = make_shared<HTTPUserCallbackInfo>();
	internal->defaultCallbackInfo->internal = internal;
	internal->defaultCallbackInfo->listen = NULL;
	internal->cachetype = type;

	{
		File::removeDirectory(internal->cachediv.c_str());
		File::makeDirectory(internal->cachediv.c_str());
	}

	internal->pooltimer = make_shared<Timer>("HTTPServer");
	internal->pooltimer->start(Timer::Proc(&HTTPServrInternal::poolTimerProc,internal),0,5000);
}
HTTPServer::~HTTPServer()
{
	internal->destroyThread();
	internal->pooltimer = NULL;	
	SAFE_DELETE(internal);
}

// path 为 请求的url,*为所有  ,callback监听消息的回掉,处理线程数据
bool HTTPServer::listen(const std::string& path, const std::string& method, const HttpListenCallback& callback)
{
	Guard locker(internal->mutex);
	if(path == "*")
	{
		internal->defaultCallback = callback;
	}
	else
	{
		bool isFindListen = false;
		for (std::map<const HTTPListenInfo*, shared_ptr<HTTPListenInfo> >::iterator iter = internal->callbackList.begin(); iter != internal->callbackList.end(); iter++)
		{
			if (strcasecmp(iter->second->listenUrl.c_str(), path.c_str()) == 0 && strcasecmp(iter->second->method.c_str(),method.c_str()) == 0)
			{
				isFindListen = true;
				break;
			}
		}

		if (!isFindListen)
		{
			shared_ptr<HTTPListenInfo> listeninfo(new HTTPListenInfo);
			listeninfo->listenUrl = path;
			listeninfo->method = method;
			listeninfo->callback = callback;
			listeninfo->userData = make_shared<HTTPUserCallbackInfo>();
			listeninfo->userData->internal = internal;
			listeninfo->userData->listen = listeninfo.get();

			internal->callbackList[listeninfo.get()] = listeninfo;
		}
	}

	return true;
}

//异步监听
bool HTTPServer::run(uint32_t httpport, uint32_t threadNum)
{
	internal->listenport = httpport;

	internal->createThread();

	return true;
}

}
}
#endif
