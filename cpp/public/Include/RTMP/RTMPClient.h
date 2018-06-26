#pragma once

#include "Defs.h"
#include "Base/Base.h"
using namespace Public::Base;

namespace Public {
namespace RTMP {

class RTMP_API RTMPClient
{
public:
	struct RTMP_API PacketInfo
	{
	private:
		PacketInfo(char* data,int size,uint32_t timestmap,int type);
	public:
		~PacketInfo();

	public:
		char*		data;
		int			size;

		uint32_t	timestamp;/*ms*/
		RTMPType	type;
		uint32_t	pts;

		union _Info
		{
			struct _VideoInfo {
				enum {
					CodeId_H263,
					CodeId_Screen,
					CodeId_VP6,
					CodeId_VP6Alpath,
					CodeId_Screen2,
					CodeId_H264,
					CodeId_Unknown,
				}codeId;

				enum {
					AVCType_SpsPps,
					AVCType_Nalu,
					AVCType_SpsPpsEnd,
					AVCType_Unknown,
				}avcType;

				enum {
					FrameType_I,
					FrameType_P_B,
					FrameType_DI,
					FrameType_GI,
					FrameType_VI,
					FrameType_Unknown,
				}frameType;
			}video;
			struct _AudioInfo{
				enum {
					Format_LinearPCM,
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
					Format_Unknown,
				}format;
				enum {
					Rate_5_5KHz,
					Rate_11KHz,
					Rate_22KHz,
					Rate_44KHz,
					Rate_Unknown,
				}rate;
				enum {
					Size_8bit,
					Size_16bit,
					Size_Unknown,
				}size;
				enum {
					Channel_Mono,
					Channel_Stereo,
					Channel_Unknown,
				}channel;
				enum {
					AACType_SH,
					AACType_Raw,
					AACType_Unknown,
				}aacType;
			}audio;
		}info;
	};
public:
	RTMPClient(const std::string& url);
	~RTMPClient();

	//Á¬½Ó
	bool connect();

	shared_ptr<PacketInfo> readPacket();
private:
	struct RTMPClientInternal;
	RTMPClientInternal *internal;
};

}
}
