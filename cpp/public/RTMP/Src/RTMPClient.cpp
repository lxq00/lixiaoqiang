#include "RTMP/RTMPClient.h"
#include "Simple-RTMP-Server/trunk/src/libs/srs_librtmp.hpp"

namespace Public {
namespace RTMP {

struct RTMPClient::RTMPClientInternal
{
	srs_rtmp_t	rmp;
	bool		initflag;
};

RTMPClient::RTMPClient(const std::string& url)
{
	internal = new RTMPClientInternal;
	internal->rmp = srs_rtmp_create(url.c_str());
	internal->initflag = false;
}
RTMPClient::~RTMPClient()
{
	srs_rtmp_destroy(internal->rmp);
	internal->initflag = false;
	SAFE_DELETE(internal);
}

bool RTMPClient::connect()
{
	if (srs_rtmp_handshake(internal->rmp) != 0) {
		return false;
	}
	if (srs_rtmp_connect_app(internal->rmp) != 0) {
		return false;
	}
	if (srs_rtmp_play_stream(internal->rmp) != 0) {
		return false;
	}
	internal->initflag = true;

	return true;
}

shared_ptr<RTMPClient::PacketInfo> RTMPClient::readPacket()
{
	if (!internal->initflag)
	{
		return shared_ptr<RTMPClient::PacketInfo>();
	}

	int size = 0;
	char vtype = SRS_RTMP_TYPE_AUDIO;
	char* data = NULL;
	uint32_t timestamp;

	if (srs_rtmp_read_packet(internal->rmp, &vtype, &timestamp, &data, &size) != 0) {
		return false;
	}

	shared_ptr<RTMPClient::PacketInfo> info = make_shared<RTMPClient::PacketInfo>(data,size,timestamp, vtype);

	return info;
}

RTMPClient::PacketInfo::PacketInfo(char* _data, int _size, uint32_t _timestmap, int _type)
	:data(_data),size(_size),timestamp(_timestmap)
{
	srs_utils_parse_timestamp(timestamp, _type, _data, _size, &pts);

	{
		if (_type == SRS_RTMP_TYPE_AUDIO) type = FlvTagAudio;
		else if (_type == SRS_RTMP_TYPE_VIDEO) type = FlvTagVideo;
		else type = FlvTagScript;
	}
	if (_type == SRS_RTMP_TYPE_VIDEO)
	{
		{
			char codec_id = srs_utils_flv_video_codec_id(_data, _size);
			switch (codec_id) {
			case 2: info.video.codeId = PacketInfo::_Info::_VideoInfo::CodeId_H263; break;
			case 3: info.video.codeId = PacketInfo::_Info::_VideoInfo::CodeId_Screen; break;
			case 4: info.video.codeId = PacketInfo::_Info::_VideoInfo::CodeId_VP6; break;
			case 5: info.video.codeId = PacketInfo::_Info::_VideoInfo::CodeId_VP6Alpath; break;
			case 6: info.video.codeId = PacketInfo::_Info::_VideoInfo::CodeId_Screen2; break;
			case 7: info.video.codeId = PacketInfo::_Info::_VideoInfo::CodeId_H264; break;
			default: info.video.codeId = PacketInfo::_Info::_VideoInfo::CodeId_Unknown; break;
			}
		}
		{
			char avc_packet_type = srs_utils_flv_video_avc_packet_type(_data, _size);
			switch (avc_packet_type) {
			case 0: info.video.avcType = PacketInfo::_Info::_VideoInfo::AVCType_SpsPps; break;
			case 1: info.video.avcType = PacketInfo::_Info::_VideoInfo::AVCType_Nalu; break;
			case 2: info.video.avcType = PacketInfo::_Info::_VideoInfo::AVCType_SpsPpsEnd; break;
			default: info.video.avcType = PacketInfo::_Info::_VideoInfo::AVCType_Unknown; break;
			}
		}
		{
			char frame_type = srs_utils_flv_video_frame_type(_data, _size);
			switch (frame_type) {
			case 1: info.video.frameType = PacketInfo::_Info::_VideoInfo::FrameType_I; break;
			case 2: info.video.frameType = PacketInfo::_Info::_VideoInfo::FrameType_P_B; break;
			case 3: info.video.frameType = PacketInfo::_Info::_VideoInfo::FrameType_DI; break;
			case 4: info.video.frameType = PacketInfo::_Info::_VideoInfo::FrameType_GI; break;
			case 5: info.video.frameType = PacketInfo::_Info::_VideoInfo::FrameType_VI; break;
			default: info.video.frameType = PacketInfo::_Info::_VideoInfo::FrameType_Unknown; break;
			}
		}
	}
	else if(_type == SRS_RTMP_TYPE_AUDIO)
	{
		{
			char sound_format = srs_utils_flv_audio_sound_format(_data, _size);
			switch (sound_format) {
			case 0: info.audio.format = PacketInfo::_Info::_AudioInfo::Format_LinearPCM; break;
			case 1: info.audio.format = PacketInfo::_Info::_AudioInfo::Format_ADPCM; break;
			case 2: info.audio.format = PacketInfo::_Info::_AudioInfo::Format_MP3; break;
			case 3: info.audio.format = PacketInfo::_Info::_AudioInfo::Format_LinearPCMLe; break;
			case 4: info.audio.format = PacketInfo::_Info::_AudioInfo::Format_NellymoserKHz16; break;
			case 5: info.audio.format = PacketInfo::_Info::_AudioInfo::Format_NellymoserKHz8; break;
			case 6: info.audio.format = PacketInfo::_Info::_AudioInfo::Format_Nellymoser; break;
			case 7: info.audio.format = PacketInfo::_Info::_AudioInfo::Format_G711APCM; break;
			case 8: info.audio.format = PacketInfo::_Info::_AudioInfo::Format_G711MuPCM; break;
			case 9: info.audio.format = PacketInfo::_Info::_AudioInfo::Format_Reserved; break;
			case 10: info.audio.format = PacketInfo::_Info::_AudioInfo::Format_AAC; break;
			case 11: info.audio.format = PacketInfo::_Info::_AudioInfo::Format_Speex; break;
			case 14: info.audio.format = PacketInfo::_Info::_AudioInfo::Format_MP3KHz8; break;
			case 15: info.audio.format = PacketInfo::_Info::_AudioInfo::Format_DeviceSpecific; break;
			default: info.audio.format = PacketInfo::_Info::_AudioInfo::Format_Unknown; break;
			}
		}

		{
			char sound_rate = srs_utils_flv_audio_sound_rate(_data, _size);
			switch (sound_rate) {
			case 0: info.audio.rate = PacketInfo::_Info::_AudioInfo::Rate_5_5KHz; break;
			case 1: info.audio.rate = PacketInfo::_Info::_AudioInfo::Rate_11KHz; break;
			case 2: info.audio.rate = PacketInfo::_Info::_AudioInfo::Rate_22KHz; break;
			case 3: info.audio.rate = PacketInfo::_Info::_AudioInfo::Rate_44KHz; break;
			default: info.audio.rate = PacketInfo::_Info::_AudioInfo::Rate_Unknown; break;
			}
		}

		{
			char sound_size = srs_utils_flv_audio_sound_size(_data, _size);
			switch (sound_size) {
			case 0: info.audio.size = PacketInfo::_Info::_AudioInfo::Size_8bit; break;
			case 1: info.audio.size = PacketInfo::_Info::_AudioInfo::Size_16bit; break;
			default: info.audio.size = PacketInfo::_Info::_AudioInfo::Size_Unknown; break;
			}
		}

		{
			char sound_type = srs_utils_flv_audio_sound_type(_data, _size);
			switch (sound_type) {
			case 0: info.audio.channel = PacketInfo::_Info::_AudioInfo::Channel_Mono; break;
			case 1: info.audio.channel = PacketInfo::_Info::_AudioInfo::Channel_Stereo; break;
			default: info.audio.channel = PacketInfo::_Info::_AudioInfo::Channel_Unknown; break;
			}
		}

		{
			char aac_packet_type = srs_utils_flv_audio_aac_packet_type(_data, _size);
			switch (aac_packet_type) {
			case 0: info.audio.aacType = PacketInfo::_Info::_AudioInfo::AACType_SH; break;
			case 1: info.audio.aacType = PacketInfo::_Info::_AudioInfo::AACType_Raw; break;
			default: info.audio.aacType = PacketInfo::_Info::_AudioInfo::AACType_Unknown; break;
			}
		}
	}
}
RTMPClient::PacketInfo::~PacketInfo()
{
	free(data);
}

}
}
