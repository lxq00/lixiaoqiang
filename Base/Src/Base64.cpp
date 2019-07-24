//
//  Copyright (c)1998-2012,  Public Technology
//  All Rights Reserved.
//
//	Description:
//	$Id: Base64.cpp 11 2013-01-22 08:42:03Z  $
//
#include "Base/IntTypes.h"
#include "Base/Base64.h"
#include "Base/BaseTemplate.h"
namespace Public{
namespace Base {

const char base[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";

/* Base64 ±àÂë */ 
char *base64_encode(const char* data, size_t data_len) 
{     
	int prepare = 0;     
	size_t ret_len;
	size_t temp = 0;
	char *ret = NULL;     
	char *f = NULL;     
	int tmp = 0;     
	char changed[4];     
	int i = 0;     
	ret_len = data_len / 3;     
	temp = data_len % 3;     
	if (temp > 0)     
	{         
		ret_len += 1;     
	}     
	ret_len = ret_len*4 + 1;     
	ret = (char *)malloc(ret_len);           
	if ( ret == NULL)     
	{
		return NULL;
	}     
	memset(ret, 0, ret_len);    
	f = ret;     
	while (tmp < data_len)     
	{        
		temp = 0;         
		prepare = 0;         
		memset(changed, '\0', 4);         
		while (temp < 3)         
		{
			//printf("tmp = %d\n", tmp);             
			if (tmp >= data_len)             
			{                 
				break;             
			}            
			prepare = ((prepare << 8) | (data[tmp] & 0xFF));             
			tmp++;             
			temp++;         
		}         
		prepare = (prepare<<((3-temp)*8));         
		//printf("before for : temp = %d, prepare = %d\n", temp, prepare);         
		for (i = 0; i < 4 ;i++ )         
		{             
			if (temp < i)             
			{                 
				changed[i] = 0x40;             
			}             
			else             
			{                 
				changed[i] = (prepare>>((3-i)*6)) & 0x3F;             
			}             
			*f = base[(int)changed[i]];             
			//printf("%.2X", changed[i]);             
			f++;         
		}     
	}     
	*f = '\0';           
	return ret;       
}
/* ×ª»»Ëã×Ó */ 
static char find_pos(char ch)   
{     
	char *ptr = (char*)strrchr(base, ch);
	//the last position (the only) in base[]     
	return (char)(ptr - base); 
}  
/* Base64 ½âÂë */ 
char *base64_decode(const char *data, size_t data_len, size_t&len)
{     
	size_t ret_len = (data_len / 4) * 3;
	int equal_count = 0;     
	char *ret = NULL;     
	char *f = NULL;     
	int tmp = 0;     
	int temp = 0;    
	int prepare = 0;     
	int i = 0;     
	if (*(data + data_len - 1) == '=')     
	{         
		equal_count += 1;     
	}     
	if (*(data + data_len - 2) == '=')     
	{         
		equal_count += 1;     
	}     
	if (*(data + data_len - 3) == '=')     
	{
		//seems impossible         
		equal_count += 1;     
	}     
	switch (equal_count)     
	{     
	case 0:         
		ret_len += 4;
		//3 + 1 [1 for NULL]         
	break;     
	case 1:         
		ret_len += 4;
		//Ceil((6*3)/8)+1         
	break;     
	case 2:         
		ret_len += 3;
		//Ceil((6*2)/8)+1         
	break;     
	case 3:         
		ret_len += 2;
		//Ceil((6*1)/8)+1         
		break;     
	}     
	ret = (char *)malloc(ret_len);     
	if (ret == NULL)     
	{         
		return NULL;
	}     
	memset(ret, 0, ret_len);     
	f = ret;     
	while (tmp < (data_len - equal_count))     
	{        
		temp = 0;         
		prepare = 0;         
		while (temp < 4)         
		{             
			if (tmp >= (data_len - equal_count))             
			{                 
				break;             
			}             
			prepare = (prepare << 6) | (find_pos(data[tmp]));             
			temp++;             
			tmp++;         
		}         
		prepare = prepare << ((4-temp) * 6);         
		for (i=0; i<3 ;i++ )         
		{             
			if (i == temp)             
			{                 
				break;             
			}             
			*f = (char)((prepare>>((2-i)*8)) & 0xFF);            
			f++;         
		}     
	}     
	*f = '\0';     
	len = (f - ret);
	return ret; 
}



/// base46 encode
std::string Base64::encode(const std::string& src)
{
	char* enstrbuf = base64_encode(src.c_str(), src.length());
	if (enstrbuf == NULL) return"";

	std::string destsrc(enstrbuf);
	SAFE_DELETEARRAY(enstrbuf);

	return Public::Base::move(destsrc);
}


/// base64 decode
std::string Base64::decode(const std::string& src)
{
	size_t len = 0;
	char* decstr = base64_decode(src.c_str(), src.length(), len);
	if (decstr == NULL) return "";

	std::string outputstr(decstr, len);
	SAFE_DELETEARRAY(decstr);

	return Public::Base::move(outputstr);
}


}//namespace Base
}//namespace Public

