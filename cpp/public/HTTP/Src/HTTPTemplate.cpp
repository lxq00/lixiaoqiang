#include "ctemplate/template.h"
#include "HTTP/HTTPServer.h"
using namespace ctemplate;

namespace Public {
namespace HTTP {


struct HTTPTemplate::HTTPTemplateInternal
{
	HTTPTemplateInternal(const std::string& _filename):dict(_filename),filename(_filename){}

	ctemplate::TemplateDictionary dict;
	std::string					  filename;
};

struct HTTPTemplateDirtionary:public HTTPTemplate::TemplateDirtionary
{
	ctemplate::TemplateDictionary* tmpitem;

	TemplateDirtionary* setValue(const std::string& key,const URI::Value&  value)
	{
		if(tmpitem != NULL)
		{
			tmpitem->SetValue(key,value.readString()); 
		}

		return this;
	}
};

HTTPTemplate::HTTPTemplate(const std::string& tmpfilename)
{
	internal = new HTTPTemplateInternal(tmpfilename);
}
HTTPTemplate::~HTTPTemplate()
{
	SAFE_DELETE(internal);
}
//更换变量到值
HTTPTemplate& HTTPTemplate::setValue(const std::string& key,const URI::Value&  value)
{
	internal->dict.SetValue(key,value.readString());

	return *this;
}
//循环更换变量，循环 HTTPTemplate
HTTPTemplate& HTTPTemplate::setValue(const std::string& key,const std::vector<TemplateObject*>&  valuelist)
{
	for(unsigned int i = 0;i < valuelist.size();i++)
	{
		std::map<std::string,URI::Value>	valuemap;
		if(valuelist[i] != NULL && valuelist[i]->toTemplateData(valuemap))
		{
			shared_ptr<HTTPTemplate::TemplateDirtionary> tmpl = addSectionDirtinary(key);
			for(std::map<std::string,URI::Value>::iterator iter = valuemap.begin();iter != valuemap.end();iter ++)
			{
				tmpl->setValue(iter->first,iter->second);
			}
		}
	}

	return *this;
}
//添加循环变量
shared_ptr<HTTPTemplate::TemplateDirtionary> HTTPTemplate::addSectionDirtinary(const std::string& starttmpkey)
{
	shared_ptr<HTTPTemplateDirtionary> tmp(new HTTPTemplateDirtionary);
	tmp->tmpitem = internal->dict.AddSectionDictionary(starttmpkey);

	return tmp;
}

std::string HTTPTemplate::toValue() const
{
	std::string outstr;
	
	ctemplate::ExpandTemplate("./_tmp_"+internal->filename,ctemplate::DO_NOT_STRIP,&internal->dict,&outstr);

	return outstr;
}

}
}