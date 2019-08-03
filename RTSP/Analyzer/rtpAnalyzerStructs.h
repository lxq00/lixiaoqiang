#pragma once

#include "Base/Base.h"
using namespace Public::Base;


/*
+---------------+
|0|1|2|3|4|5|6|7|
+-+-+-+-+-+-+-+-+
|F|NRI|  Type   |
+---------------+
*/
typedef struct _FU_INDICATOR
{
	//byte 0
	unsigned char TYPE:5;
	unsigned char NRI:2; 
	unsigned char F:1;

} FU_INDICATOR; // 1 BYTE 

/*
+---------------+
|0|1|2|3|4|5|6|7|
+-+-+-+-+-+-+-+-+
|S|E|R|  Type   |
+---------------+
*/
typedef struct _FU_HEADER
{
	unsigned char TYPE:5;
	unsigned char R:1;
	unsigned char E:1;
	unsigned char S:1;

} FU_HEADER;   // 1 BYTES

/*
+---------------+---------------+---------------+---------------+
|0|1|2|3|4|5|6|7|0|1|2|3|4|5|6|7|0|1|2|3|4|5|6|7|0|1|2|3|4|5|6|7|
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|    类型特定   |	                分段偏移      				|
+---------------+---------------+---------------+---------------+
|		类型    |       Q       |       宽		|       高      |
+---------------+---------------+---------------+---------------+
*/
typedef struct _JPEGHEADER 
{
	unsigned int tspec:8;
	unsigned int offset:24;
	uint8_t type;
	uint8_t q;
	uint8_t width;
	uint8_t height;

}JPEGHEADER, *LPJPEGHEADER;
