#ifndef __ONVIFPROTOCOLOBJECT_H__
#define __ONVIFPROTOCOLOBJECT_H__
#include "Base/Base.h"
#include <sstream>
#include "OnvifClient/OnvifClientDefs.h"
#include "../XMLNDocment.h"

using namespace Public::Base;
using namespace Public::Onvif;


#define onvif_xml_header "<s:Header>"\
"<Security s:mustUnderstand=\"1\" xmlns=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd\">"\
"<UsernameToken>"\
"<Username>%s</Username>"\
"<Password Type=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-username-token-profile-1.0#PasswordDigest\">%s</Password>"\
"<Nonce EncodingType=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-soap-message-security-1.0#Base64Binary\">%s</Nonce>"\
"<Created xmlns=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-utility-1.0.xsd\">%s</Created>"\
"</UsernameToken>"\
"</Security>"\
"</s:Header>"

#define onvif_xml_headerCreate "<s:Header>"\
"<a:Action s:mustUnderstand=\"1\">http://www.onvif.org/ver10/events/wsdl/PullPointSubscription/PullMessagesRequest</a:Action>"\
"<a:MessageID>urn:uuid:%s</a:MessageID>"\
"<a:ReplyTo>"\
"<a:Address>http://www.w3.org/2005/08/addressing/anonymous</a:Address>"\
"</a:ReplyTo>"\
"<SecURLty s:mustUnderstand=\"1\" xmlns=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecURLty-secext-1.0.xsd\">"\
"<UsernameToken>"\
"<Username>%s</Username>"\
"<Password Type=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-username-token-profile-1.0#PasswordDigest\">%s</Password>"\
"<Nonce>%s</Nonce>"\
"<Created xmlns=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecURLty-utility-1.0.xsd\">%s</Created>"\
"</UsernameToken></SecURLty>"\
"<a:To s:mustUnderstand=\"1\">http://%s%s</a:To>"\
"</s:Header>"

class CmdObject
{
public:
	const char*  recvbuffer;
	std::string  action;
public:
	CmdObject():recvbuffer(NULL){}
	virtual ~CmdObject() {}

	virtual std::string build(const URL& URL) = 0;
	virtual bool parse(XMLN * p_xml) = 0;
protected:
	std::string  buildHeader(const URL& URL)
	{
		std::string noneBase64, passwdBase64, createTime;

		buildUserAuthenInfo(noneBase64, passwdBase64, createTime, URL.authen.Password);

		char buffer[1024] = { 0 };

		snprintf_x(buffer, 1023, onvif_xml_header, URL.authen.Username.c_str(), passwdBase64.c_str(), noneBase64.c_str(), createTime.c_str());

		return buffer;
	}

	std::string buildCreateHeader(const URL& URL)
	{
		std::string noneBase64, passwdBase64, createTime;

		buildUserAuthenInfo(noneBase64, passwdBase64, createTime, URL.authen.Password);

		std::string guidstr = Guid::createGuid().getStringStream();

		char buffer[1024] = { 0 };

		snprintf_x(buffer, 1023, onvif_xml_headerCreate,
			guidstr.c_str(), URL.authen.Username.c_str(), passwdBase64.c_str(), noneBase64.c_str(), createTime.c_str(), URL.getHost().c_str(), URL.getPath().c_str());

		return buffer;
	}

	void buildUserAuthenInfo(std::string& noneBase64, std::string & passwdBase64, std::string& createTime, const std::string& password)
	{
		std::string noneString = onvif_calc_nonce();
		noneBase64 = Base64::encode(noneString);


		createTime = onvif_datetime();

		std::string basetmp = onvif_calc_digest(createTime, noneString, password);
		passwdBase64 = Base64::encode(basetmp);
	}
private:
	static std::string onvif_calc_nonce()
	{
		static unsigned short count = 0xCA53;
		static Mutex count_Mutex;

		Guard locker(count_Mutex);

		char buf[32] = { 0 };
		/* we could have used raw binary instead of hex as below */
		snprintf_x(buf, 31, "%8.8x%4.4hx%8.8x", (int)Time::getCurrentTime().makeTime(), count++, (int)rand());


		return buf;
	}

	static std::string onvif_datetime()
	{
		char buf[32] = { 0 };

		Time nowtime = Time::getCurrentTime();
		snprintf_x(buf, 32, "%04d-%02d-%02dT%02d:%02d:%02dZ", nowtime.year, nowtime.month, nowtime.day, nowtime.hour, nowtime.minute, nowtime.second);

		return buf;
	}

	static std::string onvif_calc_digest(const std::string& created, const std::string& nonce, const std::string& password)
	{
		Sha1 sha1;

		sha1.input(nonce.c_str(), nonce.length());
		sha1.input(created.c_str(), created.length());
		sha1.input(password.c_str(), password.length());

		return sha1.report(Sha1::REPORT_BIN);
	}
public:
	
	static int onvif_parse_xaddr(const char * pdata, OnvifClientDefs::XAddr& p_xaddr)
	{
		const char *p2;
		int len = strlen(pdata);

		p_xaddr.port = 80;

		if (len > 7) // skip "http://"
		{
			p2 = strchr(pdata + 7, '/');
			if (p2)
			{
				char szHost[24] = { 0 };
				strncpy(szHost, pdata + 7, (p2 - pdata - 7));
				char *p3 = strchr(szHost, ':');
				if (p3)
				{
					p_xaddr.host = std::string(szHost, p3 - szHost);
					p_xaddr.port = atoi(p3 + 1);
				}
				else
				{
					p_xaddr.host = std::string(szHost, p2 - pdata - 7);
				}
				p_xaddr.url = std::string(p2, len - (p2 - pdata));
			}
			else
			{
				len = len - 7;
				p_xaddr.host = std::string(pdata + 7,len);
			}
		}

		return 1;
	}

	static bool onvif_parse_bool(const char * pdata)
	{
		if (NULL == pdata)
		{
			return false;
		}

		if (strcasecmp(pdata, "true") == 0)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	

	static OnvifClientDefs::VIDEO_ENCODING onvif_parse_encoding(const char * pdata)
	{
		if (strcmp(pdata, "H264") == 0)
		{
			return OnvifClientDefs::VIDEO_ENCODING_H264;
		}
		else if (strcmp(pdata, "JPEG") == 0)
		{
			return OnvifClientDefs::VIDEO_ENCODING_JPEG;
		}
		else if (strcmp(pdata, "MPEG4") == 0)
		{
			return OnvifClientDefs::VIDEO_ENCODING_MPEG4;
		}
		return OnvifClientDefs::VIDEO_ENCODING_UNKNOWN;
	}

	OnvifClientDefs::H264_PROFILE onvif_parse_h264_profile(const char * pdata)
	{
		if (strcmp(pdata, "Baseline") == 0)
		{
			return OnvifClientDefs::H264_PROFILE_Baseline;
		}
		else if (strcmp(pdata, "High") == 0)
		{
			return OnvifClientDefs::H264_PROFILE_High;
		}
		else if (strcmp(pdata, "Main") == 0)
		{
			return OnvifClientDefs::H264_PROFILE_Main;
		}
		else if (strcmp(pdata, "Extended") == 0)
		{
			return OnvifClientDefs::H264_PROFILE_Extended;
		}
		return OnvifClientDefs::H264_PROFILE_Baseline;
	}

	static int onvif_parse_time(const char * pdata)
	{
		return atoi(pdata + 2);
	}
private:

};


#define  onvif_xml_ns \
"xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\" "\
"xmlns:enc=\"http://www.w3.org/2003/05/soap-encoding\" "\
"xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" "\
"xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" "\
"xmlns:wsa5=\"http://www.w3.org/2005/08/addressing\" "\
"xmlns:wsu=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecURLty-utility-1.0.xsd\" "\
"xmlns:xenc=\"http://www.w3.org/2001/04/xmlenc#\" "\
"xmlns:ds=\"http://www.w3.org/2000/09/xmldsig#\" "\
"xmlns:wsse=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecURLty-secext-1.0.xsd\" "\
"xmlns:xmime=\"http://tempURL.org/xmime.xsd\" "\
"xmlns:tt=\"http://www.onvif.org/ver10/schema\" "\
"xmlns:tds=\"http://www.onvif.org/ver10/device/wsdl\" "\
"xmlns:trt=\"http://www.onvif.org/ver10/media/wsdl\" "\
"xmlns:ter=\"http://www.onvif.org/ver10/error\""


#endif //__ONVIFPROTOCOL_H__
