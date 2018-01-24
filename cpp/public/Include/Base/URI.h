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

//录像查询额外参数
#define VGSII_URI_PARAMETER_ExtParam "extParam"
//录像相关
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

//行为分析报警相关
#define VGSII_URI_PARAMETER_RuleAlarmType "RuleAlarmType"
//设备异常报警相关
#define VGSII_URI_PARAMETER_DeviceExceptType "DeviceExceptType"
#define VGSII_URI_PARAMETER_DeviceExceptDesc "DeviceExceptDesc"

//设备外部报警相关
#define VGSII_URI_PARAMETER_DeviceExternalType "DeviceExternalType"

//日志相关
#define VGSII_URI_PARAMETER_MainType "MainType"
#define VGSII_URI_PARAMETER_SubType "SubType"
//透明参数
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
		//适用于Identifier
		std::string toStringAsIdentifier();
		//适用于ExtParameter和Parameter
		std::string toStringAsParameter();
	};
public:
	//protocol 协议 如：HTTP/FTP/VDR等
	URI(const std::string& protocol = VGSII_URI_PROTOCOL_VDR,const std::string& host = VGSII_URI_HOST_Local,int port = 0);
	URI(const std::string& protocol,int centerId);
	URI(const URI& uri);
	~URI();

	bool setProtocol(const std::string& protocol);
	
	bool setHost(const std::string& host,int port = 0);

	bool setHost(int hostId);

	bool setCenterId(int centerId);

	//添加地址标识符 val不能为空
	//VDR://local/dvsid/610001/Storages/C?camera=camera/1&date=2012-3-5
	//dvsid/storages
	bool addIdentifier(const std::string& mark,const Value& val);

	//添加参数
	//VDR://local/dvsid/610001/Storages/C?camera=camera/1&date=2012-3-5
	//camera /date
	bool addParameter(const std::string& key,const Value& val);
		

	//删除一个地址标识符及对应的值
	bool removeIndentifier(const std::string& mark);
	//删除一个参数及对应的值
	bool removeParameter(const std::string& key);

	//获取参数字符串
	std::string getParmeterString() const;
	//已字符的方式添加附加参数
	bool addParmeterString(const std::string& exstring);

	//获取一个地址标识符  NULL表示失败
	Value* getIdentifier(const std::string& mark) const;
	//获取一个参数 NULL表示失败
	Value* getParameter(const std::string& key) const;


	//获取地址标识符集合
	std::list<URIObject> getIndentifierList() const;
	//获取参数集合
	std::list<URIObject> getParmeterList() const;

	
	//获取协议
	std::string& getProtocol() const;
	//获取协议
	Value getHost() const;

	int getCenterId() const;
	//获取端口		0表示无
	int getPort() const;
	
	//转成uri字符串
	std::string toString() const;

	///获取最基本的字符串，不包括参数及扩展参数
	std::string getBasicString() const;

	//协议uri字符串
	bool parse(const std::string& uri);

	URI& operator = (const URI& uri);
private:
	URIInternal* internal;
};


}
}

#endif //__VGSII_URI_H__
