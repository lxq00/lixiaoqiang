/**
 * ��ʽ��䶨���ļ�
 */
#pragma once



//xmlͷ
#define XML_HEAD \
"<?xml version=\"1.0\" encoding=\"gb2312\" ?>\r\n"

//һ������Ԫ��
#define XML_ELEMENT1(name,value) \
	"<"#name">" << (value) << "</"#name">\r\n"

//��������Ԫ��
#define XML_ELEMENT2(name,value) \
	"<"#name">" << (value) << "</"#name">\r\n"

//��������Ԫ��
#define XML_ELEMENT3(name,value) \
	"<"#name">" << (value) << "</"#name">\r\n"

//��������
#define XML_CMD_TYPE(ct)		XML_ELEMENT1(CmdType, ct)
//snԪ��
#define XML_SN(sn)				XML_ELEMENT1(SN,sn)
//�豸idԪ��
#define XML_DEVICE_ID(id)		XML_ELEMENT1(DeviceID,id)

//���ת�����ַ���
#define RESULT_TO_STR(s)		((s)?"OK":"ERROR")
//״̬ת�����ַ���
#define STATUS_TO_STR(s)		((s)?"ON":"OFF")
//#define STATUS_TO_STR RESULT_TO_STR
