#include "GBStack/publicid.h"
#include <string.h>
#include <memory.h>


CPublicID::CPublicID(const std::string &idstr):svrtype(PUBIDTYPE_ERROR)
{
	if(idstr != "")
	{
		PrasePubID(idstr);
	}
}
CPublicID::~CPublicID()
{

}

std::string CPublicID::GetPubID(const std::string &idstr)
{
	if(idstr == "")
	{
		return "";
	}

	static char buffer[32];
	char* string = (char*)idstr.c_str();
	char *ptmp;
	if(memcmp(string,"sip:",4) == 0)
	{
		string += 4;
	}
	ptmp = strchr(string,'@');
	if(ptmp != NULL)
	{
		if(ptmp - string != PUBID_TOTAL_LEN)
		{
			return NULL;
		}
		memcpy(buffer,string,ptmp-string);
		buffer[ptmp-string] = 0;
	}
	else
	{
		if(strlen(string) != PUBID_TOTAL_LEN)
		{
			return NULL;
		}
		strcpy(buffer,string);
	}

	return buffer;
}

void CPublicID::PrasePubID(const std::string& _idstr)
{
	if(_idstr == "")
	{
		return;
	}
	svrtype = PUBIDTYPE_ERROR;

	std::string idstr = _idstr;
	
	if(idstr.length() < PUBID_TOTAL_LEN)
	{
		std::string emptysip = "00000000002160000000";

		idstr = idstr + std::string(emptysip.c_str() + idstr.length(),PUBID_TOTAL_LEN - idstr.length());
	}

	std::string buffer = GetPubID(idstr);
	if(buffer == "")
	{
		return;
	}
	if(sscanf(buffer.c_str(),"%02d%02d%02d%02d%02d%03d%07d",&provice,&city,&area,&unit,&vocation,&type,&sn) != 7)
	{
		return;
	}
	
	if(type>= 111 && type <= 199)
	{
		svrtype = PUBIDTYPE_DEVICE;
	}
	else if(type >= 200 && type <= 209)
	{
		svrtype = (PUBIDTYPE_E)type;
	}
	else if(type >= 210 && type <= 299)
	{
		svrtype = PUBIDTYPE_EXTENDPLATFORM;
	}
	else if(type >= 300 && type <= 399)
	{
		svrtype = PUBIDTYPE_CENTERUSER;
	}
	else if(type >= 400 && type <= 499)
	{
		svrtype = PUBIDTYPE_CLIENT;
	}
	else if(type >= 500 && type <= 999)
	{
		svrtype = PUBIDTYPE_EXTEND;
	}
	else
	{
		return;
	}
}

PUBIDTYPE_E CPublicID::GetSvrType()
{
	return svrtype;
}
int	CPublicID::GetProvice()
{
	return (svrtype == PUBIDTYPE_ERROR)?-1:provice;
}
int CPublicID::GetCity()
{
	return (svrtype == PUBIDTYPE_ERROR)?-1:city;
}
int CPublicID::GetArea()
{
	return (svrtype == PUBIDTYPE_ERROR)?-1:area;
}
int CPublicID::GetUnit()
{
	return (svrtype == PUBIDTYPE_ERROR)?-1:unit;
}
int CPublicID::GetVocation()
{
	return (svrtype == PUBIDTYPE_ERROR)?-1:vocation;
}

int CPublicID::GetType()
{
	return (svrtype == PUBIDTYPE_ERROR)?-1:type;
}
int CPublicID::GetSN()
{
	return (svrtype == PUBIDTYPE_ERROR)?-1:sn;
}

bool CPublicID::IsSvr() 
{
	return (svrtype >= PUBIDTYPE_SIPSVR && svrtype < PUBIDTYPE_CENTERUSER);
}
bool CPublicID::IsDev() 
{
	return svrtype == PUBIDTYPE_DEVICE;
}
bool CPublicID::IsClient() 
{
	return svrtype == PUBIDTYPE_CLIENT;
}

char* CPublicID::BuildPubID(int _type,int _sn)
{
	static char pubid[32];

	if(svrtype == PUBIDTYPE_ERROR)
	{
		return NULL;
	}
	_type = (_type > PUBIDMAXTYPEVALUE)?(_type%PUBIDMAXTYPEVALUE):_type;
	_sn   = (_sn > PUBIDMAXSNVALUE)?(_sn%PUBIDMAXSNVALUE):_sn;
	
	sprintf(pubid,"%02d%02d%02d%02d%02d%03d%07d",provice,city,area,unit,vocation,_type,_sn);

	return pubid;
}
int CPublicID::GetSipType_s(const std::string& idstr)
{
	CPublicID pubid(idstr);

	return pubid.GetSvrType();
}
int CPublicID::GetSN_s(const std::string&idstr)
{
	CPublicID pubid(idstr);

	return pubid.GetSN();
}
int CPublicID::GetType_s(const std::string&idstr)
{
	CPublicID pubid(idstr);

	return pubid.GetType();
}
std::string CPublicID::BuildPubID_s(const std::string& id,int _type,int _sn)
{
	CPublicID pubid(id);

	return pubid.BuildPubID(_type,_sn);
}

std::string CPublicID::GetPubID_s(const std::string& idstr)
{
	CPublicID pubid;
	
	return pubid.GetPubID(idstr);
}

bool CPublicID::IsSamePubID(const std::string&idstr1,const std::string&idstr2)
{
	if(idstr1 == "" || idstr2 == "")
	{
		return false;
	}
	std::string pid1 = PUBIDGetPubID(idstr1);
	std::string pid2 = PUBIDGetPubID(idstr2);
	

	return pid1 == pid2;
}


bool  CPublicID::IsSvr_s(const std::string&idstr)
{
	CPublicID pubid(idstr);


	return pubid.IsSvr();
}

bool CPublicID::IsDev_S(const std::string&idstr)
{
	CPublicID pubid(idstr);


	return pubid.IsDev();
}


