#ifndef __RTSPCMD_H__
#define __RTSPCMD_H__
#include "Base/Base.h"
#include "RTSP/RTSPStructs.h"
using namespace Public::Base;

struct WWW_Authenticate
{
public:
	static bool checkAuthenticate(const std::string& method, const std::string& username, const std::string& password, const std::string& pAuthorization)
	{
		if (NULL == strstr(pAuthorization.c_str(), "Digest"))
		{
			const char* authenstart = strstr(pAuthorization.c_str(), "Basic ");
			if (authenstart == NULL) return false;

			std::string authenresult = pAuthorization.c_str() + strlen("Basic ");

			return buildAuthorization(username, password, true) == authenresult;
		}
		else
		{
			std::string szrealm = getAuthenAttr(pAuthorization, "realm");
			std::string szRtspUrl = getAuthenAttr(pAuthorization, "uri");
			std::string szNonce = getAuthenAttr(pAuthorization, "nonce");
			std::string szResponse = getAuthenAttr(pAuthorization, "response");

			std::string authen3 = buildAuthorization(username, password, false, method, szRtspUrl, szrealm, szNonce);

			return szResponse == authen3;
		}
	}
	static std::string buildWWWAuthenticate(const std::string& method, const std::string& username, const std::string& password)
	{
		std::string timestr = Value(Time::getCurrentMilliSecond()).readString();
		std::string realmstr,noncestr;
		{	
			Sha1 sha1;
			sha1.input(method.c_str(), method.length());
			sha1.input(timestr.c_str(), timestr.length());

			realmstr = sha1.report(Sha1::REPORT_HEX_SHORT);
		}
		{
			Sha1 sha1;
			sha1.input(username.c_str(), username.length());
			sha1.input(timestr.c_str(), timestr.length());

			noncestr = sha1.report(Sha1::REPORT_HEX_SHORT);
		}

		std::string wwwauthen = std::string("Digest ") + "realm=\"" + realmstr + "\",nonce=\"" + noncestr + "\"";

		return wwwauthen;
	}
	static std::string buildAuthorization(const std::string& method, const std::string& username, const std::string& password,const std::string& url, const std::string& wwwauthen)
	{
		bool needBasic = true;
		std::string m_szRealm, m_szNonce;

		parseAuthenString(wwwauthen, needBasic, m_szRealm, m_szNonce);

		std::string authstring;
		if (needBasic)
		{
			authstring = std::string("Basic ") + buildAuthorization(username,password,needBasic);
		}
		else
		{
			std::string authen3 = buildAuthorization(username,password,needBasic,method,url,m_szRealm,m_szNonce);

			authstring = std::string("Digest username=\"") + username + "\", realm=\"" + m_szRealm + "\", nonce=\"" + m_szNonce + "\", uri=\""
				+ url + "\", response=\"" + authen3 + "\"";
		}

		return std::string("Authorization: ") + authstring + "\r\n";
	}
private:
	static std::string buildAuthorization(const std::string& username, const std::string& password,  bool needBasic, const std::string& method = "", const std::string& url = "", const std::string& m_szRealm = "", const std::string& m_szNonce = "")
	{
		std::string authstring;
		if (needBasic)
		{
			std::string authyenstr = username + ":" + password;

			std::string buffer = Base64::encode(authyenstr);

			authstring = buffer;
		}
		else
		{
			std::string authen1, authen2, authen3;
			{
				std::string strHash1src = username + ":" + m_szRealm + ":" + password;


				Md5 md5;
				md5.init();
				md5.update((uint8_t const*)strHash1src.c_str(), strHash1src.length());
				authen1 = md5.hex();
			}

			{
				std::string strHash2src = method + ":" + url;

				Md5 md5;
				md5.init();
				md5.update((uint8_t const*)strHash2src.c_str(), strHash2src.length());
				authen2 = md5.hex();
			}

			{
				std::string strHash3src = std::string(authen1) + ":" + m_szNonce + ":" + authen2;

				Md5 md5;
				md5.init();
				md5.update((uint8_t const*)strHash3src.c_str(), strHash3src.length());
				authen3 = md5.hex();
			}

			authstring = authen3;
		}

		return authstring;
	}
	static bool parseAuthenString(const std::string& pAuthorization, bool& needBasic, std::string& m_szRealm, std::string& m_szNonce)
	{
		needBasic = true;

		if (NULL != strstr(pAuthorization.c_str(), "Digest"))
		{
			needBasic = false;

			m_szRealm = getAuthenAttr(pAuthorization, "realm");
			m_szNonce = getAuthenAttr(pAuthorization, "nonce");
		}
		else
		{
			needBasic = true;
		}

		return true;
	}

	static std::string getAuthenAttr(const std::string& pAuthorization, const std::string& key)
	{
		const char* tmpstr = pAuthorization.c_str();

		std::string flag = key + "=\"";
		const char* findstr = strstr(tmpstr, flag.c_str());
		if (findstr == NULL)
		{
			return "";
		}
		findstr += flag.length();
		const char* endstr = strchr(findstr, '"');
		if (endstr == NULL)
		{
			return "";
		}
		return std::string(findstr, endstr - findstr);
	}
};

#endif //__RTSPCMD_H__
