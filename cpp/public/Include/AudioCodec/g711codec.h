/*
 * G711 encode decode HEADER.
 */

#ifndef	__G711CODEC_H__
#define	__G711CODEC_H__

#ifdef WIN32
#define G711CODEC_CALLBACK __stdcall
#define G711CODEC_METHOD _cdecl
#else
#define G711CODEC_CALLBACK 
#define G711CODEC_METHOD 
#endif
#ifdef __cplusplus
extern "C"{
#endif
/*
* u-law, A-law and linear PCM conversions.
*/
#define	SIGN_BIT	(0x80)		/* Sign bit for a A-law byte. */
#define	QUANT_MASK	(0xf)		/* Quantization field mask. */
#define	NSEGS		(8)			/* Number of A-law segments. */
#define	SEG_SHIFT	(4)			/* Left shift for segment number. */
#define	SEG_MASK	(0x70)		/* Segment field mask. */
#define	BIAS		(0x84)		/* Bias for linear code. */

int G711CODEC_METHOD PCM2G711a( char *InAudioData, char *OutAudioData, int DataLen, int reserve );
int G711CODEC_METHOD PCM2G711u( char *InAudioData, char *OutAudioData, int DataLen, int reserve );

int G711CODEC_METHOD G711a2PCM( char *InAudioData, char *OutAudioData, int DataLen, int reserve );
int G711CODEC_METHOD G711u2PCM( char *InAudioData, char *OutAudioData, int DataLen, int reserve );

int G711CODEC_METHOD g711a_decode(short amp[], const unsigned char g711a_data[], int g711a_bytes);

int G711CODEC_METHOD g711u_decode(short amp[], const unsigned char g711u_data[], int g711u_bytes);

int G711CODEC_METHOD g711a_encode(unsigned char g711_data[], const short amp[], int len);

int G711CODEC_METHOD g711u_encode(unsigned char g711_data[], const short amp[], int len);

#ifdef __cplusplus
}
#endif
#endif  /* g711codec.h */
