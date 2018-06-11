#ifndef __DEVICE_DEFINE_H__
#define __DEVICE_DEFINE_H__


//������������
typedef enum {
	DEVOBJECTTYPE_GATEWAY = 1,	//�����豸
	DEVOBJECTTYPE_MODULE,	//ģ���豸
	DEVOBJECTTYPE_LIGHTSWITH,	//���ص�����
	DEVOBJECTTYPE_LIGHTDIMMER, //�����ģ��
	DEVOBJECTTYPE_LIGHTRGB, //RGB�����
	DEVOBJECTTYPE_BLIND,	//����ģ��
	DEVOBJECTTYPE_IO,		//IO�豸
}DeviceObjectType;




//|online_state | �豸����״̬ | 0�������ߣ�1������| �����жϳ������豸������״̬|
#define DEVICEATTR_OnlineState "online_state"
//|status | �豸����״̬ | 0:�ر�״̬��1����״̬| ����״̬|
#define DEVICEATTR_Status "status"
//| brightness | ����ֵ | 0-100�ٷֱȣ�0Ϊ�ر� | ��������� | 
#define DEVICEATTR_Brightness "brightness"
//| color | ��ɫ | 0xrrggbb | RGB����ɫ | 
#define DEVICEATTR_Color  "color"
//| position | λ�ðٷֱ� | 0-100�ٷֱ� | �細���ٷֱ�λ�õ� |
#define DEVICEATTR_Position "position"
//| angle | �ǶȰٷֱ� | 0-90�� | ��ת�����Ƕȣ������Ҷ�� |
#define DEVICEATTR_Angle "angle"
//| target_temperature | �趨�¶� | ��λ�����϶� | �յ��¶ȵ�|
#define DEVICEATTR_Target_Temperature "target_temperature"
//| current_temperature | ��ǰ�¶� | ��λ�����϶� | �յ��¶ȵ�|
#define DEVICEATTR_Current_Temperature "current_temperature"
//| target_humidity | �趨ʪ�� | ���� 50RH%��������������ʪ�Ȱٷ�֮��ʮ| |
#define DEVICEATTR_Target_Humidity "target_humidity"
//| current_humidity | ��ǰʪ�� | ���� 50RH%��������������ʪ�Ȱٷ�֮��ʮ |
#define DEVICEATTR_Current_Humidity "current_humidity"
//| mode | ģʽ|  |  |
#define DEVICEATTR_Mode "mode"
//| wind | ���ٵȼ�|  |
#define DEVICEATTR_Wind "wind"
//| formaldehyde | ��ȩŨ��|  |
#define DEVICEATTR_Formaldehyde "formaldehyde"
//| co2 | CO2Ũ��|  |
#define DEVICEATTR_CO2 "co2"
//| pm2d5 | PM2.5Ũ��|  |
#define DEVICEATTR_PM2D5 "pm2d5"
//| mac_address | MAC��ַ | 00:00 : 00 : 00 : 00 : 00 |
#define DEVICEATTR_MAC_Address "mac_address"
//| ip_address | IP��ַ | 192.168.0.11 |
#define DEVICEATTR_IP_Address "ip_address"
//| port | �˿ں� | 0 - 65535 |
#define DEVICEATTR_Port "port"
//| network | �������� | UDP server; UDP client; TCP server; TCP client|  |
#define DEVICEATTR_Network "network"
//| baudrate | ������ | �磺96000 |  |
#define DEVICEATTR_Baudrate "baudrate"
//| traveltime | ����ʱ��|  | |
#define DEVICEATTR_Traveltime "traveltime"
//| scan_interval | ɨ������|  |
#define DEVICEATTR_Scan_Interval "scan_interval"
//| address | �豸��ַ|  | |
#define DEVICEATTR_Address "address"
//| channel | ģ��ͨ��|  | |
#define DEVICEATTR_Channel "channel"
//| dev_type | �豸���� | �豸���Ͷ��� 1:���ء�2��ģ�顢3�����صơ�4������ơ�5:RGB�ơ�6:������7:IO�豸|  |
#define DEVICEATTR_Dev_Type "dev_type"
//| module_type  | ģ������ | |  | 
#define DEVICEATTR_Module_Type "module_type"
//| identify | �豸��ʶ | �磺mac + gwid + channelid | ÿ��Ӳ���豸��Ψһ��ʶ |
#define DEVICEATTR_Entity_ID "entity_id"
//| fatherIdentify | ����ʶ | | Ӳ���豸����ʶ|
#define DEVICEATTR_FatherID "father_entity_id"
//| name | �豸���� | | Ӳ���豸���� |
#define DEVICEATTR_Name "name"
//| models | �豸�ͺ� | | Ӳ���豸�ͺ� |
#define DEVICEATTR_Models "models"
//| series | �豸ϵ�� | | Ӳ���豸ϵ�� |
#define DEVICEATTR_Series "series"

//|states | �豸״̬
#define DEVICEATTR_States "states"

#endif //__DEVICE_DEFINE_H__
