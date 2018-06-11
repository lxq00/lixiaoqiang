#include "Base/URI.h"
#include "Base/URLEncoding.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

namespace Public{
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
struct URI::Value::ValueInternal
{
	Type		type;
	union {
		uint64_t	_int;
		double		_float;
		bool		_bool;
		char*		_str;
	}val;

	ValueInternal() :type(Type_Empty) { val._int = 0; }
};
URI::Value::Value()
{
	internal = new ValueInternal();
	internal->type = Type_Empty;
}
URI::Value::Value(const std::string& val, URI::Value::Type type)
{
	internal = new ValueInternal();
	internal->type = type;
	switch (type)
	{
		case Type_Char:
		case Type_Int32:
		case Type_Int64:
			sscanf(val.c_str(), "%llu", &internal->val._int);
			break;		
		case Type_String:
			internal->val._str = new char[val.length() + 1];
			strcpy(internal->val._str, val.c_str());
			break;
		case Type_Double:
			sscanf(val.c_str(), "%lf", &internal->val._float);
			break;
		case Type_Bool:
			internal->val._bool = strcasecmp(val.c_str(), "true") == 0;
			break;
	}
}
URI::Value::Value(const char* val)
{
	internal = new ValueInternal();
	if (val != NULL)
	{
		internal->val._str = new char[strlen(val) + 1];
		strcpy(internal->val._str, val);
		internal->type = Type_String;
	}
}
URI::Value::Value(const std::string& val)
{
	internal = new ValueInternal();
	{
		internal->val._str = new char[val.length() + 1];
		strcpy(internal->val._str, val.c_str());
		internal->type = Type_String;
	}
}

URI::Value::Value(const unsigned char* val)
{
	internal = new ValueInternal();
	if (val != NULL)
	{
		internal->val._str = new char[strlen((const char*)val) + 1];
		strcpy(internal->val._str, (const char*)val);
		internal->type = Type_String;
	}
}
URI::Value::Value(char val)
{
	internal = new ValueInternal();
	internal->type = Type_Char;
	internal->val._int = (uint64_t)val;
}
URI::Value::Value(int val)
{
	internal = new ValueInternal();
	internal->type = Type_Int32;
	internal->val._int = (uint64_t)val;
}
URI::Value::Value(double val)
{
	internal = new ValueInternal();
	internal->type = Type_Double;
	internal->val._float = (double)val;
}
URI::Value::Value(long long val)
{
	internal = new ValueInternal();
	internal->type = Type_Int64;
	internal->val._int = (uint64_t)val;
}
URI::Value::Value(uint32_t val)
{
	internal = new ValueInternal();
	internal->type = Type_Int32;
	internal->val._int = (uint64_t)val;
}
URI::Value::Value(uint64_t val)
{
	internal = new ValueInternal();
	internal->type = Type_Int64;
	internal->val._int = (uint64_t)val;
}
URI::Value::Value(bool val)
{
	internal = new ValueInternal();
	internal->type = Type_Bool;
	internal->val._bool = val;
}

URI::Value::Value(const Value& val)
{
	internal = new ValueInternal();
	internal->type = val.internal->type;
	internal->val = val.internal->val;
	if (internal->type == Type_String)
	{
		internal->val._str = new char[strlen(val.internal->val._str) + 1];
		strcpy(internal->val._str, val.internal->val._str);
	}
}

URI::Value::~Value()
{
	if (internal->type == Type_String)
		delete[]internal->val._str;
}

URI::Value& URI::Value::operator = (const Value& val)
{
	if (internal->type == Type_String)
		delete[]internal->val._str;

	internal->type = val.internal->type;
	internal->val = val.internal->val;
	if (internal->type == Type_String)
	{
		internal->val._str = new char[strlen(val.internal->val._str) + 1];
		strcpy(internal->val._str, val.internal->val._str);
	}

	return *this;
}

URI::Value::Type URI::Value::type() const
{
	return internal->type;
}
std::string URI::Value::readString(const std::string& fmt) const
{
	switch (internal->type)
	{
	case Type_Char:
	{
		char buffer[32] = { 0 };
		snprintf(buffer, 31, fmt == "" ? "%c" : fmt.c_str(), (char)internal->val._int);
		return buffer;
	}
	case Type_Int32:
	{
		char buffer[32] = { 0 };
		snprintf(buffer, 31, fmt == "" ? "%d" : fmt.c_str(), (int)internal->val._int);
		return buffer;
	}
	case Type_Int64:
	{
		char buffer[32] = { 0 };
		snprintf(buffer, 31, fmt == "" ? "%lld" : fmt.c_str(), (int64_t)internal->val._int);
		return buffer;
	}
	case Type_String:
		return internal->val._str;
	case Type_Double:
	{
		char buffer[32] = { 0 };
		snprintf(buffer, 31, fmt == "" ? "%lf" : fmt.c_str(), internal->val._float);
		return buffer;
	}
	case Type_Bool:
		return internal->val._bool ? "true" : "false";
	}

	return "";
}
int URI::Value::readInt(const std::string& fmt) const
{
	switch (internal->type)
	{
	case Type_Char:
	case Type_Int32:
	case Type_Int64:
		return (int)internal->val._int;
	case Type_String:
	{
		int val = 0;
		sscanf(internal->val._str, fmt == "" ? "%d" : fmt.c_str(),&val);
		return val;
	}
	case Type_Double:
		return (int)internal->val._float;
	case Type_Bool:
		return internal->val._bool ? 1 : 0;
	}

	return 0;
}
float URI::Value::readFloat(const std::string& fmt) const
{
	switch (internal->type)
	{
	case Type_Char:
	case Type_Int32:
	case Type_Int64:
		return (float)internal->val._int;
	case Type_String:
	{
		float val = 0;
		sscanf(internal->val._str, fmt == "" ? "%f" : fmt.c_str(), &val);
		return val;
	}
	case Type_Double:
		return (float)internal->val._float;
	case Type_Bool:
		return internal->val._bool ? (float)1.0 : (float)0.0;
	}

	return (float)0.0;
}

long long URI::Value::readInt64(const std::string& fmt) const
{
	switch (internal->type)
	{
	case Type_Char:
	case Type_Int32:
	case Type_Int64:
		return (long long)internal->val._int;
	case Type_String:
	{
		long long val = 0;
		sscanf(internal->val._str, fmt == "" ? "%lld" : fmt.c_str(), &val);
		return val;
	}
	case Type_Double:
		return (long long)internal->val._float;
	case Type_Bool:
		return internal->val._bool ? 1 : 0;
	}

	return 0;
}

uint32_t URI::Value::readUint32(const std::string& fmt) const
{
	switch (internal->type)
	{
	case Type_Char:
	case Type_Int32:
	case Type_Int64:
		return (uint32_t)internal->val._int;
	case Type_String:
	{
		uint32_t val = 0;
		sscanf(internal->val._str, fmt == "" ? "%u" : fmt.c_str(), &val);
		return val;
	}
	case Type_Double:
		return (uint32_t)internal->val._float;
	case Type_Bool:
		return internal->val._bool ? 1 : 0;
	}

	return 0;
}

uint64_t URI::Value::readUint64(const std::string& fmt) const
{
	switch (internal->type)
	{
	case Type_Char:
	case Type_Int32:
	case Type_Int64:
		return (uint64_t)internal->val._int;
	case Type_String:
	{
		uint64_t val = 0;
		sscanf(internal->val._str, fmt == "" ? "%llu" : fmt.c_str(), &val);
		return val;
	}
	case Type_Double:
		return (uint64_t)internal->val._float;
	case Type_Bool:
		return internal->val._bool ? 1 : 0;
	}

	return 0;
}
bool URI::Value::readBool() const
{
	switch (internal->type)
	{
	case Type_Char:
	case Type_Int32:
	case Type_Int64:
		return internal->val._int != 0;
	case Type_String:
		return strcasecmp(internal->val._str, "true") == 0;
	case Type_Double:
		return (int)internal->val._float != 0;
	case Type_Bool:
		return internal->val._bool;
	}

	return false;
}
bool URI::Value::isEmpty() const
{
	return internal->type == Type_Empty;
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

URI::URI(const std::string& protocol,const Value& host,int port)
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


