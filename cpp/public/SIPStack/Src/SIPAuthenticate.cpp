//#include "DBManager/DBManager.h"
#include "osipparser2/osip_md5.h"
#include "SIPAuthenticate.h"
#include <stdlib.h>
//add by zw
#ifndef UINT8
#define UINT8 unsigned char
#endif

class Md5Cacle
{
public:
	static void DigestCalcHA1(char *pszAlg,char *pszUserName,char *pszRealm,char *pszPassword,char *pszNonce,char *pszCNonce,char SessionKey[33])
	{
		osip_MD5_CTX Md5Ctx;
		unsigned char HA1[16] = {0};
		char ff[8] = {0};
		int i = 0;

		osip_MD5Init(&Md5Ctx); 
		osip_MD5Update(&Md5Ctx, (UINT8 *)(pszUserName == NULL ? "" : pszUserName),(pszUserName == NULL ? 0 : strlen(pszUserName)));
		osip_MD5Update(&Md5Ctx, (UINT8 *)":", 1);
		osip_MD5Update(&Md5Ctx, (UINT8 *)(pszRealm == NULL ? "" : pszRealm),(pszRealm == NULL ? 0 : strlen(pszRealm)));
		osip_MD5Update(&Md5Ctx, (UINT8 *)":", 1);
		osip_MD5Update(&Md5Ctx, (UINT8 *)(pszPassword == NULL ? "" : pszPassword),(pszPassword == NULL ? 0 : strlen(pszPassword)));
		osip_MD5Final(HA1, &Md5Ctx);

		if (strcasecmp(pszAlg, "md5-sess") == 0)
		{
			osip_MD5Init(&Md5Ctx); 
			osip_MD5Update(&Md5Ctx, HA1, 16); 
			osip_MD5Update(&Md5Ctx, (UINT8 *)":", 1); 
			osip_MD5Update(&Md5Ctx, (UINT8 *)(pszNonce == NULL ? "" : pszNonce),(pszNonce == NULL ? 0 : strlen(pszNonce)));
			osip_MD5Update(&Md5Ctx, (UINT8 *)":", 1); 
			osip_MD5Update(&Md5Ctx, (UINT8 *)(pszCNonce == NULL ? "" : pszCNonce),(pszCNonce == NULL ? 0 : strlen(pszCNonce)));
			osip_MD5Final(HA1, &Md5Ctx); 
		}

		for (i=0; i<16; i++)
		{ 
			if (i == 0)
			{
				sprintf(SessionKey,  "%02x", HA1[i]);
			}
			else
			{
				sprintf(ff,  "%02x", HA1[i]);
				strcat(SessionKey, ff);
			}
		}
	}
	static void DigestCalcResponse(char HA1[33],char *pszNonce,char *pszNonceCount,char *pszCNonce,char *pszQop,char *pszMethod,char *pszDigestUri,char HEntity[33],char Response[33])
	{
		osip_MD5_CTX Md5Ctx;
		unsigned char HA2[16] ={0};
		unsigned char RespHash[16] ={0};
		char HA2Hex[33] ={0};
		char ff[8] = {0};
		int i = 0;
	
		osip_MD5Init(&Md5Ctx);
		osip_MD5Update(&Md5Ctx, (UINT8 *)(pszMethod == NULL ? "" : pszMethod),(pszMethod == NULL ? 0 : strlen(pszMethod)));
		osip_MD5Update(&Md5Ctx, (UINT8 *)":", 1);
		osip_MD5Update(&Md5Ctx, (UINT8 *)(pszDigestUri == NULL ? "" : pszDigestUri),(pszDigestUri == NULL ? 0 : strlen(pszDigestUri)));
		
		if (strcasecmp(pszQop, "auth-int") == 0)
		{	
			osip_MD5Update(&Md5Ctx, (UINT8 *)":", 1);
			osip_MD5Update(&Md5Ctx, (UINT8 *)HEntity, 32);
		}
		osip_MD5Final(HA2, &Md5Ctx);
		for (i=0; i<16; i++)
		{ 
			sprintf(ff,  "%02x", HA2[i]);
			strcat(HA2Hex,ff);
		}
	
		osip_MD5Init(&Md5Ctx);
		osip_MD5Update(&Md5Ctx, (UINT8 *)(HA1 == NULL ? "" : HA1),(HA1 == NULL ? 0 :32));
		osip_MD5Update(&Md5Ctx, (UINT8 *)":", 1);
		osip_MD5Update(&Md5Ctx, (UINT8 *)(pszNonce == NULL ? "" : pszNonce),(pszNonce == NULL ? 0 : strlen(pszNonce)));
		osip_MD5Update(&Md5Ctx, (UINT8 *)":", 1);
		if (pszQop && *pszQop) 
		{
			osip_MD5Update(&Md5Ctx, (UINT8 *)(pszNonceCount == NULL ? "" : pszNonceCount),(pszNonceCount == NULL ? 0 : strlen(pszNonceCount)));
			osip_MD5Update(&Md5Ctx, (UINT8 *)":",1); 
			osip_MD5Update(&Md5Ctx, (UINT8 *)(pszCNonce == NULL ? "" : pszCNonce),(pszCNonce == NULL ? 0 : strlen(pszCNonce)));
			osip_MD5Update(&Md5Ctx, (UINT8 *)":", 1); 
			osip_MD5Update(&Md5Ctx, (UINT8 *)pszQop, strlen(pszQop)); 
			osip_MD5Update(&Md5Ctx, (UINT8 *)":", 1); 
		}
		osip_MD5Update(&Md5Ctx, (UINT8 *)HA2Hex, 32); 
		osip_MD5Final(RespHash, &Md5Ctx); 
		for (i=0; i<16; i++)
		{ 
			if (i == 0)
			{
				sprintf(Response, "%02x", RespHash[i]);
			}
			else
			{
				sprintf(ff, "%02x", RespHash[i]);
				strcat(Response,ff);
			}
		}
	}
};


const char* SIPAuthenticate::unquote(const char* str)
{
	if(str == NULL || strlen(str) == 0)
	{
		return "";
	}
	char* pstart = (char*)str;
	char* pend = (char*)str + strlen(str) - 1;

	while (*pstart == ' ' || *pstart == '\"')
	{
		pstart ++;
	}
	while (*pend == ' ' || *pend == '\"' || *pend == '\r' || *pend == '\n')
	{
		*pend = 0;
		pend --;
	}
	
	return pstart;
}

std::string getGenerateRandom()
{
	char buffer[64];
	char* tmp = buffer;
	
	srand((unsigned int)Time::getCurrentMilliSecond());
	for(int i = 0;i < 8;i +=2)
	{
		sprintf(tmp + i,"%02x",rand()&0xff);
	}
	return buffer;
}

osip_www_authenticate_t * SIPAuthenticate::getWWWAuthenticate(const osip_message_t * msg,const std::string& username)
{
	osip_www_authenticate_t * pWWWAuth = NULL;
	if (-1 == osip_www_authenticate_init(&pWWWAuth))
	{
		return NULL;	
	}

	std::string noce1 = getGenerateRandom();
	std::string noce2 = getGenerateRandom();

	osip_www_authenticate_set_auth_type(pWWWAuth, osip_strdup("Digest") );

	char tmp[128];

	sprintf(tmp,"\"%s\"",username.c_str());
	osip_www_authenticate_set_realm(pWWWAuth, osip_strdup(tmp));

	sprintf(tmp,"\"%s%s\"",noce1.c_str(),noce2.c_str());
	osip_www_authenticate_set_nonce(pWWWAuth, osip_strdup(tmp));

	return pWWWAuth;
}

bool SIPAuthenticate::Authenticate(const osip_message_t * msg,const std::string& name,const std::string& passwd)
{
	osip_authorization_t *pAuthEcho = NULL;
	if (osip_message_get_authorization(msg, 0, &pAuthEcho) == -1)
	{
		return false;
	}

	char *pAlgorithm  = NULL;
	char *pUsername   = NULL;
	char *pRealm      = NULL;
	char *pPasswd     = NULL;
	char *pNonce      = NULL;
	char *pNonceCount = NULL;
	char *pCNonce     = NULL;
	char *pQop        = NULL;
	char *pMethod     = NULL;
	char *pUri        = NULL;
	char sNullAlg[]   = "";  //"MD5";
	char sNullQop[]   = "";  //"auth";
	char SessionKey[33] = {0};
	char Response[64] = {0};
	char *pResponse2  = NULL;

	

	if (NULL == pAuthEcho->algorithm)
		pAlgorithm = (char*)unquote(sNullAlg);
	else
		pAlgorithm = (char*)unquote(pAuthEcho->algorithm == NULL ? "" : pAuthEcho->algorithm);

	pUsername  = (char*)unquote(pAuthEcho->username == NULL ? "" : pAuthEcho->username);
	pRealm     = (char*)unquote(pAuthEcho->realm == NULL ? "" : pAuthEcho->realm);
	pPasswd    = (char*)passwd.c_str();
	pNonce     = (char*)unquote(pAuthEcho->nonce == NULL ? "" : pAuthEcho->nonce);
	pCNonce    = (char*)unquote(pAuthEcho->cnonce == NULL ? "" : pAuthEcho->cnonce);

	if(strcmp(pUsername,name.c_str()) != 0)
	{
		//return false;
	}

	Md5Cacle::DigestCalcHA1(pAlgorithm, pUsername, pRealm, pPasswd, pNonce, pCNonce, SessionKey);

	pNonceCount = (char*)unquote(pAuthEcho->nonce_count == NULL ? "" : pAuthEcho->nonce_count);

	if (NULL == pAuthEcho->message_qop)
		pQop = (char*)unquote(sNullQop);
	else
		pQop = (char*)unquote(pAuthEcho->message_qop);

	pMethod = msg->sip_method;
	pUri    = (char*)unquote(pAuthEcho->uri == NULL ? "" : pAuthEcho->uri);

	Md5Cacle::DigestCalcResponse(SessionKey, pNonce, pNonceCount, pCNonce, pQop, pMethod, pUri, (char*)"", Response);

	pResponse2  = (char*)unquote(pAuthEcho->response);
	if (strcmp(Response, pResponse2) != 0)
	{
		return false;
	}

	return true;
}
