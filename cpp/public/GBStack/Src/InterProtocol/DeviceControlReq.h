#pragma once
namespace NS_DeviceControlReq
{
	IMPLEMENT_BASE_FUN(E_PT_DEVICE_CONTROL_REQ, "Control","DeviceControl")

	//��������
	int Parse(string& strXml, void* pData);
	
	//��������
	int Build(const void* pData, string& strXml);
//private:
	/*//����ptz��Ϣ
	int ParsePtzInfo(const char* pPtz, TPtzCmd& oPtzInfo); 
	//����ptz��Ϣ
	void BuildPtzInfo(const TPtzCmd& oPtzInfo, ostringstream& oss);*/

	bool tranPtzInfo(const char* input, char* out);

	void BuildFi(int ptz, int p, ostringstream& oss);
	bool ParseFi(char* info, int& ptz, int& speed);
	void BuildY(int ptz, int p, ostringstream& oss);
	bool ParseY(char* info, int& ptz, int& pos);
	void BuildPtz(int ptz, int p, ostringstream& oss);
	bool ParsePtz(char* info, int& ptz, int& speed);
}

