#include "RTMP/FlvParser.h"
namespace Public {
namespace RTMP {


#define MAXFLVBUFFERSIZE		1024*1024*2

struct FlvParser::FlvParserInternal
{
	char*			buffer;
	int				bufferLen;
	int				bufferMaxLen;

	FlvCallback		callback;

	bool			notParseHeader;

	static uint32_t be_read_uint32(const char* ptr)
	{
		return ((ptr[0] & 0xff) << 24) | ((ptr[1] & 0xff) << 16) | ((ptr[2] & 0xff) << 8) | (ptr[3] & 0xff);
	}
};

FlvParser::FlvParser(const FlvCallback& callback)
{
	internal = new FlvParserInternal();
	internal->buffer = new char[MAXFLVBUFFERSIZE];
	internal->bufferMaxLen = MAXFLVBUFFERSIZE;
	internal->bufferLen = 0;
	internal->notParseHeader = true;
	internal->callback = callback;
}
FlvParser::~FlvParser()
{
	delete[]internal->buffer;
	SAFE_DELETE(internal);
}

void FlvParser::clean()
{
	internal->bufferLen = 0;
}
void FlvParser::input(const char* buffer, int len)
{
	while (len > 0)
	{
		int copylen = min(len, internal->bufferMaxLen - internal->bufferLen);
		if (copylen == 0) 
		{
			assert(0);
			break;
		}
		memcpy(internal->buffer + internal->bufferLen, buffer, copylen);
		internal->bufferLen += copylen;
		len -= copylen;
		buffer += copylen;

		const char* buffertmp = internal->buffer;
		int bufferlen = internal->bufferLen;
		
		while (bufferlen > 0)
		{
#define TARGETHEADERLEN	4
			if (internal->notParseHeader)
			{
#define FLVFILEHEADERLEN	9

				if (bufferlen < FLVFILEHEADERLEN + TARGETHEADERLEN) break;

				if ('F' != buffertmp[0] || 'L' != buffertmp[1] || 'V' != buffertmp[2])
					break;

				assert(0x00 == (buffertmp[4] & 0xF8) && 0x00 == (buffertmp[4] & 0x20));
				int offset = FlvParserInternal::be_read_uint32(buffertmp + 5);

				assert(offset >= FLVFILEHEADERLEN);
				if (bufferlen < offset) break;

				assert(FlvParserInternal::be_read_uint32(buffertmp + offset) == 0);

				bufferlen -= offset + TARGETHEADERLEN;
				buffertmp += offset + TARGETHEADERLEN;

				internal->notParseHeader = false;
			}
			else
			{
#define MAXTARGETLEN	11
				if (bufferlen < MAXTARGETLEN + TARGETHEADERLEN)
				{
					break;
				}
				int type = buffertmp[0] & 0xff;
				int len = ((buffertmp[1] & 0xff) << 16) | ((buffertmp[2]&0xff) << 8) | (buffertmp[3]&0xff);
				int timestmap = ((buffertmp[4] & 0xff) << 16) | ((buffertmp[5] & 0xff) << 8) | (buffertmp[6] & 0xff) | ((buffertmp[7] & 0xff) << 24);

				if (bufferlen < len + MAXTARGETLEN + TARGETHEADERLEN)
				{
					break;
				}
				PacketInfo info;
				info.type = (RTMPType)type;
				info.timestamp = timestmap;
				info.size = len; 
				info.data = (char*)(buffertmp + MAXTARGETLEN);

				internal->callback(info);

				assert(FlvParserInternal::be_read_uint32(buffertmp + len + MAXTARGETLEN) == len + MAXTARGETLEN);

				bufferlen -= len + MAXTARGETLEN + TARGETHEADERLEN;
				buffertmp += len + MAXTARGETLEN + TARGETHEADERLEN;
			}
		}

		if (bufferlen > 0 && buffertmp != internal->buffer)
		{
			memmove(internal->buffer, buffertmp, bufferlen);
		}
		internal->bufferLen = bufferlen;
	}
}

bool FlvParser::parseFlv(const PacketInfo& pkg, FlvInfo& info)
{
	if (pkg.type == FlvTagScript) return true;
	else if (pkg.type == FlvTagAudio)
	{
		info.audio.format = (FlvInfo::_AudioInfo::Format)(pkg.data[0] & 0xF0);
		info.audio.rate = (FlvInfo::_AudioInfo::Rate)((pkg.data[0] & 0x0C) >> 2);
		info.audio.size = (FlvInfo::_AudioInfo::Size)((pkg.data[0] & 0x02) >> 1);
		info.audio.channel = (FlvInfo::_AudioInfo::Channel)(pkg.data[0] & 0x01);
	}
	else if (pkg.type == FlvTagVideo)
	{
		info.video.frameType = (FlvInfo::_VideoInfo::FrameType)((pkg.data[0] & 0xF0) >> 4);
		info.video.codeId = (FlvInfo::_VideoInfo::CodeId)(pkg.data[0] & 0x0F);
	}

	return true;
}
}
}