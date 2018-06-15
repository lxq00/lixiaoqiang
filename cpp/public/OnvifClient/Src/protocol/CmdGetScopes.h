#ifndef __ONVIFPROTOCOL_PROFILES_H__GetScopes
#define __ONVIFPROTOCOL_PROFILES_H__GetScopes
#include "CmdObject.h"


class CMDGetScopes :public CmdObject
{
public:
	CMDGetScopes() {}
	virtual ~CMDGetScopes() {}

	virtual std::string build(const URI& uri)
	{
		stringstream stream;

		stream << "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
			<< "<s:Envelope "<< onvif_xml_ns <<">"
			<< "<s:Body xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\">"
			<< "<GetScopes xmlns=\"http://www.onvif.org/ver10/device/wsdl\"/>"
			<< "</s:Body></s:Envelope>";


		return stream.str();
	}
	virtual bool parse(XMLN * p_xml)
	{
		XMLN * p_res = xml_node_soap_get(p_xml, "tds:GetScopesResponse");
		if (NULL == p_res)
		{
			return false;
		}
		/*const char *p_res21 = */xml_attr_get(p_xml, "onvif://www.onvif.org/name");


		while (true)
		{
			XMLN * p_cfg = xml_node_soap_get(p_res, "tds:Scopes");
			if (NULL == p_res)
			{
				return false;
			}
			else
			{
				// 			XMLN * p_cfg = xml_node_soap_get(p_res, "tt:ScopeDef");
				// 			if (NULL == p_res)
				// 			{
				// 				return FALSE;
				// 			}
				XMLN * p_cfg1 = xml_node_soap_get(p_cfg, "tt:ScopeItem");
				if (NULL == p_res)
				{
					return false;
				}
				if (p_cfg && p_cfg->data)
				{
					if (0 == strncmp("onvif://www.onvif.org/name", p_cfg->data, 26))
					{
					}
				}
				p_res = p_cfg1;
			}
		}

		return true;
	}
};



#endif //__ONVIFPROTOCOL_H__
