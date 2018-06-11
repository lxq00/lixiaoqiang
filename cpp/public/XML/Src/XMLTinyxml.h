#ifndef __XMLTINYXML_H__
#define __XMLTINYXML_H__

#include "XML/XML.h"
#include "XML/tinyxml.h"
namespace Xunmei{
namespace XML{


void findVersionAndEncoding(const std::string& xml,std::string& ver,XM_XML::Encoding& encode);
void buildTiXmlElementFormChild(XM_XML::Child& child,TiXmlElement* pElement,XM_XML::Encoding old,XM_XML::Encoding encode);
void parseTiXmlElementAttribute(TiXmlElement* pElement,XM_XML::Child& child);
void parseTiXmlElementAndAddChild(TiXmlNode* pElement,XM_XML::Child& child);
std::string buildVaildXmlString(const std::string& val,XM_XML::Encoding old,XM_XML::Encoding encode);

}
}

#endif //__XMLTINYXML_H__
