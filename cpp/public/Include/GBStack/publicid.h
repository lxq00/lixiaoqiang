#ifndef __PUBLICID_H_LIXIAOQIANG__
#define __PUBLICID_H_LIXIAOQIANG__
#include <stdio.h>
#include <string>
using namespace std;
#include "GBStack/GBStackDefs.h"
#define PUBID_PROVINCE_POS	0
#define PUBID_PROVINCE_LEN	2

#define PUBID_CIRY_POS		PUBID_PROVINCE_LEN
#define PUBID_CITY_LEN		2

#define PUBID_AREA_POS		(PUBID_CIRY_POS + PUBID_CITY_LEN)
#define PUBID_AREA_LEN		2

#define PUBID_UNIT_POS		(PUBID_AREA_POS + PUBID_AREA_LEN)
#define PUBID_UNIT_LEN		2

#define PUBID_VOCATION_POS	(PUBID_UNIT_POS + PUBID_UNIT_LEN)
#define PUBID_VOCATION_LEN	2

#define PUBID_TYPE_POS		(PUBID_VOCATION_POS + PUBID_VOCATION_LEN)
#define PUBID_TYPE_LEN		3

#define PUBID_SN_POS		(PUBID_TYPE_POS + PUBID_TYPE_LEN)
#define PUBID_SN_LEN		7

#define PUBID_TOTAL_LEN		(PUBID_SN_POS + PUBID_SN_LEN)

typedef enum{
	PUBIDTYPE_ERROR = 0,
	PUBIDTYPE_DEVICE = 100,
	PUBIDTYPE_SIPSVR = 200,
	PUBIDTYPE_WEBSVR,
	PUBIDTYPE_MEDIASVR,
	PUBIDTYPE_PROXYSVR,
	PUBIDTYPE_SAFESVR,
	PUBIDTYPE_ALARMSVR,
	PUBIDTYPE_DBSVR,
	PUBIDTYPE_GISSVR,
	PUBIDTYPE_ADMINSVR,
	PUBIDTYPE_GATEWAY,
	PUBIDTYPE_EXTENDPLATFORM,
	PUBIDTYPE_CENTERUSER = 300,
	PUBIDTYPE_CLIENT = 400,
	PUBIDTYPE_EXTEND = 500,
}PUBIDTYPE_E;


#define PUBIDTYPE_LowrPlatfrom		PUBIDTYPE_EXTEND + 1
#define PUBIDTYPE_UpPlatfrom	PUBIDTYPE_LowrPlatfrom + 1

class GB28181_API CPublicID
{
public:
	//sip:22345599818717@192.168.1.21:2039
	CPublicID(const std::string & idstr = "");
	~CPublicID();
	std::string GetPubID(const std::string &idstr);
	void PrasePubID(const std::string &idstr);

	PUBIDTYPE_E GetSvrType();
	int	GetProvice();
	int GetCity();
	int GetArea();
	int GetUnit();
	int GetVocation();

	int GetType();
	int GetSN();

	bool IsSvr();
	bool IsDev();
	bool IsClient();

	bool IsOnePlatFrorm(const std::string & aor);

	char* BuildPubID(int type,int sn);

	static int GetSipType_s(const std::string & idstr);
	static int GetSN_s(const std::string & idstr);
	static int GetType_s(const std::string & idstr);
	static std::string BuildPubID_s(const std::string & id,int type,int sn);
	static std::string GetPubID_s(const std::string & idstr);
	static bool IsSamePubID(const std::string &idstr1,const std::string &idstr2);
	static bool IsSvr_s(const std::string &idstr);
	static bool IsDev_S(const std::string &idstr);
private:
	PUBIDTYPE_E svrtype;
	int provice;
	int city;
	int area;
	int unit;
	int vocation;
	int type;
	int sn;
};

#define PUBIDMAXTYPEVALUE	999
#define PUBIDMAXSNVALUE		9999999
#define PUBIDGetSN(idstr)	CPublicID::GetSN_s(idstr)
#define PUBIDGetSvrType(idstr)	CPublicID::GetSipType_s(idstr)
#define PUBIDGetType(idstr)	CPublicID::GetType_s(idstr)
#define PUBIDBuild(id,type,sn)	CPublicID::BuildPubID_s(id,type,sn)
#define PUBIDGetPubID(idstr)	CPublicID::GetPubID_s(idstr)
#define PUBIDIsSamePubID(idstr1,idstr2) CPublicID::IsSamePubID(idstr1,idstr2)
#define PUBIDISSVR(idstr) CPublicID::IsSvr_s(idstr)
#define PUBIDISDEV(idstr) CPublicID::IsDev_S(idstr)



#endif //__PUBLICID_H_LIXIAOQIANG__
