/**
 * Э�������ͷ�ļ�
 *
 * �������ı������ɽṹ��
 * ���߰ѽṹ��ת�����ַ���
 *
 * �����������̰߳�ȫ��
 */

#pragma once


#include "GBStack/GBTypes.h"
#include "GBStack/GBStructs.h"
#include "InterProtocol/InterProtocol_Defs.h"

#define CONTENT_TYPE_MANSCDP_RTSP		"Application/MANSRTSP"
#define CONTENT_TYPE_MANSCDP_XML		"Application/MANSCDP+xml" 
#define CONTENT_TYPE_MANSCDP_SDP		"application/sdp"

//��ȡ�ַ���������(�޷�ʶ��E_PT_FILE_QUERY E_PT_FILE_RESULT)
INTERPROTOCOL_API int GetMsgType(string strXml, EProtocolType& eType, int* pSn=NULL);
// ��xml�ַ��������ɽṹ��
INTERPROTOCOL_API int ParseMsg(string strXml, EProtocolType eType, void* pData);


//�����ַ���
INTERPROTOCOL_API int BuildMsg(EProtocolType eType, const void* pData, string& strXml);


