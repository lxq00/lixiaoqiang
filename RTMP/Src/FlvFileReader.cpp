#include "RTMP/FlvParser.h"
namespace Public {
namespace RTMP {

struct FlvFileReader::FlvFileReaderInternal
{
	FILE*			readfd;
	uint32_t		headeroffset;

	static uint32_t be_read_uint32(const char* ptr)
	{
		return ((ptr[0]&0xff) << 24) | ((ptr[1] & 0xff) << 16) | ((ptr[2] & 0xff) << 8) | (ptr[3] & 0xff);
	}
};

FlvFileReader::FlvFileReader()
{
	internal = new FlvFileReaderInternal();
	internal->readfd = NULL;
}
FlvFileReader::~FlvFileReader()
{
	close();
	SAFE_DELETE(internal);
}
bool FlvFileReader::open(const std::string& filename)
{
	close();
	internal->readfd = fopen(filename.c_str(), "rb");
	if (internal->readfd == NULL) return false;

	char data[9];

	if (sizeof(data) != fread(data, 1, sizeof(data), internal->readfd))
		return false;

	if ('F' != data[0] || 'L' != data[1] || 'V' != data[2])
		return false;

	assert(0x00 == (data[4] & 0xF8) && 0x00 == (data[4] & 0x20));
	internal->headeroffset = FlvFileReaderInternal::be_read_uint32(data + 5);

	assert(internal->headeroffset >= sizeof(data));
	if (internal->headeroffset > sizeof(data))
		fseek(internal->readfd, internal->headeroffset, SEEK_SET);

	// PreviousTagSize0
	if (4 != fread(data, 1, 4, internal->readfd))
		return false;

	assert(FlvFileReaderInternal::be_read_uint32(data) == 0);
	
	return true;
}
bool FlvFileReader::close()
{
	if (internal->readfd != NULL)
	{
		fclose(internal->readfd);
		internal->readfd = NULL;
	}

	return true;
}

bool FlvFileReader::read(PacketInfo& info)
{
	if (internal->readfd == NULL) return false;

	char header[11] = {0};
	
	if (11 != fread(&header, 1, 11, internal->readfd))
		return false; // read file error

				   // DataSize
	info.size = ((header[1] & 0xff) << 16) | ((header[2] & 0xff) << 8) | (header[3] & 0xff);

	// TagType
	info.type = (RTMPType)(header[0] & 0x1F);

	// TimestampExtended | Timestamp
	info.timestamp = ((header[4]&0xff) << 16) | ((header[5] & 0xff) << 8) | (header[6] & 0xff) | ((header[7] & 0xff) << 24);

	// StreamID Always 0
	assert(0 == header[8] && 0 == header[9] && 0 == header[10]);


	info.data = new char[info.size];
	if (info.data == NULL) return false;

	// FLV stream
	if (info.size != fread(info.data, 1, info.size, internal->readfd))
	{
		delete[]info.data;

		return false;
	}
	// PreviousTagSizeN
	if (4 != fread(header, 1, 4, internal->readfd))
	{
		delete[]info.data;

		return false;
	}

	bool check = (FlvFileReaderInternal::be_read_uint32(header) == info.size + 11);
	if (!check)
	{
		delete[]info.data;

		return false;
	}

	return check;
}
bool FlvFileReader::seek(uint32_t seektimestmap)
{
	if (internal->readfd == NULL) return false;

	fseek(internal->readfd, internal->headeroffset + 4, SEEK_SET);

	while (1)
	{
		uint32_t prefpos = ftell(internal->readfd);

		char header[11];

		if (11 != fread(&header, 1, 11, internal->readfd))
			return false; // read file error

		// DataSize
		uint32_t size = ((header[1] & 0xff) << 16) | ((header[2] & 0xff) << 8) | (header[3] & 0xff);
		// TimestampExtended | Timestamp
		uint32_t timestamp = ((header[4] & 0xff) << 16) | ((header[5] & 0xff) << 8) | (header[6] & 0xff) | ((header[7] & 0xff) << 24);

		if (timestamp > seektimestmap)
		{
			fseek(internal->readfd, prefpos, SEEK_SET);

			return true;
		}

		uint32_t seekpos = prefpos + 11 + size + 4;
		fseek(internal->readfd, seekpos, SEEK_SET);
	}

	return false;
}

}
}