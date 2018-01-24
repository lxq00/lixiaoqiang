#include "Base/URI.h"
#include "Base/URLEncoding.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

namespace Xunmei{
namespace Base{
	
#ifdef WIN32
#define strcasecmp _stricmp
#define snprintf _snprintf
#endif
bool checkUrlKeyIsValid(const std::string& key)
{
	char invaildList[] = {' ','/','?','@','~','!','#','$','^','*','&','(',')','[',']','{','}',
		'/','\\','\r','\n','\t',';',':','\"',',','<','>','|','='};

	for(unsigned int i = 0;i < sizeof(invaildList)/sizeof(char);i ++)
	{
		const char* tmp = strchr(key.c_str(),invaildList[i]);
		if(tmp != NULL)
		{
			return false;
		}
	}

	return true;
}

const char* findNodeInString(const char* str,std::string& val,const char* breakstr,const char* endstr,bool& end)
{
	if(str == NULL)
	{
		end = true;
		return NULL;
	}
	
	end = false;

	const char* tmp = str;
	while(1)
	{
		bool isbreak = (strchr(breakstr,*tmp) != NULL || strchr(endstr,*tmp) != NULL || *tmp == 0);
		
		if(isbreak)
		{
			val = std::string(str,tmp - str);
			end = (strchr(endstr,*tmp) != NULL || *tmp == 0);

			break;
		}
		tmp ++;
	}

	return *tmp ? tmp + 1 : NULL;
}

URI::Value::Value(const char* val)
{
	valuestring = val;
}
URI::Value::Value(const std::string& val)
{
	valuestring = val.c_str();
}

URI::Value::Value(const unsigned char* val)
{
	valuestring = (const char*)val;
}
URI::Value::Value(char val)
{
	char tmp[32];
	sprintf(tmp,"%c",val);
	valuestring = tmp;
}
URI::Value::Value(int val)
{
	char tmp[32];
	sprintf(tmp,"%d",val);
	valuestring = tmp;
}
URI::Value::Value(float val)
{
	char tmp[32];
	sprintf(tmp,"%f",val);
	valuestring = tmp;
}
URI::Value::Value(long long val)
{
	char tmp[32];
	sprintf(tmp,"%lld",val);
	valuestring = tmp;
}
URI::Value::Value(uint32_t val)
{
	char tmp[32];
	sprintf(tmp,"%u",val);
	valuestring = tmp;
}
URI::Value::Value(uint64_t val)
{
	unsigned long long vval = val;

	char tmp[32];
	sprintf(tmp,"%llu",vval);
	valuestring = tmp;
}
URI::Value::Value(const Value& val)
{
	valuestring = val.valuestring;
}

URI::Value::~Value(){}
std::string URI::Value::readString() const
{
	return valuestring;
}
int URI::Value::readInt() const
{
	int val = 0;
	sscanf(valuestring.c_str(),"%d",&val);

	return val;
}
float URI::Value::readFloat() const
{
	float val = 0;
	sscanf(valuestring.c_str(),"%f",&val);

	return val;
}

long long URI::Value::readInt64() const
{
	unsigned long long val = 0;
	sscanf(valuestring.c_str(),"%llu",&val);

	return val;
}

uint32_t URI::Value::readUint32() const
{
	uint32_t val = 0;
	sscanf(valuestring.c_str(),"%u",&val);

	return val;
}

uint64_t URI::Value::readUint64() const
{
	unsigned long long val = 0;
	sscanf(valuestring.c_str(),"%llu",&val);

	return val;
}
bool URI::Value::isEmpty() const
{
	return valuestring.length() <= 0;
}

std::string URI::URIObject::toStringAsIdentifier()
{
	return key + std::string("/") + val.readString();
}
std::string URI::URIObject::toStringAsParameter()
{
	return key + std::string("=") + val.readString();
}

struct URI::URIInternal
{
	URIInternal():port(0){}
	void clean()
	{
		protocol = "";
		host = std::string("");
		port = 0;
		identifier.clear();
		parameter.clear();
	}

	std::string protocol;
	Value host;
	int port;

	std::list<URI::URIObject> identifier;
	std::list<URI::URIObject> parameter;
};

URI::URI(const std::string& protocol,const std::string& host,int port)
{
	internal = new URIInternal();
	if(!checkUrlKeyIsValid(protocol))
	{
		internal->protocol = "VDR";
	}
	else
	{
		internal->protocol = protocol;
	}
	internal->protocol = protocol;
	internal->host = host;
	internal->port = port;
}
URI::URI(const std::string& protocol,int hostId)
{
	internal = new URIInternal();
	internal->protocol = protocol;	
	internal->host = hostId;
}
URI::URI(const URI& uri)
{
	internal = new URIInternal();
	internal->protocol = uri.internal->protocol;
	internal->host = uri.internal->host;
	internal->port = uri.internal->port;
	internal->identifier = uri.internal->identifier;
	internal->parameter = uri.internal->parameter;
}
URI::~URI()
{
	delete internal;
}

URI& URI::operator = (const URI& uri)
{
	internal->protocol = uri.internal->protocol;
	internal->host = uri.internal->host;
	internal->port = uri.internal->port;
	internal->identifier = uri.internal->identifier;
	internal->parameter = uri.internal->parameter;

	return *this;
}
bool URI::setProtocol(const std::string& protocol)
{
	if(!checkUrlKeyIsValid(protocol))
	{
		return false;
	}
	
	internal->protocol = protocol;

	return true;
}
bool URI::setHost(const std::string& host,int port)
{
	internal->host = host;
	internal->port = port;

	return true;
}
bool  URI::setCenterId(int centerId)
{
	return setHost(centerId);
}
bool URI::setHost(int hostId)
{
	internal->host = hostId;

	return true;
}

bool URI::addIdentifier(const std::string& mark,const Value& val)
{
	if(!checkUrlKeyIsValid(mark) || !checkUrlKeyIsValid(val.readString()) || val.isEmpty())
	{
		return false;
	}

	std::list<URI::URIObject>::iterator iter;
	for(iter = internal->identifier.begin();iter != internal->identifier.end();iter ++)
	{
		if(strcasecmp(iter->key.c_str(),mark.c_str()) == 0)
		{
			iter->val = val;
			return true;
		}
	}

	URI::URIObject obj;
	obj.key = mark;
	obj.val = val;
	internal->identifier.push_back(obj);

	return true;
}
bool URI::addParameter(const std::string& key,const Value& val)
{
	if(!checkUrlKeyIsValid(key))
	{
		return false;
	}

	std::list<URI::URIObject>::iterator iter;
	for(iter = internal->parameter.begin();iter != internal->parameter.end();iter ++)
	{
		if(strcasecmp(iter->key.c_str(),key.c_str()) == 0)
		{
			iter->val = val;
			return true;
		}
	}

	URI::URIObject obj;
	obj.key = key;
	obj.val = val;
	internal->parameter.push_back(obj);

	return true;
}

bool URI::removeIndentifier(const std::string& mark)
{
	if(!checkUrlKeyIsValid(mark))
	{
		return false;
	}

	std::list<URI::URIObject>::iterator iter;
	for(iter = internal->identifier.begin();iter != internal->identifier.end();iter ++)
	{
		if(strcasecmp(iter->key.c_str(),mark.c_str()) == 0)
		{
			internal->identifier.erase(iter);
			break;
		}
	}

	return true;
}

bool URI::removeParameter(const std::string& key)
{
	if(!checkUrlKeyIsValid(key))
	{
		return false;
	}

	std::list<URI::URIObject>::iterator iter;
	for(iter = internal->parameter.begin();iter != internal->parameter.end();iter ++)
	{
		if(strcasecmp(iter->key.c_str(),key.c_str()) == 0)
		{
			internal->parameter.erase(iter);
			break;
		}
	}

	return true;
}

std::string URI::getParmeterString() const
{
#define URISTRINGMAXSIZE 2048
	char strbufer[URISTRINGMAXSIZE] = {0};

	std::list<URI::URIObject>::iterator iter;
	for(iter = internal->parameter.begin();iter != internal->parameter.end();iter ++)
	{
		std::string tmp = URLEncoding::encode(iter->val.readString());
		snprintf(strbufer + strlen(strbufer),URISTRINGMAXSIZE - strlen(strbufer),"%s%s=%s",
			iter == internal->parameter.begin() ? "":"&",iter->key.c_str(),tmp.c_str());
	}

	return strbufer;
}
bool URI::addParmeterString(const std::string& exstring)
{
	const char* urltmp = exstring.c_str();
	bool isEnd = false;
	do{
		std::string key;
		std::string val;
		isEnd = false;

		urltmp = findNodeInString(urltmp,key,"=","",isEnd);
		if(!isEnd)
		{
			urltmp = findNodeInString(urltmp,val,"&","",isEnd);
		}
		
		if(key != "")
		{
			URI::URIObject obj;
			obj.key = key;
			obj.val = URLEncoding::decode(val);

			internal->parameter.push_back(obj);
		}		
	}while(!isEnd);

	return true;
}

URI::Value* URI::getIdentifier(const std::string& mark) const
{
	if(!checkUrlKeyIsValid(mark))
	{
		return NULL;
	}

	std::list<URI::URIObject>::iterator iter;
	for(iter = internal->identifier.begin();iter != internal->identifier.end();iter ++)
	{
		if(strcasecmp(iter->key.c_str(),mark.c_str()) == 0)
		{
			return &iter->val;
		}
	}

	return NULL;
}

URI::Value* URI::getParameter(const std::string& key) const
{
	if(!checkUrlKeyIsValid(key))
	{
		return NULL;
	}

	std::list<URI::URIObject>::iterator iter;
	for(iter = internal->parameter.begin();iter != internal->parameter.end();iter ++)
	{
		if(strcasecmp(iter->key.c_str(),key.c_str()) == 0)
		{
			return &iter->val;
		}
	}

	return NULL;
}

std::string& URI::getProtocol() const
{
	return internal->protocol;
}

URI::Value URI::getHost() const
{
	return internal->host;
}

int URI::getCenterId() const
{
	return internal->host.readInt();
}

int URI::getPort() const
{
	return internal->port;
}

std::list<URI::URIObject> URI::getIndentifierList() const
{
	return internal->identifier;
}

std::list<URI::URIObject> URI::getParmeterList() const
{
	return internal->parameter;
}

std::string URI::toString() const
{
	if(internal->protocol == "")
	{
		internal->protocol = "VDR";
	}
#define URISTRINGMAXSIZE 2048
	char strbufer[URISTRINGMAXSIZE] = {0};
	snprintf(strbufer + strlen(strbufer),URISTRINGMAXSIZE - strlen(strbufer),"%s://%s",internal->protocol.c_str(),internal->host.readString().c_str());

	if(internal->port != 0)
	{
		snprintf(strbufer + strlen(strbufer),URISTRINGMAXSIZE - strlen(strbufer),":%d",internal->port);
	}

	
	std::list<URI::URIObject>::iterator iter;
	for(iter = internal->identifier.begin();iter != internal->identifier.end();iter ++)
	{
		snprintf(strbufer + strlen(strbufer),URISTRINGMAXSIZE - strlen(strbufer),"/%s/%s",iter->key.c_str(),iter->val.readString().c_str());
	}

	for(iter = internal->parameter.begin();iter != internal->parameter.end();iter ++)
	{
		std::string tmp = URLEncoding::encode(iter->val.readString());
		snprintf(strbufer + strlen(strbufer),URISTRINGMAXSIZE - strlen(strbufer),"%c%s=%s",
			iter == internal->parameter.begin() ? '?':'&',iter->key.c_str(),tmp.c_str());
	}

	return strbufer;
}

std::string URI::getBasicString() const
{
	if(internal->protocol == "")
	{
		internal->protocol = "VDR";
	}
#define URISTRINGMAXSIZE 2048
	char strbufer[URISTRINGMAXSIZE] = {0};
	snprintf(strbufer + strlen(strbufer),URISTRINGMAXSIZE - strlen(strbufer),"%s://%s",internal->protocol.c_str(),internal->host.readString().c_str());

	if(internal->port != 0)
	{
		snprintf(strbufer + strlen(strbufer),URISTRINGMAXSIZE - strlen(strbufer),":%d",internal->port);
	}


	std::list<URI::URIObject>::iterator iter;
	for(iter = internal->identifier.begin();iter != internal->identifier.end();iter ++)
	{
		snprintf(strbufer + strlen(strbufer),URISTRINGMAXSIZE - strlen(strbufer),"/%s/%s",iter->key.c_str(),iter->val.readString().c_str());
	}

	return strbufer;
}

bool URI::parse(const std::string& uri)
{
	internal->clean();
	
	const char* urltmp = uri.c_str();

	//先解析协议
	const char* tmp = strstr(urltmp,"://");
	if(tmp == NULL)
	{
		return false;
	}

	internal->protocol = std::string(urltmp,tmp-urltmp);

	urltmp = tmp + 3;

	//在解析host和port
	tmp = strchr(urltmp,'/');
	if(tmp == NULL)
	{
		return false;
	}

	std::string hoststr(urltmp,tmp - urltmp);
	const char* porttmp = strchr(hoststr.c_str(),':');
	if(porttmp == NULL)
	{
		internal->host = hoststr;
	}
	else
	{
		internal->host = std::string(hoststr.c_str(),porttmp - hoststr.c_str());
		internal->port = atoi(porttmp + 1);
	}

	urltmp = tmp + 1;
	bool isEnd = false;
	//开始解析identifier
	do{
		std::string key;
		std::string val;
		isEnd = false;

		urltmp = findNodeInString(urltmp,key,"/","?",isEnd);

		if(!isEnd)
		{
			urltmp = findNodeInString(urltmp,val,"/","?",isEnd);
		}
		if(key != "")
		{
			URI::URIObject obj;
			obj.key = key;
			obj.val = val;

			internal->identifier.push_back(obj);
		}
		
	}while(!isEnd);


	//开始解析parameter
	do{
		std::string key;
		std::string val;
		isEnd = false;

		urltmp = findNodeInString(urltmp,key,"=","?",isEnd);
		if(!isEnd)
		{
			urltmp = findNodeInString(urltmp,val,"&","?",isEnd);
		}
		
		if(key != "")
		{
			URI::URIObject obj;
			obj.key = key;
			obj.val = URLEncoding::decode(val);

			internal->parameter.push_back(obj);
		}		
	}while(!isEnd);

	return internal->identifier.size() > 0;
}



}
}


