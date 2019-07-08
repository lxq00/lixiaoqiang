#ifndef __ONVIFPROTOCOLCAPABITIES_H__
#define __ONVIFPROTOCOLCAPABITIES_H__
#include "CmdObject.h"


class CMDGetCapabilities :public CmdObject
{
public:
	enum CapabilitiesType
	{
		CAP_ALL = 0,
		CAP_MEDIA = 1,
		CAP_DEVICE = 2,
		CAP_ANALYTICS = 3,
		CAP_EVENTS = 4,
		CAP_IMAGING = 5,
		CAP_PTZ = 6,
	};
public:
	CMDGetCapabilities(CapabilitiesType _cap = CAP_ALL) :cap(_cap)
	{
		action = "http://www.onvif.org/ver10/device/wsdl/GetCapabilities";
	}
	virtual ~CMDGetCapabilities() {}

	virtual std::string build(const URL& URL)
	{
		stringstream stream;

		stream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
			<< "<s:Envelope xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\">"
			<< buildHeader(URL)
			<< "<s:Body xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\">"
			<< "<GetCapabilities xmlns=\"http://www.onvif.org/ver10/device/wsdl\"> <Category>"
			<< get_cap_str()
			<< "</Category></GetCapabilities>"
			<< "</s:Body></s:Envelope>";

		return stream.str();
	}
	shared_ptr<OnvifClientDefs::Capabilities> capabilities;
	virtual bool parse(const XMLObject::Child& p_xml)
	{
		capabilities = make_shared<OnvifClientDefs::Capabilities>();

		const XMLObject::Child& resp = p_xml.getChild("tds:GetCapabilitiesResponse");
		if (!resp) return false;

		const XMLObject::Child& cap = resp.getChild("tds:Capabilities");
		if (!cap) return false;

		const XMLObject::Child& media = cap.getChild("tt:Media");
		if(media) capabilities->Media.Support = parseMedia(media);
		
		const XMLObject::Child& ptz = cap.getChild("tt:PTZ");
		if(ptz) capabilities->PTZ.Support = parsePtz(ptz);
		
		const XMLObject::Child& events = cap.getChild("tt:Events");
		if (events) capabilities->Events.Support = parseEvents(events);

		return true;
	}
private:
	virtual bool parseMedia(const XMLObject::Child& body)
	{
		const XMLObject::Child& xaddr = body.getChild("tt:XAddr");
		if (xaddr)
		{
			capabilities->Media.xaddr = xaddr.data();
		}
		else
		{
			return FALSE;
		}

		const XMLObject::Child& cap = body.getChild("tt:StreamingCapabilities");
		if (!cap) return FALSE;
		
		const XMLObject::Child& rtpmult = cap.getChild("tt:RTPMulticast");
		if (rtpmult)
		{
			capabilities->Media.RTPMulticast = rtpmult.data().readBool();
		}
		const XMLObject::Child& rtptcp = cap.getChild("tt:RTP_TCP");
		if (rtptcp)
		{
			capabilities->Media.RTP_TCP = rtptcp.data().readBool();
		}
		const XMLObject::Child& rtsp = cap.getChild("tt:RTP_RTSP_TCP");
		if (rtsp)
		{
			capabilities->Media.RTP_RTSP_TCP = rtsp.data().readBool();
		}

		return true;
	}


	bool parsePtz(const XMLObject::Child& ptz)
	{
		const XMLObject::Child& xaddr = ptz.getChild("tt:XAddr");
		if (!xaddr) return false;

		capabilities->PTZ.xaddr = xaddr.data();

		return true;
	}

	bool parseEvents(const XMLObject::Child& p_events)
	{
		const XMLObject::Child& xaddr = p_events.getChild("tt:XAddr");
		if (!xaddr) return false;

		capabilities->Events.xaddr = xaddr.data();

		return true;
	}
private:
	std::string get_cap_str()
	{
		switch (cap)
		{
		case CAP_ALL:
			return "All";
			break;

		case CAP_ANALYTICS:
			return "Analytics";
			break;

		case CAP_DEVICE:
			return "Device";
			break;

		case CAP_EVENTS:
			return "Events";
			break;

		case CAP_IMAGING:
			return "Imaging";
			break;

		case CAP_MEDIA:
			return "Media";
			break;

		case CAP_PTZ:
			return "PTZ";
			break;
		}

		return "";
	}
private:
	CapabilitiesType cap;
};


#endif //__ONVIFPROTOCOL_H__
