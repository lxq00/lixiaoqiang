#ifndef __XMLTINYXML_H__
#define __XMLTINYXML_H__

#include "XML/XML.h"
#include "XML/tinyxml.h"
namespace Public{
namespace XML{


void findVersionAndEncoding(const std::string& xml,std::string& ver,XMLObject::Encoding& encode);
void buildTiXmlElementFormChild(XMLObject::Child& child,TiXmlElement* pElement,XMLObject::Encoding old,XMLObject::Encoding encode);
void parseTiXmlElementAttribute(TiXmlElement* pElement,XMLObject::Child& child);
void parseTiXmlElementAndAddChild(TiXmlNode* pElement,XMLObject::Child& child);
std::string buildVaildXmlString(const std::string& val,XMLObject::Encoding old,XMLObject::Encoding encode);

}
}

#endif //__XMLTINYXML_H__
