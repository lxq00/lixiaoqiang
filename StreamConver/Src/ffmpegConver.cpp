#include "ffmpegConver.h"

extern "C"
{
#include "libavutil/timestamp.h"
#include "libavformat/avformat.h"
};

ffmpegConver::ffmpegConver() :Thread("ffmpegConver"), m_endoffile(false){}


ffmpegConver::~ffmpegConver()
{
	stop();
}

void ffmpegConver::setEndoffileFlag(bool flag)
{
	Guard locker(mutex);
	m_endoffile = flag;
}

bool ffmpegConver::setTargetCallback(const Target::DataCallback& callback, Target::StreamFormat format, Target* targetptr)
{
    m_outputCallback = callback;
    m_outPutType = format;
	m_targetptr = targetptr;

    return true;
}


bool ffmpegConver::sourceInput(void* data, uint32_t len)
{
    if (NULL == data || 0 == len)
    {
        return false;
    }

	static bool fristpackget = true;
	if (fristpackget)
	{
		logdebug("sourceInput %d",len);

		fristpackget = false;
	}
	shared_ptr<BufferInfo> info = make_shared<BufferInfo>((char*)data, len);

	Guard locker(mutex);
	cachelist.push_back(info);

    return true;
}

bool ffmpegConver::setSourceFile(const std::string& filename)
{
    m_iFilename = filename;
    return true;
}
bool ffmpegConver::setSourceReadcallback(const Source::ReadCallback& readcallback)
{
	m_readcallback = readcallback;
	return true;
}
bool ffmpegConver::setTargetFile(const std::string& filename, Target::StreamFormat format)
{
    m_oFilename = filename;
    m_outPutType = format;
    return true;
}

void ffmpegConver::setEndoffileCallback(const Conver::EndOfFileCallback& callback, Conver* conver)
{
	m_endCallback = callback;
	m_endptr = conver;
}

void ffmpegConver::start()
{
	createThread();
}

void ffmpegConver::stop()
{
	destroyThread();
}

int ffmpegConver::read_packet(void *opaque, uint8_t *buf, int buf_size)
{
	ffmpegConver* ffmpegconver = (ffmpegConver*)opaque;
	if (ffmpegconver == NULL) return -1;

	if (!ffmpegconver->looping()) return -1;

	while (ffmpegconver->looping())
	{
		if (ffmpegconver->m_readcallback != NULL)
		{
			int readlen = ffmpegconver->m_readcallback((char*)buf, buf_size);
			if (readlen == -1)
			{
				int a = 0;
			}
			if (readlen != 0) return readlen;
		}
		else
		{
			Guard locker(ffmpegconver->mutex);
			while (1)
			{
				if (ffmpegconver->cachelist.size() <= 0)	break;

				shared_ptr<BufferInfo> bufinfo = ffmpegconver->cachelist.front();

				int readlen = min(bufinfo->bufferlen - bufinfo->offset, buf_size);
				memcpy(buf, bufinfo->buffer + bufinfo->offset, readlen);

				bufinfo->offset += readlen;
				if (bufinfo->bufferlen - bufinfo->offset == 0)
				{
					ffmpegconver->cachelist.pop_front();
				}

				return readlen;
			}
			if (ffmpegconver->m_endoffile) return -1;
		}
		Thread::sleep(10);
	}


	//need wait
	return 0;
}
int ffmpegConver::write_packet(void *opaque, uint8_t *buf, int buf_size)
{
	ffmpegConver* ffmpegconver = (ffmpegConver*)opaque;
	if (ffmpegconver == NULL) return -1;

	if (!ffmpegconver->looping()) return -1;

	ffmpegconver->m_outputCallback(ffmpegconver->m_targetptr, buf, buf_size);

	return buf_size;
}


void ffmpegConver::threadProc()
{
	static bool ffmpegregistetstatus = false;
	if (!ffmpegregistetstatus)
	{
		av_register_all();
		avformat_network_init();
		ffmpegregistetstatus = true;
	}

#define MAXBUFFERCACHE 32768

	AVIOContext * incontent = NULL, *outcontent = NULL;
	AVFormatContext *ifmt_ctx = NULL, *ofmt_ctx = NULL;
	unsigned char* inbuffer = (unsigned char*)av_malloc(MAXBUFFERCACHE);
	unsigned char* oubuffer = (unsigned char*)av_malloc(MAXBUFFERCACHE);
	int *stream_mapping = NULL;
	int stream_mapping_size = 0;
	do
	{
		//init input
		{
			//buf mode
			if (m_iFilename == "")
			{
				ifmt_ctx = avformat_alloc_context();
				if (ifmt_ctx == NULL) break;

				incontent = ifmt_ctx->pb = avio_alloc_context((unsigned char*)inbuffer, MAXBUFFERCACHE, 0, this, read_packet, NULL, NULL);
				if (ifmt_ctx->pb == NULL) break;

				ifmt_ctx->flags |= AVFMT_FLAG_CUSTOM_IO;

				if (avformat_open_input(&ifmt_ctx, NULL, NULL, NULL) < 0)
				{
					break;
				}
			}
			//file mode
			else
			{
				if (avformat_open_input(&ifmt_ctx, m_iFilename.c_str(), 0, 0) < 0)
				{
					break;
				}
			}
			logdebug(" ffmpegConver::threadProc");
		}
		
		//init output
		{
			//buf mode
			if (m_oFilename == "")
			{
				avformat_alloc_output_context2(&ofmt_ctx, NULL, getflag().c_str(), NULL);
				if (ofmt_ctx == NULL) break;

				/*open output file*/
				outcontent = ofmt_ctx->pb = avio_alloc_context((unsigned char*)oubuffer, MAXBUFFERCACHE, 1, this, NULL, write_packet, NULL);
				if (ofmt_ctx->pb == NULL) break;

				ofmt_ctx->flags |= AVFMT_FLAG_CUSTOM_IO;
			}
			//filemode
			else
			{
				avformat_alloc_output_context2(&ofmt_ctx, NULL, getflag().c_str(), m_oFilename.c_str());
				if (ofmt_ctx == NULL)  break;
				int ret = avio_open(&ofmt_ctx->pb, m_oFilename.c_str(), AVIO_FLAG_WRITE);
				if (ret < 0)
				{
					break;
				}
			}
			logdebug(" ffmpegConver::threadProc");
		}

		//init stream info
		{
			if (avformat_find_stream_info(ifmt_ctx, 0) < 0)  break;

			stream_mapping_size = ifmt_ctx->nb_streams;
			stream_mapping = new int[stream_mapping_size];
			if (stream_mapping == NULL) 
			{
				break;
			}
			logdebug(" ffmpegConver::threadProc");
			int stream_index = 0;
			for (uint32_t i = 0; i < ifmt_ctx->nb_streams; i++) 
			{
				AVStream *in_stream = ifmt_ctx->streams[i];
				AVCodecParameters *in_codecpar = in_stream->codecpar;

				if (in_codecpar->codec_type != AVMEDIA_TYPE_AUDIO &&
					in_codecpar->codec_type != AVMEDIA_TYPE_VIDEO &&
					in_codecpar->codec_type != AVMEDIA_TYPE_SUBTITLE) {
					stream_mapping[i] = -1;
					continue;
				}

				stream_mapping[i] = stream_index++;


				AVStream * out_stream = avformat_new_stream(ofmt_ctx, NULL);
				if (!out_stream) 
				{
					break;
				}

				if (avcodec_parameters_copy(out_stream->codecpar, in_codecpar) < 0) 
				{
					break;
				}
				out_stream->codecpar->codec_tag = 0;
			}
		}

		if (avformat_write_header(ofmt_ctx, NULL) < 0) 
		{
			break;
		}
		logdebug(" ffmpegConver::threadProc");
		int tmp = AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX;
		int frame_index = 0;
		while (looping())
		{
			AVPacket pkt;

			if (av_read_frame(ifmt_ctx, &pkt) < 0)
			{
				break;
			}
			AVStream * in_stream = ifmt_ctx->streams[pkt.stream_index];
			if (pkt.stream_index >= stream_mapping_size ||
				stream_mapping[pkt.stream_index] < 0) {
				av_packet_unref(&pkt);
				continue;
			}

			pkt.stream_index = stream_mapping[pkt.stream_index];
			AVStream * out_stream = ofmt_ctx->streams[pkt.stream_index];

			if (pkt.pts == AV_NOPTS_VALUE)
			{
				//Write PTS
				AVRational time_base1 = ifmt_ctx->streams[pkt.stream_index]->time_base;
				//Duration between 2 frames (us)
				int64_t calc_duration = (double)AV_TIME_BASE / av_q2d(ifmt_ctx->streams[pkt.stream_index]->r_frame_rate);
				//Parameters
				pkt.pts = (double)(frame_index * calc_duration) / (double)(av_q2d(time_base1) * AV_TIME_BASE);
				pkt.dts = pkt.pts;
				pkt.duration = (double)calc_duration / (double)(av_q2d(time_base1) * AV_TIME_BASE);
				frame_index++;
			}
			/* copy packet */
			pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base, (AVRounding)tmp);
			pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base, (AVRounding)tmp);
			pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
			pkt.pos = -1;

			int ret = av_interleaved_write_frame(ofmt_ctx, &pkt);
			av_packet_unref(&pkt);

			if (ret < 0) 
			{
				break;
			}
		}
	} while (0);

	if (ifmt_ctx != NULL)
	{
		if (incontent)
		{
			av_freep(&incontent->buffer);
			av_freep(&incontent);
		}
		avformat_close_input(&ifmt_ctx);
	}
	if (ofmt_ctx != NULL)
	{
		if (outcontent)
		{
			av_freep(&outcontent->buffer);
			av_freep(&outcontent);
		}
		avformat_close_input(&ofmt_ctx);
	}
	delete[]stream_mapping;
	//不知道这里有没有内存泄漏
// 	av_free(inbuffer);
// 	av_free(oubuffer);

	m_endCallback(m_endptr);
}

std::string ffmpegConver::getflag()
{
	if (m_outPutType == Target::StreamFormat_AVI) return "avi";
	else if (m_outPutType == Target::StreamFormat_MP4) return "mp4";
	else if (m_outPutType == Target::StreamFormat_MKV) return "mkv";
	else if (m_outPutType == Target::StreamFormat_MPG) return "mpg";
	else return "flv";
}