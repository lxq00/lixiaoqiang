#include <stdio.h>
#include "XML/XML.h"
namespace Public{
namespace XML{

struct XMLObject::Value::ValueInternal
{
	ValueInternal():empty(true){}
	~ValueInternal(){}

	bool		empty;
	std::string value;
};
XMLObject::Value::Value()
{
	internal = new ValueInternal();
}
XMLObject::Value::Value(const char* val)
{
	internal = new ValueInternal();

	if(val != NULL)
	{
		internal->value = val;
		internal->empty = false;
	}
}
XMLObject::Value::Value(const std::string& val)
{
	internal = new ValueInternal();

	internal->value = val;
	internal->empty = false;
}
XMLObject::Value::Value(int val)
{
	internal = new ValueInternal();
	internal->empty = false;

	char buffer[32];
	sprintf(buffer,"%d",val);
	internal->value = buffer;
}
XMLObject::Value::Value(unsigned int val)
{
	internal = new ValueInternal();
	internal->empty = false;

	char buffer[32];
	sprintf(buffer,"%u",val);
	internal->value = buffer;
}
XMLObject::Value::Value(unsigned long long val)
{
	internal = new ValueInternal();
	internal->empty = false;

	char buffer[32];
	sprintf(buffer,"%llu",val);
	internal->value = buffer;
}
XMLObject::Value::Value(long long val)
{
	internal = new ValueInternal();
	internal->empty = false;

	char buffer[32];
	sprintf(buffer,"%lld",val);
	internal->value = buffer;
}
XMLObject::Value::Value(float val)
{
	internal = new ValueInternal();
	internal->empty = false;

	char buffer[32];
	sprintf(buffer,"%f",val);
	internal->value = buffer;
}
XMLObject::Value::Value(char val)
{
	internal = new ValueInternal();
	internal->empty = false;

	char buffer[32];
	sprintf(buffer,"%c",val);
	internal->value = buffer;
}
XMLObject::Value::Value(double val)
{
	internal = new ValueInternal();
	internal->empty = false;

	char buffer[32];
	sprintf(buffer,"%llf",val);
	internal->value = buffer;
}
XMLObject::Value::Value(const Value& val)
{
	internal = new ValueInternal();
	internal->empty = val.internal->empty;
	internal->value = val.internal->value;
}
XMLObject::Value::~Value()
{
	delete internal;
}

std::string XMLObject::Value::toString() const
{
	return internal->value;
}
int XMLObject::Value::toInt32() const
{
	int val;
	if(internal->empty || sscanf(internal->value.c_str(),"%d",&val) != 1)
	{
		return 0;
	}

	return val;
}
unsigned int XMLObject::Value::toUint32() const
{
	unsigned int val;
	if(internal->empty || sscanf(internal->value.c_str(),"%u",&val) != 1)
	{
		return 0;
	}

	return val;
}
unsigned long long XMLObject::Value::toUint64() const
{
	unsigned long long val;
	if(internal->empty || sscanf(internal->value.c_str(),"%llu",&val) != 1)
	{
		return 0;
	}

	return val;
}
long long XMLObject::Value::toInt64() const
{
	long long val;
	if(internal->empty || sscanf(internal->value.c_str(),"%lld",&val) != 1)
	{
		return 0;
	}

	return val;
}
float XMLObject::Value::toFloat() const
{
	float val;
	if(internal->empty || sscanf(internal->value.c_str(),"%f",&val) != 1)
	{
		return 0;
	}

	return val;
}
char XMLObject::Value::toChar() const
{
	char val;
	if(internal->empty || sscanf(internal->value.c_str(),"%c",&val) != 1)
	{
		return 0;
	}

	return val;
}
double XMLObject::Value::toDouble() const
{
	double val;
	if(internal->empty || sscanf(internal->value.c_str(),"%llf",&val) != 1)
	{
		return 0;
	}

	return val;
}

bool XMLObject::Value::isEmpty() const
{
	return internal->empty;
}

XMLObject::Value& XMLObject::Value::operator = (const XMLObject::Value& val)
{
	*internal = *val.internal;

	return *this;
}

bool XMLObject::Value::operator == (const XMLObject::Value& val)const
{
	return (internal->value == val.internal->value);
}

}
}
