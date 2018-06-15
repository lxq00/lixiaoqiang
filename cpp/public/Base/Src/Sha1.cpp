//
//  Copyright (c)1998-2012, Public Technology
//  All Rights Reserved.
//
//	Description:
//	$Id: Md5.h 3 2013-01-21 06:57:38Z  $
//
#include "Base/Sha1.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "Base/String.h"

namespace Public {
namespace Base {

#if !defined(SHA1_LITTLE_ENDIAN) && !defined(SHA1_BIG_ENDIAN)
#define SHA1_LITTLE_ENDIAN
#endif

#if !defined(SHA1_WIPE_VARIABLES) && !defined(SHA1_NO_WIPE_VARIABLES)
#define SHA1_WIPE_VARIABLES
#endif

#define SHA1_MAX_FILE_BUFFER (32 * 20 * 820)

	// Rotate p_val32 by p_nBits bits to the left
#ifndef ROL32
#ifdef _MSC_VER
#define ROL32(p_val32,p_nBits) _rotl(p_val32,p_nBits)
#else
#define ROL32(p_val32,p_nBits) (((p_val32)<<(p_nBits))|((p_val32)>>(32-(p_nBits))))
#endif
#endif

#ifdef SHA1_LITTLE_ENDIAN
#define SHABLK0(i) (m_block->l[i] = \
	(ROL32(m_block->l[i],24) & 0xFF00FF00) | (ROL32(m_block->l[i],8) & 0x00FF00FF))
#else
#define SHABLK0(i) (m_block->l[i])
#endif

#define SHABLK(i) (m_block->l[i&15] = ROL32(m_block->l[(i+13)&15] ^ \
	m_block->l[(i+8)&15] ^ m_block->l[(i+2)&15] ^ m_block->l[i&15],1))

	// SHA-1 rounds
#define S_R0(v,w,x,y,z,i) {z+=((w&(x^y))^y)+SHABLK0(i)+0x5A827999+ROL32(v,5);w=ROL32(w,30);}
#define S_R1(v,w,x,y,z,i) {z+=((w&(x^y))^y)+SHABLK(i)+0x5A827999+ROL32(v,5);w=ROL32(w,30);}
#define S_R2(v,w,x,y,z,i) {z+=(w^x^y)+SHABLK(i)+0x6ED9EBA1+ROL32(v,5);w=ROL32(w,30);}
#define S_R3(v,w,x,y,z,i) {z+=(((w|x)&y)|(w&x))+SHABLK(i)+0x8F1BBCDC+ROL32(v,5);w=ROL32(w,30);}
#define S_R4(v,w,x,y,z,i) {z+=(w^x^y)+SHABLK(i)+0xCA62C1D6+ROL32(v,5);w=ROL32(w,30);}

class Sha1::Sha1Internal
{
public:
	Sha1Internal()
	{
		m_block = (SHA1_WORKSPACE_BLOCK*)m_workspace;

		Reset();
	}
	~Sha1Internal(){Reset();}
	void Reset()
	{
		m_state[0] = 0x67452301;
		m_state[1] = 0xEFCDAB89;
		m_state[2] = 0x98BADCFE;
		m_state[3] = 0x10325476;
		m_state[4] = 0xC3D2E1F0;

		m_count[0] = 0;
		m_count[1] = 0;
	}
	void Transform(unsigned int* pState, const unsigned char* pBuffer)
	{
		unsigned int a = pState[0], b = pState[1], c = pState[2], d = pState[3], e = pState[4];

		memcpy(m_block, pBuffer, 64);

		// 4 rounds of 20 operations each, loop unrolled
		S_R0(a,b,c,d,e, 0); S_R0(e,a,b,c,d, 1); S_R0(d,e,a,b,c, 2); S_R0(c,d,e,a,b, 3);
		S_R0(b,c,d,e,a, 4); S_R0(a,b,c,d,e, 5); S_R0(e,a,b,c,d, 6); S_R0(d,e,a,b,c, 7);
		S_R0(c,d,e,a,b, 8); S_R0(b,c,d,e,a, 9); S_R0(a,b,c,d,e,10); S_R0(e,a,b,c,d,11);
		S_R0(d,e,a,b,c,12); S_R0(c,d,e,a,b,13); S_R0(b,c,d,e,a,14); S_R0(a,b,c,d,e,15);
		S_R1(e,a,b,c,d,16); S_R1(d,e,a,b,c,17); S_R1(c,d,e,a,b,18); S_R1(b,c,d,e,a,19);
		S_R2(a,b,c,d,e,20); S_R2(e,a,b,c,d,21); S_R2(d,e,a,b,c,22); S_R2(c,d,e,a,b,23);
		S_R2(b,c,d,e,a,24); S_R2(a,b,c,d,e,25); S_R2(e,a,b,c,d,26); S_R2(d,e,a,b,c,27);
		S_R2(c,d,e,a,b,28); S_R2(b,c,d,e,a,29); S_R2(a,b,c,d,e,30); S_R2(e,a,b,c,d,31);
		S_R2(d,e,a,b,c,32); S_R2(c,d,e,a,b,33); S_R2(b,c,d,e,a,34); S_R2(a,b,c,d,e,35);
		S_R2(e,a,b,c,d,36); S_R2(d,e,a,b,c,37); S_R2(c,d,e,a,b,38); S_R2(b,c,d,e,a,39);
		S_R3(a,b,c,d,e,40); S_R3(e,a,b,c,d,41); S_R3(d,e,a,b,c,42); S_R3(c,d,e,a,b,43);
		S_R3(b,c,d,e,a,44); S_R3(a,b,c,d,e,45); S_R3(e,a,b,c,d,46); S_R3(d,e,a,b,c,47);
		S_R3(c,d,e,a,b,48); S_R3(b,c,d,e,a,49); S_R3(a,b,c,d,e,50); S_R3(e,a,b,c,d,51);
		S_R3(d,e,a,b,c,52); S_R3(c,d,e,a,b,53); S_R3(b,c,d,e,a,54); S_R3(a,b,c,d,e,55);
		S_R3(e,a,b,c,d,56); S_R3(d,e,a,b,c,57); S_R3(c,d,e,a,b,58); S_R3(b,c,d,e,a,59);
		S_R4(a,b,c,d,e,60); S_R4(e,a,b,c,d,61); S_R4(d,e,a,b,c,62); S_R4(c,d,e,a,b,63);
		S_R4(b,c,d,e,a,64); S_R4(a,b,c,d,e,65); S_R4(e,a,b,c,d,66); S_R4(d,e,a,b,c,67);
		S_R4(c,d,e,a,b,68); S_R4(b,c,d,e,a,69); S_R4(a,b,c,d,e,70); S_R4(e,a,b,c,d,71);
		S_R4(d,e,a,b,c,72); S_R4(c,d,e,a,b,73); S_R4(b,c,d,e,a,74); S_R4(a,b,c,d,e,75);
		S_R4(e,a,b,c,d,76); S_R4(d,e,a,b,c,77); S_R4(c,d,e,a,b,78); S_R4(b,c,d,e,a,79);

		// Add the working vars back into state
		pState[0] += a;
		pState[1] += b;
		pState[2] += c;
		pState[3] += d;
		pState[4] += e;

		// Wipe variables
#ifdef SHA1_WIPE_VARIABLES
		a = b = c = d = e = 0;
#endif
	}
	void Update(const unsigned char* pbData, unsigned int uLen)
	{
		unsigned int j = ((m_count[0] >> 3) & 0x3F);

		if((m_count[0] += (uLen << 3)) < (uLen << 3))
			++m_count[1]; // Overflow

		m_count[1] += (uLen >> 29);

		unsigned int i;
		if((j + uLen) > 63)
		{
			i = 64 - j;
			memcpy(&m_buffer[j], pbData, i);
			Transform(m_state, m_buffer);

			for( ; (i + 63) < uLen; i += 64)
				Transform(m_state, &pbData[i]);

			j = 0;
		}
		else i = 0;

		if((uLen - i) != 0)
			memcpy(&m_buffer[j], &pbData[i], uLen - i);
	}
	void Final()
	{
		unsigned int i;

		unsigned char pbFinalCount[8];
		for(i = 0; i < 8; ++i)
			pbFinalCount[i] = static_cast<unsigned char>((m_count[((i >= 4) ? 0 : 1)] >>
			((3 - (i & 3)) * 8) ) & 0xFF); // Endian independent

		Update((unsigned char*)"\200", 1);

		while((m_count[0] & 504) != 448)
			Update((unsigned char*)"\0", 1);

		Update(pbFinalCount, 8); // Cause a Transform()

		for(i = 0; i < 20; ++i)
			m_digest[i] = static_cast<unsigned char>((m_state[i >> 2] >> ((3 -
			(i & 3)) * 8)) & 0xFF);

		// Wipe variables for security reasons
#ifdef SHA1_WIPE_VARIABLES
		memset(m_buffer, 0, 64);
		memset(m_state, 0, 20);
		memset(m_count, 0, 8);
		memset(pbFinalCount, 0, 8);
		Transform(m_state, m_buffer);
#endif
	}
	bool GetHash(unsigned char* pbDest20) const
	{
		if(pbDest20 == NULL) return false;
		memcpy(pbDest20, m_digest, 20);
		return true;
	}
	bool ReportHash(char* tszReport, REPORT_TYPE rtReportType) const
	{
		if(tszReport == NULL) return false;

		char tszTemp[16];

		if (rtReportType == REPORT_BIN)
		{
			memcpy(tszReport, m_digest, 20);
		}
		else if((rtReportType == REPORT_HEX) || (rtReportType == REPORT_HEX_SHORT))
		{
			sprintf(tszTemp, "%02X", m_digest[0]);
			strcpy(tszReport, tszTemp);

			const char* lpFmt = ((rtReportType == REPORT_HEX) ? " %02X" : "%02X");
			for(size_t i = 1; i < 20; ++i)
			{
				sprintf(tszTemp, lpFmt, m_digest[i]);
				strcat(tszReport, tszTemp);
			}
		}
		else if(rtReportType == REPORT_DIGIT)
		{
			sprintf(tszTemp, "%u", m_digest[0]);
			strcpy(tszReport, tszTemp);

			for(size_t i = 1; i < 20; ++i)
			{
				sprintf(tszTemp, " %u", m_digest[i]);
				strcat(tszReport, tszTemp);
			}
		}
		else
		{
			return false;
		}

		return true;
	}
private:
	typedef union
	{
		unsigned char c[64];
		unsigned int l[16];
	} SHA1_WORKSPACE_BLOCK;
private:
	unsigned int m_state[5];
	unsigned int m_count[2];
	unsigned int m_reserved0[1];
	unsigned char m_buffer[64];
	unsigned char m_digest[20];
	unsigned int m_reserved1[3];

	unsigned char m_workspace[64];
	SHA1_WORKSPACE_BLOCK* m_block;
};

Sha1::Sha1()
{
	internal = new Sha1Internal();
}
Sha1::~Sha1()
{
	delete internal;
}
std::string Sha1::hashFile(const std::string& file, REPORT_TYPE type)
{
	FILE* fpIn = fopen(file.c_str(), "rb");
	if(fpIn == NULL)
	{
		return "";
	}
	Sha1 sa1;

	char buffer[1024];
	while (true)
	{
		int readlen = fread(buffer, 1, 1024, fpIn);
		if (readlen <= 0)
		{
			break;
		}

		sa1.input(buffer, readlen);
	}
	fclose(fpIn);


	return sa1.report(type);
}
bool Sha1::input(char const* data, size_t size)
{
	if(data == NULL || size == 0)
	{
		return false;
	}

	internal->Update((const unsigned char*)data,size);

	return true;
}


std::string Sha1::report(REPORT_TYPE type)
{
	internal->Final();

	char tszOut[84] = { 0 };
	const bool bResult = internal->ReportHash(tszOut, type);
	if (!bResult)
	{
		return "";
	}
	return tszOut;
}

} // namespace Base
} // namespace Public



