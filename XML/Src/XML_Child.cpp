#include "XML/XML.h"
#include "XML/tinyxml.h"
#include "XMLTinyxml.h"
namespace Public{
namespace XML{

struct XMLObject::Child::ChildInternal
{
	ChildInternal():getChildIndex(0),getAttributeIndex(0){}
	~ChildInternal()
	{
		std::list<XMLObject::Child*>::iterator iter;
		for(iter = childList.begin();iter != childList.end();iter ++)
		{
			delete (*iter);
		}
	}

	std::list<XMLObject::Attribute>	attributeList;
	std::list<XMLObject::Child*>	childList;
	std::string						name;
	std::string 					value;
	int								getChildIndex;
	int								getAttributeIndex;
	std::string						searchChildName;


	XMLObject::Child& getChild(const std::string& name, int index)
	{
		int getIndex = 0;
		std::list<XMLObject::Child*>::iterator iter;
		for (iter = childList.begin(); iter != childList.end(); iter++)
		{
			if ((*iter)->internal->name == name)
			{
				if (getIndex == index)
				{
					return **iter;
				}
				getIndex++;
			}
		}

		static Child emptyChild;

		return emptyChild;
	}

	Value& attribute(const std::string& key)
	{
		std::list<XMLObject::Attribute>::iterator iter;
		for (iter = attributeList.begin(); iter != attributeList.end(); iter++)
		{
			if (iter->name == key)
			{
				return iter->value;
			}
		}

		static Value emptyAttri;

		return emptyAttri;
	}

	XMLObject::Child& firstChild(const std::string& childname)
	{
		static Child emptyChild;

		if (childList.size() <= 0)
		{
			return emptyChild;
		}

		getChildIndex = 0;
		searchChildName = childname;
		for (std::list<XMLObject::Child*>::const_iterator iter = childList.begin(); iter != childList.end(); iter++, getChildIndex++)
		{
			if (searchChildName == "" || searchChildName == (*iter)->name()) return **iter;
		}

		return emptyChild;
	}
	XMLObject::Child& nextChild()
	{
		static Child emptyChild;
		int getIndex = 0;

		std::list<XMLObject::Child*>::iterator iter;
		for (iter = childList.begin(); iter != childList.end(); iter++, getIndex++)
		{
			if(getIndex < getChildIndex) continue;
			getChildIndex++;

			if (searchChildName == "" || searchChildName == (*iter)->name()) return **iter;

		}

		return emptyChild;
	}

	XMLObject::Attribute& firstAttribute()
	{
		static XMLObject::Attribute empltyAttribute;
		if (attributeList.size() <= 0)
		{
			return empltyAttribute;
		}

		std::list<XMLObject::Attribute>::iterator iter = attributeList.begin();
		getAttributeIndex = 0;

		return *iter;
	}
	XMLObject::Attribute& nextAttribute()
	{
		static XMLObject::Attribute empltyAttribute;
		int getindex = 0;
		std::list<XMLObject::Attribute>::iterator iter;
		for (iter = attributeList.begin(); iter != attributeList.end(); iter++, getindex++)
		{
			if (getindex < getAttributeIndex) continue;

			getAttributeIndex++;

			return *iter;
		}
		;
		return empltyAttribute;
	}
};
XMLObject::Child::Child(const std::string& name)
{
	internal = new ChildInternal;
	internal->name = name;
}
XMLObject::Child::Child(const Child& child)
{
	internal = new ChildInternal;
	internal->name = child.internal->name;
	internal->attributeList = child.internal->attributeList;
	internal->value = child.internal->value;
	
	std::list<XMLObject::Child*>::iterator iter;
	for(iter = child.internal->childList.begin();iter != child.internal->childList.end();iter ++)
	{
		XMLObject::Child* node = new XMLObject::Child(**iter);
		internal->childList.push_back(node);
	}
}
XMLObject::Child::~Child()
{
	delete internal;
}

void XMLObject::Child::name(const std::string& _name)
{
	internal->name = _name;
}
std::string XMLObject::Child::name() const
{
	return internal->name;
}
void XMLObject::Child::data(const Value& value)
{
	internal->value = value.readString();
}
Value XMLObject::Child::data() const
{
	return internal->value == "" ? Value() : Value(internal->value);
}
XMLObject::Child::operator Value() const
{
	return data();
}
XMLObject::Child& XMLObject::Child::addChild(const XMLObject::Child& child)
{
	XMLObject::Child* node = new XMLObject::Child(child);

	internal->childList.push_back(node);

	return *node;
}
XMLObject::Child& XMLObject::Child::getChild(const std::string& name,int index)
{
	return internal->getChild(name, index);
}

const XMLObject::Child& XMLObject::Child::getChild(const std::string& name, int index)const
{
	return internal->getChild(name, index);
}

void XMLObject::Child::removeChild(const std::string& name,int index)
{
	int getIndex = 0;
	std::list<XMLObject::Child*>::iterator iter;
	for(iter = internal->childList.begin();iter != internal->childList.end();iter ++)
	{
		if((*iter)->internal->name == name)
		{
			if(getIndex == index)
			{
				delete *iter;
				internal->childList.erase(iter);
				break;
			}
			getIndex ++;
		}
	}
}
int XMLObject::Child::childCount() const
{
	return internal->childList.size();
}

int XMLObject::Child::attributeCount() const
{
	return internal->attributeList.size();
}

void XMLObject::Child::attribute(const std::string& key,const Value& val)
{
	std::list<XMLObject::Attribute>::iterator iter;
	for(iter = internal->attributeList.begin();iter != internal->attributeList.end();iter ++)
	{
		if(iter->name == key)
		{
			iter->value = val;
			return;
		}
	}

	XMLObject::Attribute attri;
	attri.name = key;
	attri.value = val;

	internal->attributeList.push_back(attri);
}
Value& XMLObject::Child::attribute(const std::string& key)
{
	return internal->attribute(key);
}
const Value& XMLObject::Child::attribute(const std::string& key)const
{
	return internal->attribute(key);
}

void XMLObject::Child::removeAttribute(const std::string& key)
{
	std::list<XMLObject::Attribute>::iterator iter;
	for(iter = internal->attributeList.begin();iter != internal->attributeList.end();iter ++)
	{
		if(iter->name == key)
		{
			internal->attributeList.erase(iter);
			break;
		}
	}
}

XMLObject::Child& XMLObject::Child::firstChild(const std::string& childname)
{
	return internal->firstChild(childname);
}
const XMLObject::Child& XMLObject::Child::firstChild(const std::string& childname)const
{
	return internal->firstChild(childname);
}
XMLObject::Child& XMLObject::Child::nextChild()
{
	return internal->nextChild();
}
const XMLObject::Child& XMLObject::Child::nextChild()const
{
	return internal->nextChild();
}

XMLObject::Attribute& XMLObject::Child::firstAttribute()
{
	return internal->firstAttribute();
}
const XMLObject::Attribute& XMLObject::Child::firstAttribute()const
{
	return internal->firstAttribute();
}
XMLObject::Attribute& XMLObject::Child::nextAttribute()
{
	return internal->nextAttribute();
}
const XMLObject::Attribute& XMLObject::Child::nextAttribute()const
{
	return internal->nextAttribute();
}
bool XMLObject::Child::isEmpty() const
{
	return internal->name.length() == 0 && (internal->attributeList.size() == 0 && internal->childList.size() == 0);
}

void XMLObject::Child::clear()
{
	internal->name = "";
	std::list<XMLObject::Child*>::iterator iter;
	for(iter = internal->childList.begin();iter != internal->childList.end();iter ++)
	{
		delete (*iter);
	}
	internal->attributeList.clear();
	internal->childList.clear();
}

XMLObject::Child& XMLObject::Child::operator = (const Child& child)
{
	internal->attributeList = child.internal->attributeList;
	internal->name = child.internal->name;
	internal->value = child.internal->value;
	std::list<XMLObject::Child*>::iterator iter;
	for(iter = internal->childList.begin();iter != internal->childList.end();iter ++)
	{
		delete (*iter);
	}
	internal->childList.clear();

	for(iter = child.internal->childList.begin();iter != child.internal->childList.end();iter ++)
	{
		XMLObject::Child* node = new XMLObject::Child(**iter);
		internal->childList.push_back(node);
	}

	return *this;
}
bool XMLObject::Child::operator == (const Child& child)const
{
	return internal->name == child.internal->name && 
		internal->value == child.internal->value && 
		internal->attributeList == child.internal->attributeList && 
		internal->childList == child.internal->childList;

	return true;
}
XMLObject::Child::operator bool() const
{
	return !isEmpty();
}


}
}
