#ifndef __DEVICE_DEFINE_H__
#define __DEVICE_DEFINE_H__


//定义驱动类型
typedef enum {
	DEVOBJECTTYPE_GATEWAY = 1,	//网关设备
	DEVOBJECTTYPE_MODULE,	//模块设备
	DEVOBJECTTYPE_LIGHTSWITH,	//开关灯类型
	DEVOBJECTTYPE_LIGHTDIMMER, //调光灯模块
	DEVOBJECTTYPE_LIGHTRGB, //RGB调光灯
	DEVOBJECTTYPE_BLIND,	//窗帘模块
	DEVOBJECTTYPE_IO,		//IO设备
}DeviceObjectType;




//|online_state | 设备在线状态 | 0：不在线，1：在线| 用于判断长连接设备的在线状态|
#define DEVICEATTR_OnlineState "online_state"
//|status | 设备开关状态 | 0:关闭状态，1：打开状态| 工作状态|
#define DEVICEATTR_Status "status"
//| brightness | 亮度值 | 0-100百分比，0为关闭 | 调光灯亮度 | 
#define DEVICEATTR_Brightness "brightness"
//| color | 颜色 | 0xrrggbb | RGB灯颜色 | 
#define DEVICEATTR_Color  "color"
//| position | 位置百分比 | 0-100百分比 | 如窗帘百分比位置等 |
#define DEVICEATTR_Position "position"
//| angle | 角度百分比 | 0-90度 | 旋转窗帘角度：比如百叶窗 |
#define DEVICEATTR_Angle "angle"
//| target_temperature | 设定温度 | 单位：摄氏度 | 空调温度等|
#define DEVICEATTR_Target_Temperature "target_temperature"
//| current_temperature | 当前温度 | 单位：摄氏度 | 空调温度等|
#define DEVICEATTR_Current_Temperature "current_temperature"
//| target_humidity | 设定湿度 | 比如 50RH%这个数读作：相对湿度百分之五十| |
#define DEVICEATTR_Target_Humidity "target_humidity"
//| current_humidity | 当前湿度 | 比如 50RH%这个数读作：相对湿度百分之五十 |
#define DEVICEATTR_Current_Humidity "current_humidity"
//| mode | 模式|  |  |
#define DEVICEATTR_Mode "mode"
//| wind | 风速等级|  |
#define DEVICEATTR_Wind "wind"
//| formaldehyde | 甲醛浓度|  |
#define DEVICEATTR_Formaldehyde "formaldehyde"
//| co2 | CO2浓度|  |
#define DEVICEATTR_CO2 "co2"
//| pm2d5 | PM2.5浓度|  |
#define DEVICEATTR_PM2D5 "pm2d5"
//| mac_address | MAC地址 | 00:00 : 00 : 00 : 00 : 00 |
#define DEVICEATTR_MAC_Address "mac_address"
//| ip_address | IP地址 | 192.168.0.11 |
#define DEVICEATTR_IP_Address "ip_address"
//| port | 端口号 | 0 - 65535 |
#define DEVICEATTR_Port "port"
//| network | 网络类型 | UDP server; UDP client; TCP server; TCP client|  |
#define DEVICEATTR_Network "network"
//| baudrate | 波特率 | 如：96000 |  |
#define DEVICEATTR_Baudrate "baudrate"
//| traveltime | 运行时间|  | |
#define DEVICEATTR_Traveltime "traveltime"
//| scan_interval | 扫描周期|  |
#define DEVICEATTR_Scan_Interval "scan_interval"
//| address | 设备地址|  | |
#define DEVICEATTR_Address "address"
//| channel | 模块通道|  | |
#define DEVICEATTR_Channel "channel"
//| dev_type | 设备类型 | 设备类型定义 1:网关、2：模块、3、开关灯、4：调光灯、5:RGB灯、6:窗帘、7:IO设备|  |
#define DEVICEATTR_Dev_Type "dev_type"
//| module_type  | 模块类型 | |  | 
#define DEVICEATTR_Module_Type "module_type"
//| identify | 设备标识 | 如：mac + gwid + channelid | 每个硬件设备的唯一标识 |
#define DEVICEATTR_Entity_ID "entity_id"
//| fatherIdentify | 父标识 | | 硬件设备父标识|
#define DEVICEATTR_FatherID "father_entity_id"
//| name | 设备名称 | | 硬件设备名称 |
#define DEVICEATTR_Name "name"
//| models | 设备型号 | | 硬件设备型号 |
#define DEVICEATTR_Models "models"
//| series | 设备系列 | | 硬件设备系列 |
#define DEVICEATTR_Series "series"

//|states | 设备状态
#define DEVICEATTR_States "states"

#endif //__DEVICE_DEFINE_H__
