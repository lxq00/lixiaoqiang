//
//  Copyright (c)1998-2012, Chongqing Xunmei Technology
//  All Rights Reserved.
//
//
//	Description:
//	$Id: Guid.cpp 254 2014-01-16 09:12:44Z lichun $
//
//

#include <string.h>
#include <stdlib.h>
#include "Network/Guid.h"

#define GUIDFORMATSTR	"%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x"

#ifdef WIN32
	#include <objbase.h>
#else
	#include "uuid/uuid.h"
typedef struct _GUID {
	unsigned int  Data1;
	unsigned short Data2;
	unsigned short Data3;
	unsigned char  Data4[ 8 ];
} GUID;

#define RPC_CSTR const char*
#define RPC_S_OK true
bool UuidFromString(const char* str,GUID* guid)
{
	unsigned int data[11];

	if(sscanf(str,GUIDFORMATSTR,&data[0],&data[1],
		&data[2],&data[3],&data[4],&data[5],&data[6],&data[7],&data[8],&data[9],&data[10]) != 11)
	{
		return false;
	}
	
	guid->Data1 = data[0];
	guid->Data2 = (unsigned short)data[1];
	guid->Data3 = (unsigned short)data[2];
	guid->Data4[0] = (unsigned char)data[3];
	guid->Data4[1] = (unsigned char)data[4];
	guid->Data4[2] = (unsigned char)data[5];
	guid->Data4[3] = (unsigned char)data[6];
	guid->Data4[4] = (unsigned char)data[7];
	guid->Data4[5] = (unsigned char)data[8];
	guid->Data4[6] = (unsigned char)data[9];
	guid->Data4[7] = (unsigned char)data[10];

	return true;
}
#endif


namespace Xunmei{
namespace Network{


Guid *Guid::createGuidEx()
{
	GUID guid;
#ifdef WIN32	
	CoCreateGuid(&guid);
#else
	uuid_t uu;
	uuid_generate(uu);
	memcpy(&guid, uu, 16);
#endif
	Guid *p = new Guid((char *)&guid);

	return p;
}

Guid Guid::createGuid()
{
	GUID guid;
#ifdef WIN32	
	CoCreateGuid(&guid);	
#else
	uuid_t uu;
	uuid_generate(uu);
	memcpy(&guid, uu, 16);
#endif
	Guid p((char *)&guid);
	return p;
}

Guid::Guid()
{
	::memset(buffer, 0, 16);
}
	/// 构造函数
Guid::Guid(const char *guid, bool isstream)
{
	setGuid(guid, isstream);
}

/// 拷贝构造函数
Guid::Guid(const Guid& other)
{
	::memcpy(buffer,(const char *)other.buffer, 16);
}


Guid& Guid::operator=(const Guid& other) 
{
	::memcpy(buffer,(const char *)other.buffer, 16);
	return *this;
}


/// 比较
bool  Guid::operator<(const Guid& other) const
{
	return (::memcmp(buffer, other.buffer,16) < 0);
}

/// 等于
bool  Guid::operator==( const Guid &other) const
{
	if (::memcmp(buffer, other.buffer, 16) == 0)
		return true;
	return false;
	
}

/// 等于
bool Guid::operator==(const char * other) const
{
	if (::memcmp(buffer, other, 16) != 0)
		return false;
	return true;
}
	

bool Guid::operator!=(const Guid &other) const
{
	if (::memcmp(buffer, other.buffer, 16) != 0)
	{
		return true;
	}
	return false;	
}
	/// 不等于 流串
bool Guid::operator!=(const char * other) const
{
	if (::memcmp(buffer, other, 16) != 0)
		return true;
	return false;

}

/// 析构函数
Guid::~Guid()
{
	
}

bool Guid::valid() const
{
	return  ((uint64_t)(*(buffer)) != 0 || (uint64_t)(*(buffer + 8)) != 0);
}
	
bool Guid::setGuid(const char *guid, bool isstream)
{
	if (isstream)
	{
		::memcpy(buffer, guid, 16);
		return true;
	}
	else
	{
		GUID guid_;
		if (UuidFromString((RPC_CSTR)guid, &guid_) != RPC_S_OK)
		{
			//logwarn("Parse setGuid Fail\n");
			return false;
		}
		else
		{
			::memcpy(buffer, &guid_, 16);
			return true;		
		}
	}
}


const unsigned char *Guid::getBitStream() const
{
	return buffer;

}

std::string Guid::getStringStream() const
{
	char buf[37];
	GUID *guid = (GUID*)(buffer);
	snprintf_x(buf, 37, GUIDFORMATSTR, 
		guid->Data1, guid->Data2, guid->Data3, guid->Data4[0],guid->Data4[1], 
		guid->Data4[2], guid->Data4[3], guid->Data4[4], guid->Data4[5], guid->Data4[6],guid->Data4[7]);
	return std::string(buf);

}
bool Guid::getStringStream(char *str) const
{
	GUID *guid = (GUID*)buffer;
	snprintf_x(str, 37, GUIDFORMATSTR, 
		guid->Data1, guid->Data2, guid->Data3, guid->Data4[0],guid->Data4[1], 
		guid->Data4[2], guid->Data4[3], guid->Data4[4], guid->Data4[5], guid->Data4[6],guid->Data4[7]);
	return true;
}
	

} // namespace Base
} // namespace Xunmei



