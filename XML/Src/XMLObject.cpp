#include "XML/XML.h"
#include "XML/tinyxml.h"
#include "XMLTinyxml.h"
namespace Public{
namespace XML{
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
const XMLObject::Child& XMLObject::getRoot()const
{
	return internal->root;
}
std::string XMLObject::getRootName() const
{
	return internal->root.name();
}
void XMLObject::setRootName(const std::string& name)
{
	internal->root.name(name);
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
			internal->root.name(pNode->Value());
			parseTiXmlElementAttribute(xml.FirstChildElement(),internal->root);
			parseTiXmlElementAndAddChild(pNode,internal->root);

			isParseElment  = true;
		}	
		pNode = pNode->NextSibling();
	}

	return true;
}
std::string XMLObject::toString(Encoding encode) const
{
	if (isEmpty())
	{
		return "";
	}
	TiXmlDocument *doc = new TiXmlDocument;

	TiXmlDeclaration* declaration = new TiXmlDeclaration(internal->version.c_str(), internal->encode == Encoding_UTF8 ? "UTF-8" : "gb2312", "");
	doc->LinkEndChild(declaration);

	TiXmlElement* root = new TiXmlElement(buildVaildXmlString(internal->root.name(), internal->encode, encode).c_str());

	buildTiXmlElementFormChild(internal->root, root, internal->encode, encode);
	doc->LinkEndChild(root);

	TiXmlPrinter printer;
	doc->Accept(&printer);

	delete doc;

	return printer.CStr();
}
bool XMLObject::saveAs(const std::string& file,Encoding encode)const
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
