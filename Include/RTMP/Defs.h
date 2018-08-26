#pragma once

#ifdef WIN32
	#ifdef RTMP_EXPORTS
		#define RTMP_API __declspec(dllexport)
	#else
		#define RTMP_API __declspec(dllimport)
	#endif
#else
	#define RTMP_API
#endif

#include "Base/Base.h"
using namespace Public::Base;

namespace Public{
namespace RTMP{
typedef enum 
{
	FlvTagAudio = 8,
	FlvTagVideo = 9,
	FlvTagScript = 18,
}RTMPType;

#define DEFAULTRTMPPORT		1935

struct RTMP_API PacketInfo
{
public:
	char*		data; //flv Êý¾Ý
	int			size;

	uint32_t	timestamp;/*ms*/
	RTMPType	type;
};
union RTMP_API FlvInfo
{
	struct _VideoInfo {
		enum CodeId {
			CodeId_Jpeg = 1,
			CodeId_H263 = 2,
			CodeId_Screen= 3,
			CodeId_VP6= 4,
			CodeId_VP6Alpath=5,
			CodeId_Screen2=6,
			CodeId_H264 = 7,
			CodeId_H265 = 12 // https://github.com/CDN-Union/H265
		}codeId;

		enum FrameType {
			FrameType_KeyFrame = 1,
			FrameType_IFrame = 2,
			FrameType_DIFrame = 3,
			FrameType_GKeyFrame = 4,
			FrameType_InfoFrame = 5,
			FrameType_Unknown,
		}frameType;
	}video;
	struct _AudioInfo {
		enum Format {
			Format_LinearPCM = 0,
			Format_ADPCM,
			Format_MP3,
			Format_LinearPCMLe,
			Format_NellymoserKHz16,
			Format_NellymoserKHz8,
			Format_Nellymoser,
			Format_G711APCM,
			Format_G711MuPCM,
			Format_Reserved,
			Format_AAC,
			Format_Speex,
			Format_MP3KHz8,
			Format_DeviceSpecific,
		}format;
		enum Rate {
			Rate_5_5KHz = 0,
			Rate_11KHz,
			Rate_22KHz,
			Rate_44KHz,
		}rate;
		enum Size {
			Size_8bit = 0,
			Size_16bit,
		}size;
		enum Channel {
			Channel_Mono = 0,
			Channel_Stereo,
		}channel;
		enum AACType {
			AACType_SH,
			AACType_Raw,
			AACType_Unknown,
		}aacType;
	}audio;
};
}
}