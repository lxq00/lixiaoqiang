#include "XML/XML.h"
#include "XML/tinyxml.h"
#include "XMLTinyxml.h"
namespace Public{
namespace XML{
#ifdef WIN32
#define strcasecmp _stricmp
#endif

XMLObject::Attribute::Attribute(){}
XMLObject::Attribute::Attribute(const Attribute& attri)
{
	name = attri.name;
	value = attri.value;
}
XMLObject::Attribute::~Attribute(){}
bool XMLObject::Attribute::isEmpty()
{
	return name == "" || value.isEmpty();
}

bool XMLObject::Attribute::operator == (const Attribute& val)const
{
	return name == val.name && value == val.value;
}

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
	std::list<XMLObject::Child*>		childList;
	std::string					name;
	std::string 				value;
	int							getChildIndex;
	int							getAttributeIndex;
};
XMLObject::Child::Child(const char* name)
{
	internal = new ChildInternal();
	if(name != NULL)
	{
		internal->name = name;
	}
}
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

void XMLObject::Child::setName(const std::string& name)
{
	internal->name = name;
}
std::string XMLObject::Child::getName()
{
	return internal->name;
}
void XMLObject::Child::setValue(const XMLObject::Value& value)
{
	internal->value = value.toString();
}
XMLObject::Value XMLObject::Child::getValue()
{
	return XMLObject::Value(internal->value);
}

XMLObject::Child& XMLObject::Child::addChild(const Child& child)
{
	XMLObject::Child* node = new XMLObject::Child(child);
	
	internal->childList.push_back(node);

	return *node;
}

XMLObject::Child& XMLObject::Child::getChild(const std::string& name,int index)
{
	int getIndex = 0;
	std::list<XMLObject::Child*>::iterator iter;
	for(iter = internal->childList.begin();iter != internal->childList.end();iter ++)
	{
		if((*iter)->internal->name == name)
		{
			if(getIndex == index)
			{
				return **iter;
			}
			getIndex ++;
		}
	}

	static Child emptyChild;

	return emptyChild;
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

void XMLObject::Child::setAttribute(const std::string& key,const Value& val)
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
XMLObject::Value& XMLObject::Child::getAttribute(const std::string& key)
{
	std::list<XMLObject::Attribute>::iterator iter;
	for(iter = internal->attributeList.begin();iter != internal->attributeList.end();iter ++)
	{
		if(iter->name == key)
		{
			return iter->value;
		}
	}

	static XMLObject::Value emptyAttri;

	return emptyAttri;
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

XMLObject::Child& XMLObject::Child::firstChild()
{
	static Child emptyChild;
	
	if(internal->childList.size() <= 0)
	{
		return emptyChild;
	}
	
	internal->getChildIndex = 0;
	std::list<XMLObject::Child*>::iterator iter = internal->childList.begin();

	return **iter;
}
XMLObject::Child& XMLObject::Child::nextChild()
{
	static Child emptyChild;
	int getIndex = 0;

	std::list<XMLObject::Child*>::iterator iter;
	for(iter = internal->childList.begin();iter != internal->childList.end();iter ++)
	{
		if(getIndex == internal->getChildIndex)
		{
			iter ++;
			if(iter != internal->childList.end())
			{
				internal->getChildIndex ++;
				return **iter;
			}
			else
			{
				return emptyChild;
			}
		}
		getIndex ++;
	}

	return emptyChild;
}

XMLObject::Attribute& XMLObject::Child::firstAttribute()
{
	static XMLObject::Attribute empltyAttribute;
	if(internal->attributeList.size() <= 0)
	{
		return empltyAttribute;
	}

	std::list<XMLObject::Attribute>::iterator iter = internal->attributeList.begin();
	internal->getAttributeIndex = 0;

	return *iter;
}
XMLObject::Attribute& XMLObject::Child::nextAttribute()
{
	static XMLObject::Attribute empltyAttribute;
	int getindex = 0;
	std::list<XMLObject::Attribute>::iterator iter;
	for(iter = internal->attributeList.begin();iter != internal->attributeList.end();iter ++)
	{
		if(getindex == internal->getAttributeIndex)
		{
			iter ++;
			if(iter != internal->attributeList.end())
			{
				internal->getAttributeIndex ++;

				return *iter;
			}
			else
			{
				return empltyAttribute;
			}
		}
		getindex ++;
	}
;
	return empltyAttribute;
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

struct XMLObject::XMLInternal
{
	XMLInternal():encode(Encoding_Unknown),version("1.0"){}

	Child		root;
	Encoding	encode;
	std::string	version;
};

XMLObject::XMLObject()
{
	internal = new XMLInternal();
}
XMLObject::~XMLObject()
{
	delete internal;
}
XMLObject::Child& XMLObject::setRoot(const Child& root)
{
	internal->root = root;

	return internal->root;
}
XMLObject::Child& XMLObject::getRoot()
{
	return internal->root;
}

std::string XMLObject::getRootName() const
{
	return internal->root.internal->name;
}
void XMLObject::setRootName(const std::string& name)
{
	internal->root.internal->name = name;
}
bool XMLObject::isEmpty() const
{
	return internal->root.isEmpty();
}
void XMLObject::clear()
{
	XMLObject::Child root;

	internal->root = root;
	internal->version = "1.0";
	internal->encode = Encoding_Unknown;
}
bool XMLObject::parseFile(const std::string& file)
{
	clear();

	FILE* fd = fopen(file.c_str(),"rb");
	if(fd == NULL)
	{
		return false;
	}
	fseek(fd,0,SEEK_END);
	int totalsize = ftell(fd);
	fseek(fd,0,SEEK_SET);

	char* buffer = new char[totalsize + 100];
	int readlen = fread(buffer,1,totalsize,fd);
	buffer[readlen] = 0;
	fclose(fd);

	bool ret = parseBuffer(buffer);

	delete []buffer;

	return ret;
}

bool XMLObject::parseBuffer(const std::string& buf)
{
	clear();

	TiXmlDocument xml;
	xml.Parse(buf.c_str());
	if (xml.Error())
	{
		return false;
	}
	bool isParseElment = false;
	TiXmlNode* pNode = xml.FirstChild();
	while(pNode != NULL)
	{
		if(pNode->Type() == TiXmlNode::DECLARATION)
		{
			TiXmlDeclaration* declaration = pNode->ToDeclaration();
			const char* version = declaration->Version();
			const char* encoding = declaration->Encoding();
			if(version != NULL)
			{
				internal->version = version;
			}
			if(encoding != NULL)
			{
				if(strcasecmp(encoding,"gb2312") == 0 || strcasecmp(encoding,"gbk") == 0)
				{
					internal->encode = XMLObject::Encoding_GBK;
				}
				else if(strcasecmp(encoding,"utf-8") == 0)
				{
					internal->encode = XMLObject::Encoding_UTF8;
				}
			}
		}
		else if(!isParseElment)
		{
			internal->root.setName(pNode->Value());
			parseTiXmlElementAttribute(xml.FirstChildElement(),internal->root);
			parseTiXmlElementAndAddChild(pNode,internal->root);

			isParseElment  = true;
		}	
		pNode = pNode->NextSibling();
	}

	return true;
}
std::string XMLObject::toString(Encoding encode)
{
	if(isEmpty())
	{
		return "";
	}
	TiXmlDocument *doc = new TiXmlDocument;

	TiXmlDeclaration* declaration = new TiXmlDeclaration(internal->version.c_str(),internal->encode == Encoding_UTF8 ? "UTF-8" : "gb2312","");
	doc->LinkEndChild(declaration);

	TiXmlElement* root = new TiXmlElement(buildVaildXmlString(internal->root.getName(),internal->encode,encode).c_str());

	buildTiXmlElementFormChild(internal->root,root,internal->encode,encode);
	doc->LinkEndChild(root);

	TiXmlPrinter printer;
	doc->Accept(&printer);

	delete doc;

	return printer.CStr();
}
bool XMLObject::saveAs(const std::string& file,Encoding encode)
{
	FILE* fd = fopen(file.c_str(),"wb+");
	if(fd == NULL)
	{
		return false;
	}

	std::string str = toString(encode);

	fwrite(str.c_str(),1,str.length(),fd);

	fclose(fd);

	return true;
}


}
}
