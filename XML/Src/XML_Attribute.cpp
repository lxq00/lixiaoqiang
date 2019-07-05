#include "XML/XML.h"
#include "XML/tinyxml.h"
#include "XMLTinyxml.h"
namespace Public{
namespace XML{

XMLObject::Attribute::Attribute(){}
XMLObject::Attribute::Attribute(const Attribute& attri)
{
	name = attri.name;
	value = attri.value;
}
XMLObject::Attribute::~Attribute(){}
bool XMLObject::Attribute::isEmpty() const
{
	return name == "" || value.empty();
}

bool XMLObject::Attribute::operator == (const Attribute& val)const
{
	return name == val.name && value == val.value;
}
XMLObject::Attribute::operator bool() const
{
	return !isEmpty();
}

}
}
