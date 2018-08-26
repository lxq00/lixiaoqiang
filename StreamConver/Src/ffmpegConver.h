#pragma once
#include "StreamConver/StreamConver.h"
using namespace Public::StreamConver;

class ffmpegConver:public Thread
{
public:
	ffmpegConver();
	~ffmpegConver();

    bool setTargetCallback(const Target::DataCallback& callback, Target::StreamFormat format, Target* targetptr);

    bool sourceInput(void* data, uint32_t len);
	
	bool setSourceFile(const std::string& filename);

	bool setSourceReadcallback(const Source::ReadCallback& readcallback);

	bool setTargetFile(const std::string& filename, Target::StreamFormat format);

	void setEndoffileCallback(const Conver::EndOfFileCallback& callback, Conver* conver);

	void setEndoffileFlag(bool flag);

	void start();

	void stop();
private:
	static int read_packet(void *opaque, uint8_t *buf, int buf_size);

    static int write_packet(void *opaque, uint8_t *buf, int buf_size);

	void threadProc();

	std::string getflag();
private:
	struct BufferInfo
	{
		char*	buffer;
		int		bufferlen;

		int		offset;

		BufferInfo(char* tmp, int len)
		{
			buffer = new char[len];
			memcpy(buffer, tmp, len);

			bufferlen = len;
			offset = 0;
		}
		~BufferInfo()
		{
			delete[] buffer;
		}
	};
	
	Mutex								mutex;
	std::list<shared_ptr<BufferInfo> >	cachelist;


	Target::DataCallback		m_outputCallback;
	Target::StreamFormat		m_outPutType;
	Target*						m_targetptr;

	Conver::EndOfFileCallback	m_endCallback;
	Conver*						m_endptr;

	Source::ReadCallback		m_readcallback;

    std::string					m_iFilename;
    std::string					m_oFilename;

	bool						m_endoffile;
};