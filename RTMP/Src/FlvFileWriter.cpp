#include "RTMP/FlvParser.h"
#include "libflv/flv-writer.h"
namespace Public {
namespace RTMP {

struct FlvFileWriter::FlvFileWriterInternal
{
	FILE*			writefd;
	WriteCallback	callback;

	void writeCallback(const char* buffer, int len)
	{
		if(writefd != NULL)
			fwrite(buffer, 1, len, writefd);
	}
	static void be_write_uint32(uint8_t* ptr, uint32_t val)
	{
		ptr[0] = (uint8_t)((val >> 24) & 0xFF);
		ptr[1] = (uint8_t)((val >> 16) & 0xFF);
		ptr[2] = (uint8_t)((val >> 8) & 0xFF);
		ptr[3] = (uint8_t)(val & 0xFF);
	}
	static int flv_write_tag(uint8_t* tag, uint8_t type, uint32_t bytes, uint32_t timestamp)
	{
		// TagType
		tag[0] = type & 0x1F;

		// DataSize
		tag[1] = (bytes >> 16) & 0xFF;
		tag[2] = (bytes >> 8) & 0xFF;
		tag[3] = bytes & 0xFF;

		// Timestamp
		tag[4] = (timestamp >> 16) & 0xFF;
		tag[5] = (timestamp >> 8) & 0xFF;
		tag[6] = (timestamp >> 0) & 0xFF;
		tag[7] = (timestamp >> 24) & 0xFF; // Timestamp Extended

										   // StreamID(Always 0)
		tag[8] = 0;
		tag[9] = 0;
		tag[10] = 0;

		return 11;
	}
};

FlvFileWriter::FlvFileWriter()
{
	internal = new FlvFileWriterInternal();
	internal->writefd = NULL;
}
FlvFileWriter::~FlvFileWriter()
{
	close();
	SAFE_DELETE(internal);
}
bool FlvFileWriter::open(const std::string& filename)
{
	close();
	internal->writefd = fopen(filename.c_str(), "wb+");
	if (internal->writefd == NULL) return false;

	return open(WriteCallback(&FlvFileWriterInternal::writeCallback,internal));
}
bool FlvFileWriter::open(const WriteCallback& callback)
{
	internal->callback = callback;

	{
		uint8_t header[9 + 4];
		header[0] = 'F'; // FLV signature
		header[1] = 'L';
		header[2] = 'V';
		header[3] = 0x01; // File version
		header[4] = 0x05; // Type flags (audio & video)
		FlvFileWriterInternal::be_write_uint32(header + 5, 9); // Data offset
		FlvFileWriterInternal::be_write_uint32(header + 9, 0); // PreviousTagSize0(Always 0)

		internal->callback((const char*)header, sizeof(header));
	}

	return true;
}
bool FlvFileWriter::close()
{
	{
		uint8_t header[11 + 5 + 4];
		FlvFileWriterInternal::flv_write_tag(header, FlvTagVideo, 5, 0);
		header[11] = (1 << 4) /* FrameType */ | 7 /* AVC */;
		header[12] = 2; // AVC end of sequence
		header[13] = 0;
		header[14] = 0;
		header[15] = 0;
		FlvFileWriterInternal::be_write_uint32(header + 16, 16); // TAG size

		internal->callback((char*)header, sizeof(header));
	}
	if (internal->writefd != NULL)
	{
		fclose(internal->writefd);
		internal->writefd = NULL;
	}

	return true;
}

bool FlvFileWriter::write(RTMPType type, uint32_t timestmap /*ms*/, const char* buffer, int maxlen)
{
	uint8_t tag[11];
	FlvFileWriterInternal::flv_write_tag(tag, (uint8_t)type, (uint32_t)maxlen, timestmap);
	internal->callback((char*)tag, 11);
	
	internal->callback((char*)buffer, maxlen);

	FlvFileWriterInternal::be_write_uint32(tag, (uint32_t)maxlen + 11);
	internal->callback((char*)tag, 4);

	return true;
}

}
}