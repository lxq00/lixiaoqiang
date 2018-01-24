#ifndef __VGSII_URI_H__
#define __VGSII_URI_H__
#include "Base/Defs.h"
#include "Base/IntTypes.h"
#include <string>
#include <list>
using namespace std;
namespace Xunmei{
namespace Base{

#define VGSII_URI_PROTOCOL_VDR			"VDR"
#define VGSII_URI_HOST_Local			"local"

#define VGSII_URI_IDENTIFIER_VGSId		"vgsid"
#define VGSII_URI_IDENTIFIER_DvsId		"dvsid"
#define VGSII_URI_IDENTIFIER_Cameras	"Cameras"
#define VGSII_URI_IDENTIFIER_Storages	"Storages"
#define VGSII_URI_IDENTIFIER_Streams	"Streams"
#define VGSII_URI_IDENTIFIER_AlarmInputs "AlarmInputs"
#define VGSII_URI_IDENTIFIER_AlarmOutputs "AlarmOutputs"
#define VGSII_URI_IDENTIFIER_Talkbacks "Talkbacks"
#define VGSII_URI_IDENTIFIER_Disks	"disks"
#define VGSII_URI_IDENTIFIER_CSID		"csid"

//¼���ѯ�������
#define VGSII_URI_PARAMETER_ExtParam "extParam"
//¼�����
#define VGSII_URI_PARAMETER_Starttime "starttime"
#define VGSII_URI_PARAMETER_Stoptime "stoptime"
#define VGSII_URI_PARAMETER_TriggerType "TriggerType"
#define VGSII_URI_PARAMETER_CardNum "CardNum"
#define VGSII_URI_PARAMETER_IsLock "IsLock"
#define VGSII_URI_PARAMETER_LimitCount "LimitCount"
#define VGSII_URI_PARAMETER_Filename "filename"
#define VGSII_URI_PARAMETER_Camera "Camera"
#define VGSII_URI_PARAMETER_AddLabel "AddLabel"
#define VGSII_URI_PARAMETER_CleanLabel "CleanLabel"
#define VGSII_URI_PARAMETER_Label	"Label"
#define VGSII_URI_PARAMETER_StreamFmt	"StreamFmt"

#define VGSII_URI_STREAMFMT_STANDARD	"standard"

//��Ϊ�����������
#define VGSII_URI_PARAMETER_RuleAlarmType "RuleAlarmType"
//�豸�쳣�������
#define VGSII_URI_PARAMETER_DeviceExceptType "DeviceExceptType"
#define VGSII_URI_PARAMETER_DeviceExceptDesc "DeviceExceptDesc"

//�豸�ⲿ�������
#define VGSII_URI_PARAMETER_DeviceExternalType "DeviceExternalType"

//��־���
#define VGSII_URI_PARAMETER_MainType "MainType"
#define VGSII_URI_PARAMETER_SubType "SubType"
//͸������
#define VGSII_URI_PARAMETER_Cmd "Cmd"
#define VGSII_URI_PARAMETER_Parm "Parm"


class BASE_API URI
{
	struct URIInternal;
public:
	class BASE_API Value
	{
	public:
		Value() {}
		Value(const std::string& val);
		Value(char val);
		Value(const char* val);
		Value(const unsigned char* val);
		Value(int val);
		Value(float val);
		Value(long long val);
		Value(uint32_t val);
		Value(uint64_t val);
		Value(const Value& val);
		~Value();

		Value& operator = (const Value& val)
		{
			valuestring = val.valuestring; 
			return*this;
		}

		std::string readString() const;
		int readInt() const;
		float readFloat() const;
		long long readInt64() const;
		uint32_t readUint32() const;
		uint64_t readUint64() const;

		bool isEmpty() const;
	private:
		std::string valuestring;
	};
	struct BASE_API URIObject
	{
		std::string	key;
		Value		val;
		//������Identifier
		std::string toStringAsIdentifier();
		//������ExtParameter��Parameter
		std::string toStringAsParameter();
	};
public:
	//protocol Э�� �磺HTTP/FTP/VDR��
	URI(const std::string& protocol = VGSII_URI_PROTOCOL_VDR,const std::string& host = VGSII_URI_HOST_Local,int port = 0);
	URI(const std::string& protocol,int centerId);
	URI(const URI& uri);
	~URI();

	bool setProtocol(const std::string& protocol);
	
	bool setHost(const std::string& host,int port = 0);

	bool setHost(int hostId);

	bool setCenterId(int centerId);

	//��ӵ�ַ��ʶ�� val����Ϊ��
	//VDR://local/dvsid/610001/Storages/C?camera=camera/1&date=2012-3-5
	//dvsid/storages
	bool addIdentifier(const std::string& mark,const Value& val);

	//��Ӳ���
	//VDR://local/dvsid/610001/Storages/C?camera=camera/1&date=2012-3-5
	//camera /date
	bool addParameter(const std::string& key,const Value& val);
		

	//ɾ��һ����ַ��ʶ������Ӧ��ֵ
	bool removeIndentifier(const std::string& mark);
	//ɾ��һ����������Ӧ��ֵ
	bool removeParameter(const std::string& key);

	//��ȡ�����ַ���
	std::string getParmeterString() const;
	//���ַ��ķ�ʽ��Ӹ��Ӳ���
	bool addParmeterString(const std::string& exstring);

	//��ȡһ����ַ��ʶ��  NULL��ʾʧ��
	Value* getIdentifier(const std::string& mark) const;
	//��ȡһ������ NULL��ʾʧ��
	Value* getParameter(const std::string& key) const;


	//��ȡ��ַ��ʶ������
	std::list<URIObject> getIndentifierList() const;
	//��ȡ��������
	std::list<URIObject> getParmeterList() const;

	
	//��ȡЭ��
	std::string& getProtocol() const;
	//��ȡЭ��
	Value getHost() const;

	int getCenterId() const;
	//��ȡ�˿�		0��ʾ��
	int getPort() const;
	
	//ת��uri�ַ���
	std::string toString() const;

	///��ȡ��������ַ�������������������չ����
	std::string getBasicString() const;

	//Э��uri�ַ���
	bool parse(const std::string& uri);

	URI& operator = (const URI& uri);
private:
	URIInternal* internal;
};


}
}

#endif //__VGSII_URI_H__
