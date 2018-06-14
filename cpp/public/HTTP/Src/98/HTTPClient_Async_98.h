#include "Base/Defs.h"
#include "HTTP/Defs.h"

#ifndef GCCSUPORTC11

#include "curl/curl.h"
#include "HTTP/HTTPClient.h"

 namespace Public{
 namespace HTTP{


struct HTTPClientAsyncInternal
{
	CURL* curl;
	shared_ptr<HTTPBuffer> headerbuf;
	shared_ptr<HTTPResponse> response;
	shared_ptr<HTTPRequest>  request;
	struct curl_slist* header;
public:
	HTTPClientAsyncInternal() :request(NULL),header(NULL)
	{
		response = make_shared<HTTPResponse>();
		response->setStatusCode(500, "Not Init");

		curl = curl_easy_init();
	}
	virtual ~HTTPClientAsyncInternal()
	{
		if (curl != NULL)
		{
			curl_easy_cleanup(curl);
			curl = NULL;
		}
		curl_slist_free_all(header);
	}

	static size_t OnWriteHeader(void* buffer, size_t size, size_t nmemb, void* lpVoid)
	{
		HTTPClientAsyncInternal* internal = (HTTPClientAsyncInternal *)lpVoid;
		if (NULL == internal || NULL == buffer)
		{
			return -1;
		}

		if (internal->headerbuf == NULL)
		{
			internal->headerbuf = make_shared<HTTPBuffer>();
		}
		if (internal->headerbuf != NULL)
		{
			internal->headerbuf->write(std::string((char*)buffer, size * nmemb));
		}

		return size * nmemb;
	}
	static size_t OnWriteData(void* buffer, size_t size, size_t nmemb, void* lpVoid)
	{
		HTTPClientAsyncInternal* internal = (HTTPClientAsyncInternal *)lpVoid;
		if (NULL == internal || NULL == buffer)
		{
			return -1;
		}

		internal->response->buf.write(std::string((char*)buffer, size * nmemb));

		return size * nmemb;
	}
	static size_t OnReadData(void* buffer, size_t size, size_t nmemb, void* lpVoid)
	{
		HTTPClientAsyncInternal* internal = (HTTPClientAsyncInternal *)lpVoid;
		if (NULL == internal || NULL == buffer)
		{
			return -1;
		}
		if (internal->request == NULL)
		{
			return 0;
		}
		int readlen = internal->request->buf.read((char*)buffer, size* nmemb);

		return readlen;
	}


	bool setHeader(const shared_ptr<HTTPRequest>& req)
	{
		if (req == NULL || curl == NULL)
		{
			return false;
		}

		for (std::map<std::string, URI::Value>::const_iterator iter = req->headers.begin(); iter != req->headers.end(); iter++)
		{
			header = curl_slist_append(header, (iter->first + (": ") + iter->second.readString()).c_str());
		}

		if (header != NULL)
		{
			curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);
		}

		return true;
	}
	void buildRequest(const shared_ptr<HTTPRequest>& req)
	{
		if (curl == NULL)
		{
			return;
		}

		request = req;

		setHeader(request);

		{
			curl_easy_setopt(curl, CURLOPT_URL, request->url.href().c_str());
			curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
			curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, req->timeout / 1000);
			curl_easy_setopt(curl, CURLOPT_TIMEOUT, req->timeout / 1000);
		}

		if (strcasecmp(req->method.c_str(), "post") == 0)
		{
			curl_easy_setopt(curl, CURLOPT_POST, 1);
			curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, request->buf.size());
		}
		else if (strcasecmp(req->method.c_str(), "put") == 0)
		{
			curl_easy_setopt(curl, CURLOPT_PUT, 1);
			curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, request->buf.size());
		}
		else if (strcasecmp(req->method.c_str(), "delete") == 0)
		{
			response->setStatusCode(500, "not suport the method");

			return;
		}

		curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, HTTPClientAsyncInternal::OnWriteHeader);
		curl_easy_setopt(curl, CURLOPT_WRITEHEADER, this);
		curl_easy_setopt(curl, CURLOPT_READFUNCTION, HTTPClientAsyncInternal::OnReadData);
		curl_easy_setopt(curl, CURLOPT_READDATA, this);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, HTTPClientAsyncInternal::OnWriteData);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);
	}
};

class HTTPClientAsync: public Thread
{
	struct AsyncInfo :public HTTPClientAsyncInternal
	{
		AsyncInfo(const HTTPClient::AsynCallback& _callback) :HTTPClientAsyncInternal(), callback(_callback){}
		~AsyncInfo() {}

		HTTPClient::AsynCallback	callback;
	};

	typedef std::map<CURL*, shared_ptr<AsyncInfo> > HttpMap;

	int curl_multi_select()
	{
		struct timeval timeout_tv;
		fd_set  fd_read;
		fd_set  fd_write;
		fd_set  fd_except;
		int     max_fd = -1;
		FD_ZERO(&fd_read);
		FD_ZERO(&fd_write);
		FD_ZERO(&fd_except);
		/*CURLMcode hr = */
		curl_multi_fdset(curl_m, &fd_read, &fd_write, &fd_except, &max_fd);
		if(max_fd == -1)
		{
			Thread::sleep(10);
			return -1;
		}
		timeout_tv.tv_sec = 0;
		timeout_tv.tv_usec = 100;

		int ret_code = ::select(max_fd + 1, &fd_read, &fd_write, &fd_except, &timeout_tv);
		
		return ret_code > 0;
	}

	virtual void threadProc()
	{
		while (looping())
		{
			//first do add event
			{
				std::list<shared_ptr<AsyncInfo> > addErrorList;

				{
					Guard locker(mutex);
					while (waitList.size() > 0)
					{
						shared_ptr<AsyncInfo> internal = waitList.front();
						waitList.pop_front();
						CURLMcode hr = curl_multi_add_handle(curl_m, internal->curl);
						if (hr != CURLM_OK)
						{
							logerror("curl_multi_add_handle() fail! err:%d file:%s line:%d", hr, __FILE__, __LINE__);
							addErrorList.push_back(internal);
							continue;
						}
						mapHttp[internal->curl] = internal;
					}
				}

				for (std::list<shared_ptr<AsyncInfo> >::iterator iter = addErrorList.begin(); iter != addErrorList.end(); iter++)
				{
					shared_ptr<AsyncInfo> internal = *iter;
					internal->response->setStatusCode(100, "curl_multi_add_handle error");

					internal->callback(internal->request, internal->response);
				}
			}

			std::list<shared_ptr<AsyncInfo> > doSuccessList;
			//do perform
			{
				int running_handles = 0;
				curl_multi_perform(curl_m, &running_handles);
				curl_multi_select();

				int msgs_left;
				CURLMsg* msg = NULL;
				while ((msg = curl_multi_info_read(curl_m, &msgs_left)) != NULL)
				{
					if (CURLMSG_DONE == msg->msg)
					{
						shared_ptr<AsyncInfo> internal;
						{
							Guard locker(mutex);
							HttpMap::iterator item = mapHttp.find(msg->easy_handle);
							if (item != mapHttp.end())
							{
								internal = item->second;
								mapHttp.erase(item);
							}
						}
						if (internal != NULL)
						{
							curl_easy_getinfo(internal->curl, CURLINFO_HTTP_CODE, &internal->response->statusCode);
							if (msg->data.result == CURLE_OK)
							{
								internal->response->errorMessage = "";
							}
							else
							{
								internal->response->statusCode = msg->data.result;
								internal->response->errorMessage = curl_easy_strerror(msg->data.result);
							}

							doSuccessList.push_back(internal);
						}
						curl_multi_remove_handle(curl_m, msg->easy_handle);
					}
				}
			}

			//dosuccess callback
			{
				for (std::list<shared_ptr<AsyncInfo> >::iterator iter = doSuccessList.begin(); iter != doSuccessList.end(); iter++)
				{
					shared_ptr<AsyncInfo> internal = *iter;
					internal->callback(internal->request, internal->response);
				}
			}
		}
	}
public:
	HTTPClientAsync() : Thread("MultiHttpClient"), curl_m(NULL)
	{
		curl_m = curl_multi_init();
		createThread();
	}

	~HTTPClientAsync()
	{
		destroyThread();
		curl_multi_cleanup(curl_m);
	}
	bool HTTPClientAsync::requestAsyn(const shared_ptr<HTTPRequest>& req,const HTTPClient::AsynCallback& _callback)
	{
		if (curl_m == NULL)
		{
			return false;
		}

		shared_ptr<AsyncInfo> asyncinfo(new AsyncInfo(_callback));

		if (asyncinfo->curl == NULL)
		{
			return false;
		}

		asyncinfo->buildRequest(req);

		{
			Guard locker(mutex);
			waitList.push_back(asyncinfo);
		}

		return true;
	}
public:
	CURLM*		curl_m;	

	Mutex												mutex;
	HttpMap												mapHttp;
	std::list<shared_ptr<AsyncInfo> >	waitList;
};



}
}

#endif
