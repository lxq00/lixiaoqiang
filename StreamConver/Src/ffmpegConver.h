#pragma once
#include "StreamConver/StreamConver.h"
using namespace Public::StreamConver;

extern "C"
{
#include "libavutil/timestamp.h"
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavfilter/avfilter.h"
#include "libavfilter/buffersink.h"
#include "libavfilter/buffersrc.h"
#include "libavutil/avutil.h"
#include "libswscale/swscale.h"
#include "libavutil/time.h"
#include "libavutil/opt.h"
};

typedef struct FilteringContext
{
    AVFilterContext  *buffersink_ctx;
    AVFilterContext  *buffersrc_ctx;
    AVFilterGraph    *filter_graph;
} FilteringContext;



class ffmpegConver:public Thread
{
public:
	ffmpegConver();
	ffmpegConver(uint32_t hls_timeSecond, uint32_t recordTotalTime, const std::string &hls_Path,const std::string& m3u8name );
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

	void clearBuffer();

	void setTimestamp(uint32_t timeStamp);

	int test();
private:
	static int read_packet(void *opaque, uint8_t *buf, int buf_size);

    static int write_packet(void *opaque, uint8_t *buf, int buf_size);
	
	static int64_t seek_packet(void *opaque, int64_t offset, int whence);

	void threadProc();
	void ffmpegRTMPConverProc( );
	void ffmpegHLSConverProc( );
	void ffmpegDecEnCodeHLSConverProc();
	int write_HLS_indexfile();

	std::string getflag();

	int  Check_input_file_CodeType( const char *filename );
	
	int  open_input_file( const char *filename );
	int  open_output_file( const char *filename );
	int  init_filter( FilteringContext* fctx, AVCodecContext *dec_ctx,
									AVCodecContext *enc_ctx, const char *filter_spec );

	int  init_filters( void );
	int  encode_write_frame( AVFrame *filt_frame, unsigned int stream_index, int*got_frame );
	int  filter_encode_write_frame( AVFrame *frame, unsigned int stream_index );



                       
	
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

	int 						m_ffmpegConver_type;

	uint32_t 					m_hls_timeSecond;
	std::string 				m_hls_Path;
	uint32_t					m_recordTotalTime;
	std::string					m_m3u8name;

	AVFormatContext 			*ifmt_ctx;
	AVFormatContext 			*ofmt_ctx;
	FilteringContext 			*filter_ctx;
	 
	uint32_t					m_timeStamp;

};