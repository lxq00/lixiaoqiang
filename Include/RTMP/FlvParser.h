#pragma once

#include "Defs.h"
#include "Base/Base.h"
using namespace Public::Base;

namespace Public {
namespace RTMP {

typedef Function1<void, const PacketInfo&> FlvCallback;

class RTMP_API FlvParser
{
public:
	FlvParser(const FlvCallback& callback);
	~FlvParser();

	void clean();
	void input(const char* buffer, int len);
	static bool parseFlv(const PacketInfo& pkg, FlvInfo& info);
private:
	struct FlvParserInternal;
	FlvParserInternal* internal;
};

class RTMP_API FlvFileReader
{
public:
	FlvFileReader();
	~FlvFileReader();

	bool open(const std::string& filename);
	bool close();

	bool read(PacketInfo& info);
	bool seek(uint32_t timestmap);
private:
	struct FlvFileReaderInternal;
	FlvFileReaderInternal* internal;
};

class RTMP_API FlvFileWriter
{
public:
	typedef Function2<void, const char*, int> WriteCallback;
public:
	FlvFileWriter();
	~FlvFileWriter();

	bool open(const std::string& filename);
	bool open(const WriteCallback& callback);
	bool close();
	bool write(RTMPType type, uint32_t timestmap /*ms*/, const char* buffer, int maxlen);
private:
	struct FlvFileWriterInternal;
	FlvFileWriterInternal* internal;
};

class RTMP_API H264ToFlv
{
public:
	H264ToFlv(const FlvCallback& callback);
	~H264ToFlv();

	void clean();
	void input(const char* buffer, int len);
private:
	struct H264ToFlvInternal;
	H264ToFlvInternal* internal;
};

}
}