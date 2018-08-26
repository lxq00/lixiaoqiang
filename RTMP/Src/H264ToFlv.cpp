#include "RTMP/FlvParser.h"
#include "libflv/flv-muxer.h"
namespace Public {
namespace RTMP {

#define MAXH264BUFFERLEN		1024*1024*2

struct H264ToFlv::H264ToFlvInternal
{
	flv_muxer_t* flv;
	char*		 buffer;
	int			 bufferlen;
	FlvCallback	 callback;
	uint32_t	timestmap;

	static int on_flv_packet(void* param, int type, const void* data, size_t bytes, uint32_t timestamp)
	{
		H264ToFlvInternal* internal = (H264ToFlvInternal*)param;

		PacketInfo info;
		info.type = (RTMPType)type;
		info.timestamp = timestamp;
		info.size = bytes;
		info.data = (char*)data;

		internal->callback(info);

		return 1;
	}

	static const char* search_start_code(const char* ptr, const char* end)
	{
		for (const char *p = ptr; p + 3 < end; p++)
		{
			char a = p[0];
			char b = p[1];
			char c = p[2];
			char d = p[3];
			if (0x00 == (p[0]&0xff) && 0x00 == (p[1] & 0xff) && (0x01 == (p[2] & 0xff) || (0x00 == (p[2] & 0xff) && 0x01 == (p[3] & 0xff))))
				return p;
		}
		return end;
	}
};

H264ToFlv::H264ToFlv(const FlvCallback& callback)
{
	internal = new H264ToFlvInternal;
	internal->bufferlen = 0;
	internal->buffer = new char[MAXH264BUFFERLEN];
	internal->flv = flv_muxer_create(H264ToFlvInternal::on_flv_packet, internal);
	internal->callback = callback;
	internal->timestmap = 0;
}
H264ToFlv::~H264ToFlv()
{
	flv_muxer_destroy(internal->flv);
	internal->flv = NULL;
	SAFE_DELETEARRAY(internal->buffer);
	
	SAFE_DELETE(internal);
}

void H264ToFlv::clean()
{
	internal->bufferlen = 0;
}
void H264ToFlv::input(const char* buffer, int len)
{
	while (len > 0)
	{
		int copylen = min(len, MAXH264BUFFERLEN - internal->bufferlen);
		if (copylen == 0)
		{
			assert(0);
			break;
		}
		memcpy(internal->buffer + internal->bufferlen, buffer, copylen);
		internal->bufferlen += copylen;
		len -= copylen;
		buffer += copylen;

		const char* end = internal->buffer + internal->bufferlen;
		const char* nalu = H264ToFlvInternal::search_start_code(internal->buffer, end);
		if (nalu < end)
		{
			const char* next = H264ToFlvInternal::search_start_code(nalu + 4, end);
			while (next < end)
			{
				nalu += 1 == (nalu[2]&0xff) ? 3 : 4;
				flv_muxer_h264_nalu(internal->flv, nalu, next - nalu, internal->timestmap, internal->timestmap);
				uint8_t type = *nalu & 0x1f;
				if (type > 0 && type < 6)
				{
					internal->timestmap += 35;
				}

				nalu = next;
				next = H264ToFlvInternal::search_start_code(nalu + 4, end);
			}
		}
		internal->bufferlen = end - nalu;

		if (internal->bufferlen > 0 && nalu != internal->buffer)
		{
			memmove(internal->buffer, nalu, internal->bufferlen);
		}
	}
}

}
}