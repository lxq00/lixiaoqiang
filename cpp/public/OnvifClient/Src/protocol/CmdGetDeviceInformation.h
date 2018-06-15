#ifndef __ONVIFPROTOCOL_DEVICEINFORMATSION_H__
#define __ONVIFPROTOCOL_DEVICEINFORMATSION_H__
#include "CmdObject.h"

class CMDGetDeviceInformation :public CmdObject
{
public:
	CMDGetDeviceInformation()
	{
		action = "http://www.onvif.org/ver10/device/wsdl/GetDeviceInformation";
	}
	virtual ~CMDGetDeviceInformation() {}

	virtual std::string build(const URI& uri)
	{
		stringstream stream;

		stream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
			<< "<s:Envelope " << onvif_xml_ns << ">"
			<< buildHeader(uri)
			<< "<s:Body>"
			<< "<tds:GetDeviceInformation></tds:GetDeviceInformation>"
			<< "</s:Body>"
			<< "</s:Envelope>";

		return stream.str();
	}
	shared_ptr<OnvifClientDefs::Info> devinfo;
	virtual bool parse(XMLN * p_xml)
	{
		devinfo = make_shared<OnvifClientDefs::Info>();

		XMLN * p_res = xml_node_soap_get(p_xml, "tds:GetDeviceInformationResponse");
		if (NULL == p_res)
		{
			return false;
		}

		XMLN * p_manu = xml_node_soap_get(p_res, "tds:Manufacturer");
		if (p_manu && p_manu->data)
		{
			devinfo->Manufacturer = p_manu->data;
		}

		XMLN * p_model = xml_node_soap_get(p_res, "tds:Model");
		if (p_model && p_model->data)
		{
			devinfo->Model = p_model->data;
		}

		XMLN * p_fmv = xml_node_soap_get(p_res, "tds:FirmwareVersion");
		if (p_fmv && p_fmv->data)
		{
			devinfo->FirmwareVersion = p_fmv->data;
		}

		XMLN * p_sn = xml_node_soap_get(p_res, "tds:SerialNumber");
		if (p_sn && p_sn->data)
		{
			devinfo->SerialNumber = p_sn->data;
		}

		XMLN * p_hd = xml_node_soap_get(p_res, "tds:HardwareId");
		if (p_hd && p_hd->data)
		{
			devinfo->HardwareId = p_hd->data;
		}

		return true;
	}
};

#endif //__ONVIFPROTOCOL_H__
