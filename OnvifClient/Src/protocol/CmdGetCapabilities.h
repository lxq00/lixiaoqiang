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
	virtual bool parse(XMLN * p_xml)
	{
		capabilities = make_shared<OnvifClientDefs::Capabilities>();

		XMLN * p_res = xml_node_soap_get(p_xml, "tds:GetCapabilitiesResponse");
		if (NULL == p_res)
		{
			return false;
		}

		XMLN * p_cap = xml_node_soap_get(p_res, "tds:Capabilities");
		if (NULL == p_cap)
		{
			return false;
		}

		XMLN * p_media = xml_node_soap_get(p_cap, "tt:Media");
		if (p_media)
		{
			capabilities->Media.Support = parseMedia(p_media);
		}

		XMLN * p_ptz = xml_node_soap_get(p_cap, "tt:PTZ");
		if (p_ptz)
		{
			capabilities->PTZ.Support = parsePtz(p_ptz);
		}

		XMLN* p_events = xml_node_soap_get(p_cap, "tt:Events");
		if (p_events)
		{
			capabilities->Events.Support = parseEvents(p_events);
		}

		return true;
	}
private:
	virtual bool parseMedia(XMLN * p_media)
	{
		XMLN * p_xaddr = xml_node_soap_get(p_media, "tt:XAddr");
		if (p_xaddr && p_xaddr->data)
		{
			onvif_parse_xaddr(p_xaddr->data, capabilities->Media.xaddr);
		}
		else
		{
			return FALSE;
		}

		XMLN * p_cap = xml_node_soap_get(p_media, "tt:StreamingCapabilities");
		if (NULL == p_cap)
		{
			return FALSE;
		}

		XMLN * p_mc = xml_node_soap_get(p_cap, "tt:RTPMulticast");
		if (p_mc && p_mc->data)
		{
			capabilities->Media.RTPMulticast = onvif_parse_bool(p_mc->data);
		}

		XMLN * p_rtp_tcp = xml_node_soap_get(p_cap, "tt:RTP_TCP");
		if (p_rtp_tcp && p_rtp_tcp->data)
		{
			capabilities->Media.RTP_TCP = onvif_parse_bool(p_rtp_tcp->data);;
		}

		XMLN * p_rtp_rtsp_tcp = xml_node_soap_get(p_cap, "RTP_RTSP_TCP");
		if (p_rtp_rtsp_tcp && p_rtp_rtsp_tcp->data)
		{
			capabilities->Media.RTP_RTSP_TCP = onvif_parse_bool(p_rtp_rtsp_tcp->data);;
		}

		return true;
	}


	bool parsePtz(XMLN * p_ptz)
	{
		XMLN * p_xaddr = xml_node_soap_get(p_ptz, "tt:XAddr");
		if (p_xaddr && p_xaddr->data)
		{
			onvif_parse_xaddr(p_xaddr->data, capabilities->PTZ.xaddr);
		}
		else
		{
			return false;
		}

		return true;
	}

	bool parseEvents(XMLN * p_events)
	{
		XMLN * p_xaddr = xml_node_soap_get(p_events, "tt:XAddr");
		if (p_xaddr && p_xaddr->data)
		{
			onvif_parse_xaddr(p_xaddr->data, capabilities->Events.xaddr);
		}
		else
		{
			return false;
		}

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
