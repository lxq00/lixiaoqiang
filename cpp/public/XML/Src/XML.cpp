#include "XML/XML.h"
#include "XML/tinyxml.h"
#include "XMLTinyxml.h"
namespace Xunmei{
namespace XML{
#ifdef WIN32
#define strcasecmp _stricmp
#endif

XM_XML::Attribute::Attribute(){}
XM_XML::Attribute::Attribute(const Attribute& attri)
{
	name = attri.name;
	value = attri.value;
}
XM_XML::Attribute::~Attribute(){}
bool XM_XML::Attribute::isEmpty()
{
	return name == "" || value.isEmpty();
}

bool XM_XML::Attribute::operator == (const Attribute& val)const
{
	return name == val.name && value == val.value;
}

struct XM_XML::Child::ChildInternal
{
	ChildInternal():getChildIndex(0),getAttributeIndex(0){}
	~ChildInternal()
	{
		std::list<XM_XML::Child*>::iterator iter;
		for(iter = childList.begin();iter != childList.end();iter ++)
		{
			delete (*iter);
		}
	}

	std::list<XM_XML::Attribute>	attributeList;
	std::list<XM_XML::Child*>		childList;
	std::string					name;
	std::string 				value;
	int							getChildIndex;
	int							getAttributeIndex;
};
XM_XML::Child::Child(const char* name)
{
	internal = new ChildInternal();
	if(name != NULL)
	{
		internal->name = name;
	}
}
XM_XML::Child::Child(const std::string& name)
{
	internal = new ChildInternal;
	internal->name = name;
}
XM_XML::Child::Child(const Child& child)
{
	internal = new ChildInternal;
	internal->name = child.internal->name;
	internal->attributeList = child.internal->attributeList;
	internal->value = child.internal->value;
	
	std::list<XM_XML::Child*>::iterator iter;
	for(iter = child.internal->childList.begin();iter != child.internal->childList.end();iter ++)
	{
		XM_XML::Child* node = new XM_XML::Child(**iter);
		internal->childList.push_back(node);
	}
}
XM_XML::Child::~Child()
{
	delete internal;
}

void XM_XML::Child::setName(const std::string& name)
{
	internal->name = name;
}
std::string XM_XML::Child::getName()
{
	return internal->name;
}
void XM_XML::Child::setValue(const XM_XML::Value& value)
{
	internal->value = value.toString();
}
XM_XML::Value XM_XML::Child::getValue()
{
	return XM_XML::Value(internal->value);
}

XM_XML::Child& XM_XML::Child::addChild(const Child& child)
{
	XM_XML::Child* node = new XM_XML::Child(child);
	
	internal->childList.push_back(node);

	return *node;
}

XM_XML::Child& XM_XML::Child::getChild(const std::string& name,int index)
{
	int getIndex = 0;
	std::list<XM_XML::Child*>::iterator iter;
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

void XM_XML::Child::removeChild(const std::string& name,int index)
{
	int getIndex = 0;
	std::list<XM_XML::Child*>::iterator iter;
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
int XM_XML::Child::childCount() const
{
	return internal->childList.size();
}

int XM_XML::Child::attributeCount() const
{
	return internal->attributeList.size();
}

void XM_XML::Child::setAttribute(const std::string& key,const Value& val)
{
	std::list<XM_XML::Attribute>::iterator iter;
	for(iter = internal->attributeList.begin();iter != internal->attributeList.end();iter ++)
	{
		if(iter->name == key)
		{
			iter->value = val;
			return;
		}
	}

	XM_XML::Attribute attri;
	attri.name = key;
	attri.value = val;

	internal->attributeList.push_back(attri);
}
XM_XML::Value& XM_XML::Child::getAttribute(const std::string& key)
{
	std::list<XM_XML::Attribute>::iterator iter;
	for(iter = internal->attributeList.begin();iter != internal->attributeList.end();iter ++)
	{
		if(iter->name == key)
		{
			return iter->value;
		}
	}

	static XM_XML::Value emptyAttri;

	return emptyAttri;
}

void XM_XML::Child::removeAttribute(const std::string& key)
{
	std::list<XM_XML::Attribute>::iterator iter;
	for(iter = internal->attributeList.begin();iter != internal->attributeList.end();iter ++)
	{
		if(iter->name == key)
		{
			internal->attributeList.erase(iter);
			break;
		}
	}
}

XM_XML::Child& XM_XML::Child::firstChild()
{
	static Child emptyChild;
	
	if(internal->childList.size() <= 0)
	{
		return emptyChild;
	}
	
	internal->getChildIndex = 0;
	std::list<XM_XML::Child*>::iterator iter = internal->childList.begin();

	return **iter;
}
XM_XML::Child& XM_XML::Child::nextChild()
{
	static Child emptyChild;
	int getIndex = 0;

	std::list<XM_XML::Child*>::iterator iter;
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

XM_XML::Attribute& XM_XML::Child::firstAttribute()
{
	static XM_XML::Attribute empltyAttribute;
	if(internal->attributeList.size() <= 0)
	{
		return empltyAttribute;
	}

	std::list<XM_XML::Attribute>::iterator iter = internal->attributeList.begin();
	internal->getAttributeIndex = 0;

	return *iter;
}
XM_XML::Attribute& XM_XML::Child::nextAttribute()
{
	static XM_XML::Attribute empltyAttribute;
	int getindex = 0;
	std::list<XM_XML::Attribute>::iterator iter;
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

bool XM_XML::Child::isEmpty() const
{
	return internal->name.length() == 0 && (internal->attributeList.size() == 0 && internal->childList.size() == 0);
}

void XM_XML::Child::clear()
{
	internal->name = "";
	std::list<XM_XML::Child*>::iterator iter;
	for(iter = internal->childList.begin();iter != internal->childList.end();iter ++)
	{
		delete (*iter);
	}
	internal->attributeList.clear();
	internal->childList.clear();
}

XM_XML::Child& XM_XML::Child::operator = (const Child& child)
{
	internal->attributeList = child.internal->attributeList;
	internal->name = child.internal->name;
	internal->value = child.internal->value;
	std::list<XM_XML::Child*>::iterator iter;
	for(iter = internal->childList.begin();iter != internal->childList.end();iter ++)
	{
		delete (*iter);
	}
	internal->childList.clear();

	for(iter = child.internal->childList.begin();iter != child.internal->childList.end();iter ++)
	{
		XM_XML::Child* node = new XM_XML::Child(**iter);
		internal->childList.push_back(node);
	}

	return *this;
}
bool XM_XML::Child::operator == (const Child& child)const
{
	return internal->name == child.internal->name && 
		internal->value == child.internal->value && 
		internal->attributeList == child.internal->attributeList && 
		internal->childList == child.internal->childList;

	return true;
}

struct XM_XML::XMLInternal
{
	XMLInternal():encode(Encoding_Unknown),version("1.0"){}

	Child		root;
	Encoding	encode;
	std::string	version;
};

XM_XML::XM_XML()
{
	internal = new XMLInternal();
}
XM_XML::~XM_XML()
{
	delete internal;
}
XM_XML::Child& XM_XML::setRoot(const Child& root)
{
	internal->root = root;

	return internal->root;
}
XM_XML::Child& XM_XML::getRoot()
{
	return internal->root;
}

std::string XM_XML::getRootName() const
{
	return internal->root.internal->name;
}
void XM_XML::setRootName(const std::string& name)
{
	internal->root.internal->name = name;
}
bool XM_XML::isEmpty() const
{
	return internal->root.isEmpty();
}
void XM_XML::clear()
{
	XM_XML::Child root;

	internal->root = root;
	internal->version = "1.0";
	internal->encode = Encoding_Unknown;
}
bool XM_XML::parseFile(const std::string& file)
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

bool XM_XML::parseBuffer(const std::string& buf)
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
					internal->encode = XM_XML::Encoding_GBK;
				}
				else if(strcasecmp(encoding,"utf-8") == 0)
				{
					internal->encode = XM_XML::Encoding_UTF8;
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
std::string XM_XML::toString(Encoding encode)
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
bool XM_XML::saveAs(const std::string& file,Encoding encode)
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
