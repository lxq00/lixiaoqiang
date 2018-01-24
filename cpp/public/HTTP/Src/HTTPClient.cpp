#include "curl/curl.h"
#include "HTTP/HTTPClient.h"

 namespace Xunmei{
 namespace HTTP{

struct HTTPClient::HTTPForm::HTTPFormInternal
{
	struct curl_httppost* formpost;
	struct curl_httppost* lastptr;

	HTTPFormInternal():formpost(NULL),lastptr(NULL){}
	~HTTPFormInternal()
	{
		if (formpost != NULL)
		{
			curl_formfree(formpost);
			formpost = NULL;
		}
	}
};

HTTPClient::HTTPForm::HTTPForm()
{
	internal = new HTTPFormInternal();
}
HTTPClient::HTTPForm::~HTTPForm()
{
	SAFE_DELETE(internal);
}
bool HTTPClient::HTTPForm::addContents(const std::string& key, const std::string& data)
{
	CURLFORMcode code  = curl_formadd(&internal->formpost, &internal->lastptr, CURLFORM_COPYNAME, key.c_str(), CURLFORM_COPYCONTENTS, data.c_str(), CURLFORM_END);

	return code == CURL_FORMADD_OK;
}
bool HTTPClient::HTTPForm::addFile(const std::string& filename)
{
	CURLFORMcode code = curl_formadd(&internal->formpost, &internal->lastptr, CURLFORM_COPYNAME, "file", CURLFORM_FILE, filename.c_str(), CURLFORM_END);
	return code == CURL_FORMADD_OK;
}
bool HTTPClient::HTTPForm::addFile(const std::string& filename, const char* filebuffer, uint32_t bufferlen)
{
	CURLFORMcode code = curl_formadd(&internal->formpost, &internal->lastptr, CURLFORM_COPYNAME, "file", CURLFORM_BUFFER, filename.c_str(), CURLFORM_BUFFERPTR, filebuffer , CURLFORM_BUFFERLENGTH, long(bufferlen), CURLFORM_END);
	return code == CURL_FORMADD_OK;
}

struct HTTPClient::HTTPRequest::HTTPRequestInternal
{
	std::map<HTTPHeaderType, URI::Value>	header;
	std::map<std::string, URI::Value>		headertmp;
	shared_ptr<HTTPBuffer>					buffer;
	shared_ptr<HTTPForm>					form;
};

HTTPClient::HTTPRequest::HTTPRequest(HTTPBufferCacheType type)
{
	internal = new HTTPRequestInternal;
	internal->buffer = new HTTPBuffer(type);
	internal->form = new HTTPForm();
}
HTTPClient::HTTPRequest::~HTTPRequest()
{
	SAFE_DELETE(internal);
}

bool HTTPClient::HTTPRequest::setHeader(HTTPHeaderType key, const URI::Value& val)
{
	internal->header[key] = val;

	return true;
}
bool HTTPClient::HTTPRequest::setHeader(const std::string& key, const URI::Value& val)
{
	internal->headertmp[key] = val;

	return true;
}
shared_ptr<HTTPBuffer> HTTPClient::HTTPRequest::buffer()
{
	return internal->buffer;
}
shared_ptr<HTTPClient::HTTPForm>  HTTPClient::HTTPRequest::form()
{
	return internal->form;
}

struct HTTPClient::HTTPResponse::HTTPResponseInternal
{
	uint32_t				code;
	std::string				errormsg;
	shared_ptr<HTTPBuffer>	buffer;
	std::map<std::string, URI::Value>	header;

	static std::string strip(const std::string& tmp)
	{
		const char* tmpptr = tmp.c_str();
		while (*tmpptr == ' ') tmpptr++;
		int len = strlen(tmpptr);
		while (len > 0 && tmpptr[len - 1] == ' ') len--;

		return std::string(tmpptr, len);
	}
	void parseHttpHeader(const shared_ptr<HTTPBuffer>& buffer)
	{
		if (buffer == NULL) return;

		std::string datatmp = buffer->read();

		const char* tmp = datatmp.c_str();
		while (1)
		{
			const char* nexttmp = NULL;
			const char* tmp1 = strstr(tmp, "\r\n");
			if (tmp1 != NULL)
			{
				nexttmp = tmp1 + 2;
			}
			else if (tmp1 == NULL && (tmp1 = strchr(tmp, '\n')) != NULL)
			{
				nexttmp = tmp1 + 1;
			}
			std::string headerdata = (tmp1 == NULL) ? tmp : std::string(tmp, tmp1 - tmp);
			const char* headerptr = headerdata.c_str();
			const char* posptr = strchr(headerptr, ':');
			if (posptr != NULL)
			{
				std::string key = strip(std::string(headerptr, posptr - headerptr));
				std::string val = strip(posptr + 1);

				header[key] = val;
			}
			if (nexttmp == NULL) break;
			tmp = nexttmp;
		}
	}
};

HTTPClient::HTTPResponse::HTTPResponse(HTTPBufferCacheType type)
{
	internal = new HTTPResponseInternal();
	internal->buffer = new HTTPBuffer(type);
}
HTTPClient::HTTPResponse::~HTTPResponse()
{
	SAFE_DELETE(internal);
}
uint32_t HTTPClient::HTTPResponse::statucCode() const
{
	return internal->code;
}
std::string HTTPClient::HTTPResponse::errorMessage() const
{
	return internal->errormsg;
}
const shared_ptr<HTTPBuffer> HTTPClient::HTTPResponse::buffer() const
{
	return internal->buffer;
}
URI::Value HTTPClient::HTTPResponse::getHeader(const std::string& key) const
{
	std::map<std::string, URI::Value>::iterator iter = internal->header.find(key);
	if (iter == internal->header.end())
	{
		return URI::Value();
	}
	return iter->second;
}
const std::map<std::string, URI::Value> HTTPClient::HTTPResponse::getHeader() const
{
	return internal->header;
}
struct HTTPClient::HTTPClientInternal
{
	CURL* curl;
	shared_ptr<HTTPBuffer> headerbuf;

	HTTPClientInternal(const URL& url, int timeout, HTTPBufferCacheType type) :header(NULL)
	{
		response = new HTTPResponse(type);
		response->internal->errormsg = "Not Init";
		response->internal->code = 500;

		curl = curl_easy_init();
		if (curl != NULL)
		{
			curl_easy_setopt(curl, CURLOPT_URL, url.href().c_str());
			curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
			curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, timeout / 1000);
			curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout / 1000);
		}
	}
	virtual ~HTTPClientInternal()
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
		HTTPClientInternal* internal = dynamic_cast<HTTPClientInternal*>((HTTPClientInternal *)lpVoid);
		if( NULL == internal || NULL == buffer )
		{  
			return -1;  
		}  

		if (internal->headerbuf == NULL)
		{
			internal->headerbuf = new HTTPBuffer(HTTPBufferCacheType_Mem);
		}
		if (internal->headerbuf != NULL)
		{
			internal->headerbuf->write(std::string((char*)buffer, size * nmemb));
		}

		return size * nmemb;
	}   
	static size_t OnWriteData(void* buffer, size_t size, size_t nmemb, void* lpVoid)  
	{  
		HTTPClientInternal* internal = dynamic_cast<HTTPClientInternal*>((HTTPClientInternal *)lpVoid);
  		if( NULL == internal || NULL == buffer )
  		{  
  			return -1;  
  		}  

		internal->response->internal->buffer->write(std::string((char*)buffer, size * nmemb));

  		return size * nmemb;
	}   
	static size_t OnReadData(void* buffer, size_t size, size_t nmemb, void* lpVoid)  
	{  
		HTTPClientInternal* internal = dynamic_cast<HTTPClientInternal*>((HTTPClientInternal *)lpVoid);
		if (NULL == internal || NULL == buffer)
		{
			return -1;
		}
		if(internal->request == NULL || internal->request->buffer() == NULL)
		{
			return 0;
		} 
		shared_ptr<HTTPBuffer> inbuffer = internal->request->buffer();

		int readlen =  inbuffer->read((char*)buffer, size* nmemb);

		return readlen;
	}


	bool setHeader(const shared_ptr<HTTPRequest>& req)
	{
		if (req == NULL || curl == NULL)
		{
			return false;
		}

		for (std::map<HTTPHeaderType, URI::Value>::const_iterator iter = req->internal->header.begin(); iter != req->internal->header.end(); iter++)
		{
			switch (iter->first)
			{
			case HTTPHeader_Username:
				curl_easy_setopt(curl, CURLOPT_USERNAME, iter->second.readString().c_str());
				break;
			case HTTPHeader_Password:
				curl_easy_setopt(curl, CURLOPT_PASSWORD, iter->second.readString().c_str());
				break;
			case HTTPHeader_Content_Encoding:
				curl_easy_setopt(curl, CURLOPT_HTTP_CONTENT_DECODING, iter->second.readString().c_str());
				break;
			case HTTPHeader_User_Agent:
				curl_easy_setopt(curl, CURLOPT_USERAGENT, iter->second.readString().c_str());
				break;
			case HTTPHeader_Cookies:
				curl_easy_setopt(curl, CURLOPT_COOKIE, iter->second.readString().c_str());
				break;
			default:
				break;
			}
		}
		for (std::map<std::string, URI::Value>::const_iterator iter = req->internal->headertmp.begin(); iter != req->internal->headertmp.end(); iter++)
		{
			header = curl_slist_append(header, (iter->first + (": ") + iter->second.readString()).c_str());
		}

		if(header != NULL)
		{
			curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);		
		}

		return true;
	}
	void buildRequest(const std::string& method, const shared_ptr<HTTPRequest>& req)
	{
		if (curl == NULL)
		{
			return;
		}

		request = req;
		setHeader(req);

		if (strcasecmp(method.c_str(), "post") == 0)
		{
			curl_easy_setopt(curl, CURLOPT_POST, 1);
			curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, req->buffer()->totalSize());
		}
		else if (strcasecmp(method.c_str(), "put") == 0)
		{
			curl_easy_setopt(curl, CURLOPT_PUT, 1);
			curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, req->buffer()->totalSize());
		}
		else if (strcasecmp(method.c_str(), "delete") == 0)
		{
			response->internal->code = 500;
			response->internal->errormsg = "not suport the method";

			return ;
		}

		if (request != NULL && NULL != request->internal->form->internal->formpost)
		{
			curl_easy_setopt(curl, CURLOPT_HTTPPOST, (curl_httppost*)request->internal->form->internal->formpost);
		}

		curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, HTTPClientInternal::OnWriteHeader);
		curl_easy_setopt(curl, CURLOPT_WRITEHEADER, this);
		curl_easy_setopt(curl, CURLOPT_READFUNCTION, HTTPClientInternal::OnReadData);
		curl_easy_setopt(curl, CURLOPT_READDATA, this);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, HTTPClientInternal::OnWriteData);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);
	}

	shared_ptr<HTTPResponse> response;
	shared_ptr<HTTPRequest>  request;
	struct curl_slist* header;
};
  

HTTPClient::HTTPClient(const URL& url, int timeout, HTTPBufferCacheType type)
{
	internal = new HTTPClientInternal(url,timeout,type);	
}
HTTPClient::~HTTPClient()
{
	SAFE_DELETE(internal);
}

const shared_ptr<HTTPClient::HTTPResponse> HTTPClient::request(const std::string& method, const shared_ptr<HTTPRequest>& req)
{
	if (internal->curl == NULL)
	{
		return internal->response;
	}

	internal->buildRequest(method,req);

	CURLcode res = curl_easy_perform(internal->curl);
	curl_easy_getinfo(internal->curl, CURLINFO_HTTP_CODE, &internal->response->internal->code);
	if (res == CURLE_OK)
	{
		internal->response->internal->errormsg = "";
	}
	else
	{
		internal->response->internal->code = res;
		internal->response->internal->errormsg = curl_easy_strerror(res);
	}
	internal->response->internal->parseHttpHeader(internal->headerbuf);

	return internal->response;
}



struct HTTPClientAsync::MultiHttpClient : public Thread
{
	struct AsyncInfo :public HTTPClient::HTTPClientInternal
	{
		AsyncInfo(const URL& _url, int timeout, HTTPBufferCacheType type, void* _content) :HTTPClientInternal(_url, timeout, type),url(_url),context(_content){}

		URL				url;
		void *			context;
	};

	typedef std::map<CURL*, shared_ptr<AsyncInfo> > HttpMap;

	MultiHttpClient(const AsynHttpRequest& _callback, HTTPBufferCacheType _type)
		: Thread("MultiHttpClient")
		, curl_m(NULL)
	{
		callback = _callback;
		type = _type;

		curl_m = curl_multi_init();
		createThread();
	}

	~MultiHttpClient()
	{
		destroyThread();
		curl_multi_cleanup(curl_m);
	}

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
				std::list<shared_ptr<MultiHttpClient::AsyncInfo> > addErrorList;

				{
					Guard locker(mutex);
					while (waitList.size() > 0)
					{
						shared_ptr<MultiHttpClient::AsyncInfo> internal = waitList.front();
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

				for (std::list<shared_ptr<MultiHttpClient::AsyncInfo> >::iterator iter = addErrorList.begin(); iter != addErrorList.end(); iter++)
				{
					shared_ptr<MultiHttpClient::AsyncInfo> internal = *iter;
					internal->response->internal->code = 100;
					internal->response->internal->errormsg = "curl_multi_add_handle error";

					callback(internal->url, internal->response, internal->context);
				}
			}

			std::list<shared_ptr<MultiHttpClient::AsyncInfo> > doSuccessList;
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
							curl_easy_getinfo(internal->curl, CURLINFO_HTTP_CODE, &internal->response->internal->code);
							if (msg->data.result == CURLE_OK)
							{
								internal->response->internal->errormsg = "";
							}
							else
							{
								internal->response->internal->code = msg->data.result;
								internal->response->internal->errormsg = curl_easy_strerror(msg->data.result);
							}

							doSuccessList.push_back(internal);
						}
						curl_multi_remove_handle(curl_m, msg->easy_handle);
					}
				}
			}

			//dosuccess callback
			{
				for (std::list<shared_ptr<MultiHttpClient::AsyncInfo> >::iterator iter = doSuccessList.begin(); iter != doSuccessList.end(); iter++)
				{
					shared_ptr<MultiHttpClient::AsyncInfo> internal = *iter;
					callback(internal->url, internal->response, internal->context);
				}
			}
		}
	}

	bool addMultiRequest(const shared_ptr<MultiHttpClient::AsyncInfo> &internal)
	{
		Guard locker(mutex);
		waitList.push_back(internal);

		return true;
	}

public:
	CURLM*		curl_m;	
	AsynHttpRequest callback;
	HTTPBufferCacheType type;

	Mutex												mutex;
	HttpMap												mapHttp;
	std::list<shared_ptr<MultiHttpClient::AsyncInfo> >	waitList;
};

HTTPClientAsync::HTTPClientAsync(const AsynHttpRequest& callback, HTTPBufferCacheType type)
{
	internal = new HTTPClientAsync::MultiHttpClient(callback,type);
}
HTTPClientAsync::~HTTPClientAsync()
{
	SAFE_DELETE(internal);
}
bool HTTPClientAsync::requestAsyn(const URL& url, const std::string &method, int timeout, void* content, const shared_ptr<HTTPClient::HTTPRequest>& req)
{
	if (internal->curl_m == NULL)
	{
		return false;
	}

	shared_ptr<MultiHttpClient::AsyncInfo> asyncinfo(new MultiHttpClient::AsyncInfo(url,timeout,internal->type,content));

	if (asyncinfo->curl == NULL)
	{
		return false;
	}

	asyncinfo->buildRequest(method, req);
	internal->addMultiRequest(asyncinfo);

	return true;
}




 }
 }
 
 
