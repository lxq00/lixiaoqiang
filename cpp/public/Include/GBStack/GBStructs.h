/**
 * �ṹ�嶨��ͷ�ļ�
 *
 */

#pragma once
#include <string>
#include <cstring>
#include <vector>
#include <map>
#include <list>
#include "SIPStack/SIPSession.h"
using namespace std;
using namespace Public::SIPStack;

/**
 *E_PT_DEVICE_CONTROL
 */
//��̨��������
struct TPtzCmd
{
	bool bControl;		//�Ƿ������̨
	int nAddress;			//��ַ����ʱ��֪���к���;(0-4095)
	EPtzZoomOpt eZoomOpt;	//�䱶����
	EPtzTiltOpt eTiltOpt;	//��ֱ�������
	EPtzPanOpt ePanOpt;		//ˮƽ�������
	int nTiltSpeed;			//��ֱ�����ٶ�(0-255)
	int nPanSpeed;			//ˮƽ�����ٶ�(0-255)
	int nZoomSpeed;			//�䱶�ٶ�(0-15)
};
/*
//�豸������������
struct TDeviceControlReq
{
	int nSn;					// *���к�
	string strDeviceId;			// *�豸����
	TPtzCmd oPtzCmd;			//��̨��������
	BOOL bTeleBoot;				//�Ƿ�Զ������
	ERecordCmd eRecordCmd;		//¼����Ʋ���	
	EGuardCmd eGuardCmd;		//��������/��������
	BOOL bAlarmCmd;				//������λ����
	vector<string> vExtendInfo;	//��չ��Ϣ

	TDeviceControlReq():bTeleBoot(0),eRecordCmd(E_RC_NONE),eGuardCmd(E_GC_NONE),
		bAlarmCmd(0){memset(&oPtzCmd,0,sizeof(TPtzCmd));}
};
*/

enum Ptz
{
	PtzStop = 0,

	PtzUp,
	PtzDwon,
	PtzLeft,
	PtzRight,
	PtzLU,
	PtzRU,
	PtzLD,
	PtzRD,
	PtzAuto,

	PtzZI,
	PtzZO,

	PtzII,
	PtzIO,
	PtzFI,
	PtzFO,

	PtzYS,
	PtzYD,
	PtzYC,
};
//�豸������������
struct TDeviceControlReq
{
	int nSn;					//*���к�
	string strDeviceId;			//*�豸����
	int ptz;
	int ptzParam;
	bool bTeleBoot;				//�Ƿ�Զ������
	ERecordCmd eRecordCmd;		//¼����Ʋ���	
	EGuardCmd eGuardCmd;		//��������/��������
	bool bAlarmCmd;				//������λ����
	vector<string> vExtendInfo;	//��չ��Ϣ

	TDeviceControlReq():ptz(-1), bTeleBoot(0),eRecordCmd(E_RC_NONE),eGuardCmd(E_GC_NONE),
		bAlarmCmd(0){}
};

//�豸���������Ӧ
struct TDeviceControlResp
{
	int nSn;					//*���к�
	string strDeviceId;			//*�豸����
	bool bResult;				//*���
};



/**
 *E_PT_DEVICE_STATUS_QUERY
 */
//�豸״̬��Ϣ����
struct TDeviceStatusQueryReq
{
	TDeviceStatusQueryReq():bSubcribe(false){}
	int nSn;					//*���к�
	string strDeviceId;			//*�豸����

	bool   bSubcribe;
	string strStartTime;
	string strEndTime;
};
//����״̬��Ϣ
struct TAlarmStatus
{
	string strDeviceId;			//*�豸����
	EAlarmStatus eAlarmStatus;	//*����״̬
};
//�豸״̬��Ϣ��Ӧ
struct TDeviceStatusQueryResp
{
	int nSn;					//*���к�
	string strDeviceId;			//*�豸����
	bool bResult;				//*���(�ɹ���ʧ��)
	bool bOnline;				//*�Ƿ�����
	bool bStatus;				//*�Ƿ���
	string strReason;			//��������ԭ��(����Ϊ��)
	bool bEncode;				//����״̬
	bool bRecord;				//¼��״̬
	string strDeviceTime;		//�豸ʱ��
	vector<TAlarmStatus> vTAlarmStatus;//����״̬�б�
	vector<string> vExtendInfo;	//��չ��Ϣ

	TDeviceStatusQueryResp():bEncode(false),bRecord(false){}
};


/**
 *E_PT_DEVICE_STATUS_QUERY
 */
//�豸Ŀ¼��ѯ����
typedef TDeviceStatusQueryReq TDeviceCatalogQueryReq;
//�豸��Ŀ��Ϣ
struct TDeviceItem
{
	string strDeviceId;										//�豸/����/ϵͳ����(20���ַ���ʶ)
	string strName;												//�豸/����/ϵͳ����
	string strManufacturer;								//�豸����
	string strFirameware;								//�̼��汾 add by zhongwu
	string strModel;											//�豸�ͺ�
	string strOwner;											//�豸����
	string strCivilCode;										//��������
	string strBlock;												//����
	string strAddress;											//��װ��ַ
	int nParental;												//�Ƿ������豸
	string strParentId;										//���豸/����/ϵͳID(20���ַ���ʶ)
	int safetyMode;											//���ȫģʽ
	int nRegisterWay;										//ע�᷽��
	string certificateNo;										//֤�����к�
	string certificateIndentify;							//֤����Чʶ��
	int certificateError;										//��Чԭ����
	string certificateEnding;								//֤����ֹ��Ч��
	int nSecrecy;													//	��������
	string strIPAddress;										//�豸/����/ϵͳIP��ַ
	int nPort;														//�豸/����/ϵͳ�˿ں�
	string strPasswd;											//�豸����
	int bStatus;												//�豸״̬  -1 unused 1:online 0 :offline
	float fLongitude;											//����
	float fLatitude;												//γ��
	int	  nCode;

	TDeviceItem():nParental(-1),safetyMode(-1),nRegisterWay(-1),nSecrecy(-1),nPort(-1),bStatus(0),fLongitude(-1),fLatitude(-1),nCode(-1){}
};
//�豸Ŀ¼��ѯ��Ӧ
struct TDeviceCatalogQueryResp
{
	TDeviceCatalogQueryResp():bSubcribe(false){}
	int nSn;					//*���к�
	int nSumNum;				//���� -1 ��ʾ��Ч
	bool bEndofFile;			//�Ƿ����
	string strDeviceId;			//*�豸����
	vector<TDeviceItem> vDeviceItem;//�豸�б�
	vector<string> vExtendInfo;	//��չ��Ϣ

	bool			bSubcribe;
	bool			bResult;
};



/**
 *E_PT_DEVICE_INFO_QUERY
 */
//�豸��Ϣ��ѯ����
typedef TDeviceStatusQueryReq TDeviceInfoQueryReq;
//�豸��Ϣ��ѯ��Ӧ
struct TDeviceInfoQueryResp
{
	int nSn;					//*���к�
	string strDeviceId;			//*�豸����
	bool bResult;				//*���
	string strManufacturer;		//�豸������
	string strModel;			//�豸�ͺ�
	string strFirmware;			//�豸�̼��汾
	int nMaxCamera;				//ͨ������
	int nMaxAlarm;
	vector<string> vExtendInfo;	//��չ��Ϣ
	TDeviceInfoQueryResp():nMaxCamera(-1), nMaxAlarm(-1){}
};


/**
 *E_PT_DEVICE_CONTROL
 */
//�豸״̬֪ͨ����
struct TKeepAliveNotify
{
	int nSn;					//*���к�
	string strDeviceId;			//*�豸����
	bool bStatus;				//*״̬
};

struct TAlarmQueryReq
{
	int nSn;					//*���к�
	string strDeviceId;			//*�豸����
	int nStartAlarmPriority;
	int nEndAlarmPriority;
	int nAlarmMethod;			//*������ʽ(1-7)
	string strStartTime;
	string strEndTime;
};


/**
 *E_PT_DEVICE_CONTROL
 */
//����֪ͨ����
struct TAlarmNotifyReq
{
	int nSn;					//*���к�
	string strDeviceId;			//*�豸����
	int nAlarmPriority;			//*��������(1-4)
	int nAlarmMethod;			//*������ʽ(1-7)
	string strAlarmTime;		//*����ʱ��
	string strAlarmDescription;	//��������
	double dLongitude;			//����
	double dLatitude;			//ά��
	std::map<string,string> vExtendInfo;	//��չ��Ϣ

	TAlarmNotifyReq():dLongitude(-1),dLatitude(-1){}
};
//����֪ͨӦ��
typedef TDeviceControlResp TAlarmNotifyResp;

struct TAlarmSubcribeReq
{
	int nSn;
	string strDeviceId;
	
};

//��ʷ�ļ���Ϣ��ѯ����
struct TRecordInfoQueryReq
{
	int nSn;					//*���к�
	string strDeviceId;			//*�豸����
	string strStartTime;		//��ʼʱ��
	string strEndTime;			//����ʱ��
	string strFilePath;			//�ļ���
	string strAddress;			//��ַ
	int nSecrecy;				//����
	ERecordInfoType eType;		//����
	string strRecorderID;		//����id
};


//��ʷ�ļ���Ϣ��ѯ��Ӧ
struct TRecordItem
{
	string strDeviceId;			//*�豸����
	string strName;				//*����
	string strStartTime;		//��ʼʱ��
	string strEndTime;			//����ʱ��
	string strFilePath;			//�ļ���
	string strAddress;			//��ַ
	unsigned long long	nFileSize;				//�ļ���С
	int nSecrecy;				//����
	ERecordInfoType eType;		//����
	string strRecorderID;		//����id
};
struct TRecordInfoQueryResp
{
	int nSn;					//*���к�
	string strDeviceId;			//*�豸����
	string strName;				//*��ʼʱ��
	int nSumNum;				//*����ʱ��
	bool bEndofFile;			//�Ƿ����
	vector<TRecordItem> vRecordList;
};

//E_PT_DECODER_CONTROL_REQ
struct TDecoderControlReq
{
	int nSn;
	string strDecoderId;
	string strDeviceId;
	bool bStart;
};
typedef TDeviceControlResp TDecoderControlResp;

//E_PT_DECODER_INFO_QUERY_REQ
struct TDecoderInfoQueryReq
{
	int nSn;
	string strDecoderId;
};

//E_PT_DECODER_INFO_QUERY_RESP
struct TDecoderInfo
{
	string strDecoderId;
	string strDeviceId;
};
struct TDecoderInfoQueryResp
{
	int nSn;
	string strDeviceId;
	vector<TDecoderInfo> vDecoderInfo;
};

typedef struct{
	string		 DeviceAor;			//�豸��ַ����
	string		 DestIpAddr;			//������IP��ַ
	int			 DestPort;			//�����߽��ܶ˿�
	INVITETYPE_E PlayType;
	long		 StartTime;
	long		 EndTime;
	bool   		 IsOutPlatForm;
	int			 cseq;
}TInviteIMaxReq;


typedef struct{
	string		DeviceAor;			//�豸��ַ����
	int 		Result;				//ִ�н��
}TIviteIMaxResp;

struct TVodCtrlReq
{
	TVodCtrlReq():lStartTime(0),dSpeed(1){}
	ERtspType eType;//����
	int nSn;
	long lStartTime;
	double dSpeed;
};

struct TVodCtrlResp
{
	TVodCtrlResp():nSn(0),nResult(0),rtpinfo_seq(0),rtpinfo_time(0){}
	int nSn;
	int nResult;
	long rtpinfo_seq;
	long rtpinfo_time;
};

typedef struct{
	int			nSn;
	string		strDeviceId;
	int			nType;
}MediaStatusNotify;


