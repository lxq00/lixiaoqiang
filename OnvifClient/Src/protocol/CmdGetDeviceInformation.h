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

	virtual std::string build(const URL& URL)
	{
		stringstream stream;

		stream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
			<< "<s:Envelope " << onvif_xml_ns << ">"
			<< buildHeader(URL)
			<< "<s:Body xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\">"
			<< "<GetDeviceInformation></GetDeviceInformation>"
			<< "</s:Body>"
			<< "</s:Envelope>";

		return stream.str();
	}
	shared_ptr<OnvifClientDefs::Info> devinfo;

	virtual bool parse(const XMLObject::Child& body)
	{
		devinfo = make_shared<OnvifClientDefs::Info>();

		const XMLObject::Child& response = body.getChild("tds:GetDeviceInformationResponse");
		if (response.isEmpty()) return false;

		const XMLObject::Child& manu = response.getChild("tds:Manufacturer");
		if (!manu.isEmpty()) devinfo->Manufacturer = manu.data().readString();

		const XMLObject::Child& model = response.getChild("tds:Model");
		if (!model.isEmpty()) devinfo->Model = model.data().readString();

		const XMLObject::Child& fireware = response.getChild("tds:FirmwareVersion");
		if (!fireware.isEmpty()) devinfo->FirmwareVersion = fireware.data().readString();

		const XMLObject::Child& sn = response.getChild("tds:SerialNumber");
		if (!sn.isEmpty()) devinfo->SerialNumber = sn.data().readString();

		const XMLObject::Child& hardware = response.getChild("tds:HardwareId");
		if (!hardware.isEmpty()) devinfo->HardwareId = hardware.data().readString();

		return true;
	}
};

#endif //__ONVIFPROTOCOL_H__
