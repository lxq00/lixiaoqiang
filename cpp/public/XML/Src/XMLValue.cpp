#include <stdio.h>
#include "XML/XML.h"
namespace Xunmei{
namespace XML{

struct XM_XML::Value::ValueInternal
{
	ValueInternal():empty(true){}
	~ValueInternal(){}

	bool		empty;
	std::string value;
};
XM_XML::Value::Value()
{
	internal = new ValueInternal();
}
XM_XML::Value::Value(const char* val)
{
	internal = new ValueInternal();

	if(val != NULL)
	{
		internal->value = val;
		internal->empty = false;
	}
}
XM_XML::Value::Value(const std::string& val)
{
	internal = new ValueInternal();

	internal->value = val;
	internal->empty = false;
}
XM_XML::Value::Value(int val)
{
	internal = new ValueInternal();
	internal->empty = false;

	char buffer[32];
	sprintf(buffer,"%d",val);
	internal->value = buffer;
}
XM_XML::Value::Value(unsigned int val)
{
	internal = new ValueInternal();
	internal->empty = false;

	char buffer[32];
	sprintf(buffer,"%u",val);
	internal->value = buffer;
}
XM_XML::Value::Value(unsigned long long val)
{
	internal = new ValueInternal();
	internal->empty = false;

	char buffer[32];
	sprintf(buffer,"%llu",val);
	internal->value = buffer;
}
XM_XML::Value::Value(long long val)
{
	internal = new ValueInternal();
	internal->empty = false;

	char buffer[32];
	sprintf(buffer,"%lld",val);
	internal->value = buffer;
}
XM_XML::Value::Value(float val)
{
	internal = new ValueInternal();
	internal->empty = false;

	char buffer[32];
	sprintf(buffer,"%f",val);
	internal->value = buffer;
}
XM_XML::Value::Value(char val)
{
	internal = new ValueInternal();
	internal->empty = false;

	char buffer[32];
	sprintf(buffer,"%c",val);
	internal->value = buffer;
}
XM_XML::Value::Value(double val)
{
	internal = new ValueInternal();
	internal->empty = false;

	char buffer[32];
	sprintf(buffer,"%llf",val);
	internal->value = buffer;
}
XM_XML::Value::Value(const Value& val)
{
	internal = new ValueInternal();
	internal->empty = val.internal->empty;
	internal->value = val.internal->value;
}
XM_XML::Value::~Value()
{
	delete internal;
}

std::string XM_XML::Value::toString() const
{
	return internal->value;
}
int XM_XML::Value::toInt32() const
{
	int val;
	if(internal->empty || sscanf(internal->value.c_str(),"%d",&val) != 1)
	{
		return 0;
	}

	return val;
}
unsigned int XM_XML::Value::toUint32() const
{
	unsigned int val;
	if(internal->empty || sscanf(internal->value.c_str(),"%u",&val) != 1)
	{
		return 0;
	}

	return val;
}
unsigned long long XM_XML::Value::toUint64() const
{
	unsigned long long val;
	if(internal->empty || sscanf(internal->value.c_str(),"%llu",&val) != 1)
	{
		return 0;
	}

	return val;
}
long long XM_XML::Value::toInt64() const
{
	long long val;
	if(internal->empty || sscanf(internal->value.c_str(),"%lld",&val) != 1)
	{
		return 0;
	}

	return val;
}
float XM_XML::Value::toFloat() const
{
	float val;
	if(internal->empty || sscanf(internal->value.c_str(),"%f",&val) != 1)
	{
		return 0;
	}

	return val;
}
char XM_XML::Value::toChar() const
{
	char val;
	if(internal->empty || sscanf(internal->value.c_str(),"%c",&val) != 1)
	{
		return 0;
	}

	return val;
}
double XM_XML::Value::toDouble() const
{
	double val;
	if(internal->empty || sscanf(internal->value.c_str(),"%llf",&val) != 1)
	{
		return 0;
	}

	return val;
}

bool XM_XML::Value::isEmpty() const
{
	return internal->empty;
}

XM_XML::Value& XM_XML::Value::operator = (const XM_XML::Value& val)
{
	*internal = *val.internal;

	return *this;
}

bool XM_XML::Value::operator == (const XM_XML::Value& val)const
{
	return (internal->value == val.internal->value);
}

}
}
