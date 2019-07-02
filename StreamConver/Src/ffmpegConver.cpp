#include "ffmpegConver.h"

extern "C"
{
#include "libavutil/timestamp.h"
#include "libavformat/avformat.h"
};

#define  TYPE_RTMP  1
#define  TYPE_HLS   2

#define VIDEOFLAG		"video"
        
ffmpegConver::ffmpegConver() :Thread("ffmpegConver"), m_endoffile(false),m_timeStamp(0)
{
    m_ffmpegConver_type = TYPE_RTMP;
}


ffmpegConver::ffmpegConver(uint32_t hls_timeSecond, uint32_t recordTotalTime, const std::string &hls_Path, const std::string& m3u8name) :Thread("ffmpegHLSConver"), m_endoffile(false)
{
    m_hls_timeSecond = hls_timeSecond;
	m_recordTotalTime = recordTotalTime;
	m_m3u8name		 = m3u8name;
    m_hls_Path       = hls_Path;
    m_ffmpegConver_type = TYPE_HLS;
}


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

void ffmpegConver::clearBuffer()
{
	Guard locker(mutex);
	cachelist.clear();
}

void ffmpegConver::setTimestamp(uint32_t timeStamp)
{
	m_timeStamp = timeStamp;
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

int64_t ffmpegConver::seek_packet(void *opaque, int64_t offset, int whence)
{
	return 0;
}

int ffmpegConver::write_HLS_indexfile()
{
	FILE *index_fp = NULL;
    char buffer[1024] = {0};

	if (!File::access(m_hls_Path, File::accessExist)) File::makeDirectory(m_hls_Path);
	
	sprintf( buffer, "%s/%s",  m_hls_Path.c_str(), m_m3u8name.c_str() );

    index_fp = fopen(buffer, "wb+" );

    if ( !index_fp )
    {
        logdebug( "Could not open m3u8 index file (%s), no index file will be created\n", buffer);
        return -1;
    }

   sprintf(buffer, "#EXTM3U\n#EXT-X-VERSION:3\n#EXT-X-MEDIA-SEQUENCE:0\n#EXT-X-ALLOW-CACHE:YES\n#EXT-X-TARGETDURATION:%d\n", m_hls_timeSecond );


    if ( fwrite(buffer, strlen(buffer), 1, index_fp ) != 1 )
    {
        logdebug( "Could not write to m3u8 index file, will not continue writing to index file\n" );
        fclose( index_fp );
        return -1;
    }

	uint32_t tmpVal = m_recordTotalTime % m_hls_timeSecond;
	uint32_t tsFileCount = tmpVal != 0 ? m_recordTotalTime / m_hls_timeSecond + 1 : m_recordTotalTime / m_hls_timeSecond;

    for (uint32_t i = 0; i < tsFileCount; i++ )
    {
		//如果没有余下的时间，就说明刚好切完
		uint32_t tmp_hls_timeSecond = m_hls_timeSecond;
		if (tmpVal != 0 && i == tsFileCount - 1)
		{
			tmp_hls_timeSecond = tmpVal;
		}

        sprintf(buffer, "#EXTINF:%u,\n%s_%04d.ts\n", tmp_hls_timeSecond,  VIDEOFLAG, i );

        if ( fwrite(buffer, strlen(buffer), 1, index_fp ) != 1 )
        {
            printf( "Could not write to m3u8 index file, will not continue writing to index file\n" );
            fclose( index_fp );
            return -1;
        }
    }


	//end
	sprintf(buffer, "#EXT-X-ENDLIST\n" );
	if ( fwrite(buffer, strlen(buffer), 1, index_fp ) != 1 )
	{
		logdebug( "Could not write last file and endlist tag to m3u8 index file\n" );
		fclose( index_fp );
		return -1;
	}
    fclose( index_fp );
    return 0;
}


void ffmpegConver::ffmpegHLSConverProc()
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
	int framerate = 0;

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
				if (ifmt_ctx->pb == NULL) 
                    break;

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
			logdebug(" ffmpegHLSConver::threadProc");
		}
		
		//init stream info
		{
			if (avformat_find_stream_info(ifmt_ctx, 0) < 0) 
			{
                
                break;
			}

			stream_mapping_size = ifmt_ctx->nb_streams;
			stream_mapping = new int[stream_mapping_size];
            
			if (stream_mapping == NULL) 
			{   
                
				break;
			}
		}

		av_dump_format(ifmt_ctx, NULL, m_iFilename.c_str(), 0);

		AVStream* stream = ifmt_ctx->streams[0];
		if (stream != NULL && stream->r_frame_rate.den != 0)
		{
			framerate = stream->r_frame_rate.num / stream->r_frame_rate.den;
		}
		else
		{
			framerate = 25;
		}

		logdebug(" ffmpegHLSConver::threadProc");
		int tmp = AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX;
		int frame_index = 0;
        int init_ofmt_ctx  = 0;
        int m_output_index = 0;
        char output_file_name[1024] = {0};
        int Videoframe_index = 0;
        int videoindex = -1;
		int64_t lastVideoDts = 0;
		int64_t lastAudioDts = 0;

        write_HLS_indexfile();

        
		while (looping())
		{
			AVPacket pkt;

			//init output
			if (init_ofmt_ctx == 0)
			{
				sprintf(output_file_name, "%s/%s_%04d.ts", m_hls_Path.c_str(), VIDEOFLAG, m_output_index++);

				avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, output_file_name);
				//avformat_alloc_output_context2(&ofmt_ctx, NULL, getflag().c_str(), m_oFilename.c_str());

				if (ofmt_ctx == NULL)
				{

					break;
				}

				int stream_index = 0;

				for (uint32_t i = 0; i < ifmt_ctx->nb_streams; i++)
				{
					AVStream *in_stream = ifmt_ctx->streams[i];
					AVCodecParameters *in_codecpar = in_stream->codecpar;

					if (ifmt_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
					{
						videoindex = i;
					}

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
					if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER) {
						int a = 0;
					}
				}


				//int ret = avio_open(&ofmt_ctx->pb, m_oFilename.c_str(), AVIO_FLAG_WRITE);
				int ret = avio_open(&ofmt_ctx->pb, output_file_name, AVIO_FLAG_WRITE);
				if (ret < 0)
				{
					printf("avio_open < 0\n");
					break;
				}

				if (avformat_write_header(ofmt_ctx, NULL) < 0)
				{
					break;
				}


				//av_dump_format(ofmt_ctx, NULL, output_file_name, 1);

				init_ofmt_ctx = 1;

			}

			if (av_read_frame(ifmt_ctx, &pkt) < 0)
			{
				break;
			}
         
			AVStream * in_stream = ifmt_ctx->streams[pkt.stream_index];
            
			if (pkt.stream_index >= stream_mapping_size || stream_mapping[pkt.stream_index] < 0) 
			{
				av_packet_unref(&pkt);
				continue;
			}

            if( pkt.stream_index == videoindex)
            {
                Videoframe_index++;
            }

            //printf("ffmpegConver::av_read_frame = %d\n", pkt.size );
			pkt.stream_index = stream_mapping[pkt.stream_index];
			AVStream * out_stream = ofmt_ctx->streams[pkt.stream_index];

			if (pkt.pts == AV_NOPTS_VALUE)
			{
				//Write PTS
				AVRational time_base1 = ifmt_ctx->streams[pkt.stream_index]->time_base;
				//Duration between 2 frames (us)
				double calc_duration = (double)AV_TIME_BASE / av_q2d(ifmt_ctx->streams[pkt.stream_index]->r_frame_rate);
				//Parameters
				pkt.pts = (int64_t)((double)(frame_index * calc_duration) / (double)(av_q2d(time_base1) * AV_TIME_BASE));
				pkt.dts = pkt.pts;
				pkt.duration = (int64_t)((double)calc_duration / (double)(av_q2d(time_base1) * AV_TIME_BASE));
				frame_index++;
			}
			/* copy packet */
			pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base, (AVRounding)tmp);
			pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base, (AVRounding)tmp);
			pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
			pkt.pos = -1;

			if (pkt.stream_index == 0)
			{
				if(lastVideoDts != 0 && lastVideoDts >= pkt.dts)
				{
					continue;
				}
				lastVideoDts = pkt.dts;
			}
			else if (pkt.stream_index == 1)
			{
				if (lastAudioDts != 0 && lastAudioDts >= pkt.dts)
				{
					continue;
				}
				lastAudioDts = pkt.dts;
			}

			int ret = av_interleaved_write_frame(ofmt_ctx, &pkt);
			if (ret < 0) 
			{
				av_packet_unref(&pkt);
				break;
			}
			av_packet_unref(&pkt);

            if (Videoframe_index != 0 &&  Videoframe_index % (framerate * m_hls_timeSecond) == 0 )
            {
                ret = av_write_trailer( ofmt_ctx );
                avio_close( ofmt_ctx->pb);
            	avformat_free_context(ofmt_ctx);
                ofmt_ctx  = nullptr;
            	init_ofmt_ctx = 0;
				Videoframe_index = 0;
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

void ffmpegConver::ffmpegRTMPConverProc()
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
	int64_t lastVideoDts = 0;
	int64_t lastAudioDts = 0;

    AVDictionary    *format_opts = NULL;
	AVDictionary    *format_outpts = NULL;
	int frameRate = 0;

	if(m_outPutType == Target::StreamFormat_FLV)
	{
		/*av_dict_set(&format_opts, "fflags", "nobuffer ", 0);
		av_dict_set(&format_opts, "max_delay", "50", 0);
		av_dict_set(&format_opts, "analyzeduration", "1000000", 0);*/
	}
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

            

				//if (avformat_open_input(&ifmt_ctx, NULL, NULL, NULL) < 0)
                AVInputFormat* iformat = av_find_input_format("h264");
				if (avformat_open_input(&ifmt_ctx, NULL, iformat, &format_opts) < 0)
				{
                    break;
				}
			}
			//file mode
			else
			{
                if (avformat_open_input(&ifmt_ctx, m_iFilename.c_str(), NULL, &format_opts) < 0)
				//if (avformat_open_input(&ifmt_ctx, m_iFilename.c_str(), 0, 0) < 0)
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
				outcontent = ofmt_ctx->pb = avio_alloc_context((unsigned char*)oubuffer, MAXBUFFERCACHE, 1, this, NULL, write_packet, seek_packet);
				if (ofmt_ctx->pb == NULL) break;

				ofmt_ctx->flags |= AVFMT_FLAG_CUSTOM_IO;

				if (m_outPutType == Target::StreamFormat_MP4)
				{
					av_dict_set(&format_outpts, "movflags", "frag_keyframe+empty_moov", 0);
				}
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

		ifmt_ctx->probesize = 60 * 1024;
		ifmt_ctx->max_analyze_duration = 3 * AV_TIME_BASE;

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
		int ret = avformat_write_header(ofmt_ctx, &format_outpts);
		if (ret < 0) 
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
                int frameRate = av_q2d(ifmt_ctx->streams[pkt.stream_index]->r_frame_rate);
				if (m_timeStamp != 0)
				{
					frame_index =   (m_timeStamp * frameRate);
					m_timeStamp = 0;
				}
				//Write PTS
				AVRational time_base1 = ifmt_ctx->streams[pkt.stream_index]->time_base;
				//Duration between 2 frames (us)
				double calc_duration = (double)AV_TIME_BASE / av_q2d(ifmt_ctx->streams[pkt.stream_index]->r_frame_rate);
				//Parameters
				pkt.pts = (int64_t)((double)(frame_index * calc_duration) / (double)(av_q2d(time_base1) * AV_TIME_BASE));
				pkt.dts = pkt.pts;
				pkt.duration = (int64_t)((double)calc_duration / (double)(av_q2d(time_base1) * AV_TIME_BASE));
				frame_index++;
			}
			/* copy packet */
			pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base, (AVRounding)tmp);
			pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base, (AVRounding)tmp);
			pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
			pkt.pos = -1;

			if (pkt.stream_index == 0)
			{
				if (lastVideoDts != 0 && lastVideoDts >= pkt.dts)
				{
					continue;
				}
				lastVideoDts = pkt.dts;
			}
			else if (pkt.stream_index == 1)
			{
				if (lastAudioDts != 0 && lastAudioDts >= pkt.dts)
				{
					continue;
				}
				lastAudioDts = pkt.dts;
			}

			//printf("pkt.pts %lld, pkt.dts %lld, pkt.duration %lld\r\n", pkt.pts, pkt.dts, pkt.duration);
			int ret = av_interleaved_write_frame(ofmt_ctx, &pkt);
			av_packet_unref(&pkt);

			if (ret < 0) 
			{
				break;
			}
		}

		av_write_trailer(ofmt_ctx);
	} while (0);

	if (format_opts != NULL)
	{
		av_dict_free(&format_opts);
	}
	if (format_outpts != NULL)
	{
		av_dict_free(&format_outpts);
	}
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
    //av_free(inbuffer);
    //av_free(oubuffer);

	m_endCallback(m_endptr);
}

void ffmpegConver::threadProc()
{
    if ( m_ffmpegConver_type == TYPE_RTMP )
    {
        ffmpegRTMPConverProc();
    }
    else if ( m_ffmpegConver_type == TYPE_HLS )
    {
        if (m_iFilename == "")
        { 
            ffmpegHLSConverProc();
        }
        else
        {
            ////file mode
            //if ( Check_input_file_CodeType( m_iFilename.c_str() ) == 1 )
            //{
            //    ffmpegDecEnCodeHLSConverProc( );
            //}
            //else
            //{
                 ffmpegHLSConverProc();
            //}
        }
    }
}

std::string ffmpegConver::getflag()
{
	if (m_outPutType == Target::StreamFormat_AVI) return "avi";
	else if (m_outPutType == Target::StreamFormat_MP4) return "mp4";
	else if (m_outPutType == Target::StreamFormat_MKV) return "mkv";
	else if (m_outPutType == Target::StreamFormat_MPG) return "mpg";
	else return "flv";
}





void ffmpegConver::ffmpegDecEnCodeHLSConverProc()
{
	static bool ffmpegregistetstatus = false;
	if (!ffmpegregistetstatus)
	{
		av_register_all();
		avformat_network_init();
		ffmpegregistetstatus = true;
	}


    int ret;
    AVPacket packet;
    AVFrame *frame = NULL;
    enum AVMediaType type;
    unsigned int stream_index;
    unsigned int i;
    int got_frame;
    int m_fps;
    int last_index = 0, Videoframe_index = 0;
    int init_ofmt_ctx = 0;
    int m_output_index = 0;
    char output_file_name[1024] = {0};


    int ( *dec_func )( AVCodecContext *, AVFrame *, int *, const AVPacket* );


    printf("ffmpegDecEnCodeHLSConverProc--------->\n");
    

    if ( ( ret = open_input_file( m_iFilename.c_str() ) ) < 0 )
        goto end;


    sprintf(output_file_name, "%s/%s_%04d.ts", m_hls_Path.c_str(), VIDEOFLAG, 0);
    
    if ( ( ret = open_output_file( output_file_name ) ) < 0 )
        goto end;

    if ( ( ret = init_filters() ) < 0 )
        goto end;


    for ( i = 0; i < ifmt_ctx->nb_streams; i++ )
    {
        if ( ifmt_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO )
        {
            if ( ifmt_ctx->streams[i]->avg_frame_rate.den && ifmt_ctx->streams[i]->avg_frame_rate.num)
            {
                m_fps = av_q2d( ifmt_ctx->streams[i]->avg_frame_rate);
            }
            else
                m_fps = 25;
        }
    }

    write_HLS_indexfile();
    
    while ( looping() )
    {
        if ( ( ret = av_read_frame( ifmt_ctx, &packet ) ) < 0 )
            break;

        if ( init_ofmt_ctx == 0 )
        {
            sprintf(output_file_name, "%s/%s_%04d.ts", m_hls_Path.c_str(), VIDEOFLAG, m_output_index++);
            printf("output_file_name = %s\n", output_file_name);

            if ( !( ofmt_ctx->oformat->flags & AVFMT_NOFILE ) )
            {
                ret = avio_open( &ofmt_ctx->pb, output_file_name, AVIO_FLAG_WRITE );

                if ( ret < 0 )
                {
                    av_log( NULL, AV_LOG_ERROR, "Could not open output file '%s'", output_file_name );
                    return;
                }
            }

            ret = avformat_write_header( ofmt_ctx, NULL );
            if ( ret < 0 )
            {
                av_log( NULL, AV_LOG_ERROR, "Error occurred when opening output file\n" );
                return;
            }
            init_ofmt_ctx = 1;
        }

        stream_index = packet.stream_index;
        type = ifmt_ctx->streams[packet.stream_index]->codec->codec_type;

        av_log( NULL, AV_LOG_DEBUG, "Demuxergave frame of stream_index %u\n", stream_index );

        if ( filter_ctx[stream_index].filter_graph )
        {
            av_log( NULL, AV_LOG_DEBUG, "Going to reencode&filter the frame\n" );

            frame = av_frame_alloc();

            if ( !frame )
            {
                ret = AVERROR( ENOMEM );
                break;
            }
            packet.dts = av_rescale_q_rnd( packet.dts,
                                           ifmt_ctx->streams[stream_index]->time_base,
                                           ifmt_ctx->streams[stream_index]->codec->time_base,
                                           ( AVRounding )( AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX ) );

            packet.pts = av_rescale_q_rnd( packet.pts,
                                           ifmt_ctx->streams[stream_index]->time_base,
                                           ifmt_ctx->streams[stream_index]->codec->time_base,
                                           ( AVRounding )( AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX ) );

            //Decode
            dec_func = ( type == AVMEDIA_TYPE_VIDEO ) ? avcodec_decode_video2 : avcodec_decode_audio4;

            ret = dec_func( ifmt_ctx->streams[stream_index]->codec, frame, &got_frame, &packet );

            if ( ret < 0 )
            {
                av_frame_free( &frame );
                av_log( NULL, AV_LOG_ERROR, "Decoding failed\n" );
                break;
            }

            if ( got_frame )
            {
                frame->pts = av_frame_get_best_effort_timestamp( frame );

                ret = filter_encode_write_frame( frame, stream_index );

                av_frame_free( &frame );

                if ( ret < 0 )
                {
                    printf("filter_encode_write_frame goto end\n");
                    goto end;
                }
            }
            else
            {
                av_frame_free( &frame );
            }
        }
        else
        {
            packet.dts = av_rescale_q_rnd( packet.dts,
                                           ifmt_ctx->streams[stream_index]->time_base,
                                           ofmt_ctx->streams[stream_index]->time_base,
                                           ( AVRounding )( AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX ) );

            packet.pts = av_rescale_q_rnd( packet.pts,
                                           ifmt_ctx->streams[stream_index]->time_base,
                                           ofmt_ctx->streams[stream_index]->time_base,
                                           ( AVRounding )( AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX ) );

            ret = av_interleaved_write_frame( ofmt_ctx, &packet );

            if ( ret < 0 )
                goto end;
        }

        /*********************************/
        if( packet.stream_index == 0 )
        {
            Videoframe_index++;
        }

        if ( ( last_index != Videoframe_index) && ( Videoframe_index % (50 * 30) == 0 ) )
        {
            ret = av_write_trailer( ofmt_ctx );
            avio_close( ofmt_ctx->pb);
            last_index = Videoframe_index;
            init_ofmt_ctx = 0;
            printf("av_write_trailer = %d\n", Videoframe_index );
        }

        av_free_packet( &packet );
    }

end:
    av_free_packet( &packet );
    av_frame_free( &frame );

    for ( i = 0; i < ifmt_ctx->nb_streams; i++ )
    {
        avcodec_close( ifmt_ctx->streams[i]->codec );

        if ( ofmt_ctx && ofmt_ctx->nb_streams > i && ofmt_ctx->streams[i] && ofmt_ctx->streams[i]->codec )
            avcodec_close( ofmt_ctx->streams[i]->codec );

        if( filter_ctx && filter_ctx[i].filter_graph )
            avfilter_graph_free( &filter_ctx[i].filter_graph );
    }
    
    printf("av_free( filter_ctx )\n");
    
    av_free( filter_ctx );
    avformat_close_input( &ifmt_ctx );

    //if ( ofmt_ctx && !( ofmt_ctx->oformat->flags & AVFMT_NOFILE ) )
        //avio_close( ofmt_ctx->pb );
    printf("avformat_free_context\n");
    avformat_free_context( ofmt_ctx );

    m_endCallback(m_endptr);
}





int ffmpegConver::Check_input_file_CodeType( const char *filename )
{
    int ret = 0, CodeType;
    unsigned int i;
    AVFormatContext *fmtContex = NULL;

    if ( ( ret = avformat_open_input( &fmtContex, filename, NULL, NULL ) ) < 0 )
    {
        av_log( NULL, AV_LOG_ERROR, "Cannot open input file\n" );
        return ret;
    }

    if ( ( ret = avformat_find_stream_info( fmtContex, NULL ) ) < 0 )
    {
        av_log( NULL, AV_LOG_ERROR, "Cannot find stream information\n" );
        return ret;
    }

    CodeType = 0;
    for ( i = 0; i < fmtContex->nb_streams; i++ )
    {
        AVStream    *stream;
        AVCodecContext *codec_ctx;
        stream = fmtContex->streams[i];
        codec_ctx = stream->codec;

        if ( codec_ctx->codec_type == AVMEDIA_TYPE_VIDEO )
        {
            printf("codec_ctx->codec_id = %d\n", codec_ctx->codec_id );
            if ( codec_ctx->codec_id != AV_CODEC_ID_H264 )
                CodeType = 1;
            else
                CodeType = 0;
        }
    }
    avformat_close_input( &fmtContex );

    return CodeType;
}



int ffmpegConver::open_input_file( const char *filename )
{
    int ret;
    unsigned int i;
    ifmt_ctx = NULL;

    if ( ( ret = avformat_open_input( &ifmt_ctx, filename, NULL, NULL ) ) < 0 )
    {
        av_log( NULL, AV_LOG_ERROR, "Cannot open input file\n" );
        return ret;
    }

    if ( ( ret = avformat_find_stream_info( ifmt_ctx, NULL ) ) < 0 )
    {
        av_log( NULL, AV_LOG_ERROR, "Cannot find stream information\n" );
        return ret;
    }

    for ( i = 0; i < ifmt_ctx->nb_streams; i++ )
    {
        AVStream*stream;
        AVCodecContext *codec_ctx;
        stream = ifmt_ctx->streams[i];
        codec_ctx = stream->codec;

        if ( codec_ctx->codec_type == AVMEDIA_TYPE_VIDEO || codec_ctx->codec_type == AVMEDIA_TYPE_AUDIO )
        {
            ret = avcodec_open2( codec_ctx, avcodec_find_decoder( codec_ctx->codec_id ), NULL );
            if ( ret < 0 )
            {
                av_log( NULL, AV_LOG_ERROR, "Failed to open decoder for stream #%u\n", i );
                return ret;
            }
        }
    }
    av_dump_format( ifmt_ctx, 0, filename, 0 );
    return 0;
}



int ffmpegConver::open_output_file( const char *filename )
{
    AVStream  *out_stream;
    AVStream  *in_stream;
    AVCodecContext  *dec_ctx, *enc_ctx;
    AVCodec  *encoder;
    int ret;
    unsigned int i;

    ofmt_ctx = NULL;
    avformat_alloc_output_context2( &ofmt_ctx, NULL, NULL, filename );

    if ( !ofmt_ctx )
    {
        av_log( NULL, AV_LOG_ERROR, "Could not create output context\n" );
        return AVERROR_UNKNOWN;
    }

    for ( i = 0; i < ifmt_ctx->nb_streams; i++ )
    {
        out_stream = avformat_new_stream( ofmt_ctx, NULL );

        if ( !out_stream )
        {
            av_log( NULL, AV_LOG_ERROR, "Failed allocating output stream\n" );
            return AVERROR_UNKNOWN;
        }

        in_stream = ifmt_ctx->streams[i];
        dec_ctx = in_stream->codec;
        enc_ctx = out_stream->codec;

        if ( dec_ctx->codec_type == AVMEDIA_TYPE_VIDEO || dec_ctx->codec_type == AVMEDIA_TYPE_AUDIO )
        {
            encoder = avcodec_find_encoder( dec_ctx->codec_id );

            if ( dec_ctx->codec_type == AVMEDIA_TYPE_VIDEO )
            {
                enc_ctx->height = dec_ctx->height;
                enc_ctx->width = dec_ctx->width;
                enc_ctx->sample_aspect_ratio = dec_ctx->sample_aspect_ratio;

                enc_ctx->pix_fmt = encoder->pix_fmts[0];

                enc_ctx->time_base = dec_ctx->time_base;
            }
            else
            {
                enc_ctx->sample_rate = dec_ctx->sample_rate;
                enc_ctx->channel_layout = dec_ctx->channel_layout;
                enc_ctx->channels = av_get_channel_layout_nb_channels( enc_ctx->channel_layout );

                enc_ctx->sample_fmt = encoder->sample_fmts[0];
                AVRational time_base = {1, enc_ctx->sample_rate};
                enc_ctx->time_base = time_base;
            }

            ret = avcodec_open2( enc_ctx, encoder, NULL );
            if ( ret < 0 )
            {
                av_log( NULL, AV_LOG_ERROR, "Cannot open video encoder for stream #%u\n", i );
                return ret;
            }
        }
        else if( dec_ctx->codec_type == AVMEDIA_TYPE_UNKNOWN )
        {
            av_log( NULL, AV_LOG_FATAL, "Elementary stream #%d is of unknown type, cannot proceed\n", i );
            return AVERROR_INVALIDDATA;
        }
        else
        {
            ret = avcodec_copy_context( ofmt_ctx->streams[i]->codec, ifmt_ctx->streams[i]->codec );

            if ( ret < 0 )
            {
                av_log( NULL, AV_LOG_ERROR, "Copying stream context failed\n" );
                return ret;
            }
        }
        if ( ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER )
            enc_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }

    return 0;
}


int ffmpegConver::init_filter( FilteringContext* fctx, AVCodecContext *dec_ctx,
                       AVCodecContext *enc_ctx, const char *filter_spec )
{
    char args[512];
    int ret = 0;
    const AVFilter	*buffersrc = NULL;
	const AVFilter	*buffersink = NULL;
    AVFilterContext		*buffersrc_ctx = NULL;
    AVFilterContext		*buffersink_ctx = NULL;

    AVFilterInOut	*outputs = avfilter_inout_alloc();
    AVFilterInOut	*inputs  = avfilter_inout_alloc();
    AVFilterGraph	*filter_graph = avfilter_graph_alloc();

    if ( !outputs || !inputs || !filter_graph )
    {
        ret = AVERROR( ENOMEM );
        goto end;
    }

    if ( dec_ctx->codec_type == AVMEDIA_TYPE_VIDEO )
    {
        buffersrc = avfilter_get_by_name( "buffer" );
        buffersink = avfilter_get_by_name( "buffersink" );

        if ( !buffersrc || !buffersink )
        {
            av_log( NULL, AV_LOG_ERROR, "filtering source or sink element not found\n" );
            ret = AVERROR_UNKNOWN;
            goto end;
        }

        _snprintf( args, sizeof( args ),
                   "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
                   dec_ctx->width, dec_ctx->height, dec_ctx->pix_fmt,
                   dec_ctx->time_base.num, dec_ctx->time_base.den,
                   dec_ctx->sample_aspect_ratio.num,
                   dec_ctx->sample_aspect_ratio.den );

        ret = avfilter_graph_create_filter( &buffersrc_ctx, buffersrc, "in",
                                            args, NULL, filter_graph );

        if ( ret < 0 )
        {
            av_log( NULL, AV_LOG_ERROR, "Cannot create buffer source\n" );
            goto end;
        }

        ret = avfilter_graph_create_filter( &buffersink_ctx, buffersink, "out",
                                            NULL, NULL, filter_graph );

        if ( ret < 0 )
        {
            av_log( NULL, AV_LOG_ERROR, "Cannot create buffer sink\n" );
            goto end;
        }

        ret = av_opt_set_bin( buffersink_ctx, "pix_fmts",
                              ( uint8_t* )&enc_ctx->pix_fmt, sizeof( enc_ctx->pix_fmt ),
                              AV_OPT_SEARCH_CHILDREN );

        if ( ret < 0 )
        {
            av_log( NULL, AV_LOG_ERROR, "Cannot set output pixel format\n" );
            goto end;
        }
    }
    else if( dec_ctx->codec_type == AVMEDIA_TYPE_AUDIO )
    {
        buffersrc = avfilter_get_by_name( "abuffer" );
        buffersink = avfilter_get_by_name( "abuffersink" );

        if ( !buffersrc || !buffersink )
        {
            av_log( NULL, AV_LOG_ERROR, "filtering source or sink element not found\n" );
            ret = AVERROR_UNKNOWN;
            goto end;
        }

        if ( !dec_ctx->channel_layout )
            dec_ctx->channel_layout =
                av_get_default_channel_layout( dec_ctx->channels );


        _snprintf( args, sizeof( args ),
                   "time_base=%d/%d:sample_rate=%d:sample_fmt=%s:channel_layout=0x%I64x",
                   dec_ctx->time_base.num, dec_ctx->time_base.den, dec_ctx->sample_rate,
                   av_get_sample_fmt_name( dec_ctx->sample_fmt ),
                   dec_ctx->channel_layout );


        ret = avfilter_graph_create_filter( &buffersrc_ctx, buffersrc, "in",
                                            args, NULL, filter_graph );

        if ( ret < 0 )
        {
            av_log( NULL, AV_LOG_ERROR, "Cannot create audio buffer source\n" );
            goto end;
        }

        ret = avfilter_graph_create_filter( &buffersink_ctx, buffersink, "out",
                                            NULL, NULL, filter_graph );
        if ( ret < 0 )
        {
            av_log( NULL, AV_LOG_ERROR, "Cannot create audio buffer sink\n" );
            goto end;
        }


        ret = av_opt_set_bin( buffersink_ctx, "sample_fmts",
                              ( uint8_t* )&enc_ctx->sample_fmt, sizeof( enc_ctx->sample_fmt ),
                              AV_OPT_SEARCH_CHILDREN );
        if ( ret < 0 )
        {
            av_log( NULL, AV_LOG_ERROR, "Cannot set output sample format\n" );
            goto end;
        }


        ret = av_opt_set_bin( buffersink_ctx, "channel_layouts",
                              ( uint8_t* )&enc_ctx->channel_layout,
                              sizeof( enc_ctx->channel_layout ), AV_OPT_SEARCH_CHILDREN );
        if ( ret < 0 )
        {
            av_log( NULL, AV_LOG_ERROR, "Cannot set output channel layout\n" );
            goto end;
        }


        ret = av_opt_set_bin( buffersink_ctx, "sample_rates",
                              ( uint8_t* )&enc_ctx->sample_rate, sizeof( enc_ctx->sample_rate ),
                              AV_OPT_SEARCH_CHILDREN );
        if ( ret < 0 )
        {
            av_log( NULL, AV_LOG_ERROR, "Cannot set output sample rate\n" );
            goto end;
        }
    }
    else
    {
        ret = AVERROR_UNKNOWN;
        goto end;
    }


    outputs->name       = av_strdup( "in" );
    outputs->filter_ctx = buffersrc_ctx;
    outputs->pad_idx    = 0;
    outputs->next       = NULL;
    inputs->name       = av_strdup( "out" );
    inputs->filter_ctx = buffersink_ctx;
    inputs->pad_idx    = 0;
    inputs->next       = NULL;


    if ( !outputs->name || !inputs->name )
    {
        ret = AVERROR( ENOMEM );
        goto end;
    }
    if ( ( ret = avfilter_graph_parse_ptr( filter_graph, filter_spec, &inputs, &outputs, NULL ) ) < 0 )
        goto end;

    if ( ( ret = avfilter_graph_config( filter_graph, NULL ) ) < 0 )
        goto end;


    fctx->buffersrc_ctx = buffersrc_ctx;
    fctx->buffersink_ctx = buffersink_ctx;
    fctx->filter_graph = filter_graph;
end:
    avfilter_inout_free( &inputs );
    avfilter_inout_free( &outputs );
    return ret;
}


int ffmpegConver::init_filters( void )
{
    const char*filter_spec;
    unsigned int i;
    int ret;

    filter_ctx = ( FilteringContext * )av_malloc_array( ifmt_ctx->nb_streams, sizeof( *filter_ctx ) );

    if ( !filter_ctx )
        return AVERROR( ENOMEM );


    for ( i = 0; i < ifmt_ctx->nb_streams; i++ )
    {
        filter_ctx[i].buffersrc_ctx  = NULL;
        filter_ctx[i].buffersink_ctx = NULL;
        filter_ctx[i].filter_graph   = NULL;

        if( !( ifmt_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO
                || ifmt_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO ) )
            continue;


        if ( ifmt_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO )
            filter_spec = "null"; 
        else
            filter_spec = "anull"; 


        ret = init_filter( &filter_ctx[i], ifmt_ctx->streams[i]->codec,
                           ofmt_ctx->streams[i]->codec, filter_spec );
        if ( ret )
            return ret;
    }
    return 0;
}


int ffmpegConver::encode_write_frame( AVFrame *filt_frame, unsigned int stream_index, int*got_frame )
{
    int ret;
    int got_frame_local;
    AVPacket enc_pkt;

    static int64_t last_pts=0, last_dts=0;

    int ( *enc_func )( AVCodecContext *, AVPacket *, const AVFrame *, int* ) =
        ( ifmt_ctx->streams[stream_index]->codec->codec_type ==
          AVMEDIA_TYPE_VIDEO ) ? avcodec_encode_video2 : avcodec_encode_audio2;


    if ( !got_frame )
        got_frame = &got_frame_local;

    enc_pkt.data = NULL;
    enc_pkt.size = 0;
    av_init_packet( &enc_pkt );

    ret = enc_func( ofmt_ctx->streams[stream_index]->codec, &enc_pkt, filt_frame, got_frame );

    av_frame_free( &filt_frame );

    if ( ret < 0 )
        return ret;

    if ( !( *got_frame ) )
        return 0;


    enc_pkt.stream_index = stream_index;
    enc_pkt.dts = av_rescale_q_rnd( enc_pkt.dts,
                                    ofmt_ctx->streams[stream_index]->codec->time_base,
                                    ofmt_ctx->streams[stream_index]->time_base,
                                    ( AVRounding )( AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX ) );


    enc_pkt.pts = av_rescale_q_rnd( enc_pkt.pts,
                                    ofmt_ctx->streams[stream_index]->codec->time_base,
                                    ofmt_ctx->streams[stream_index]->time_base,
                                    ( AVRounding )( AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX ) );


    enc_pkt.duration = av_rescale_q( enc_pkt.duration,
                                     ofmt_ctx->streams[stream_index]->codec->time_base,
                                     ofmt_ctx->streams[stream_index]->time_base );


    av_log( NULL, AV_LOG_DEBUG, "Muxing frame\n" );

    if ( last_pts >= enc_pkt.pts  )
    {
        enc_pkt.pts = 20 + last_pts;
        enc_pkt.dts = 20 + last_dts;
    }
    last_pts = enc_pkt.pts;
    last_dts = enc_pkt.dts;

    ret = av_interleaved_write_frame( ofmt_ctx, &enc_pkt );
    
    return ret;
}


int ffmpegConver::filter_encode_write_frame( AVFrame *frame, unsigned int stream_index )
{
    int ret;
    AVFrame*filt_frame;

    ret = av_buffersrc_add_frame_flags( filter_ctx[stream_index].buffersrc_ctx, frame, 0 );

    if ( ret < 0 )
    {
        av_log( NULL, AV_LOG_ERROR, "Error while feeding the filter graph\n" );
        return ret;
    }

    while ( 1 )
    {
        filt_frame = av_frame_alloc();

        if ( !filt_frame )
        {
            ret = AVERROR( ENOMEM );
            break;
        }
        
        ret = av_buffersink_get_frame( filter_ctx[stream_index].buffersink_ctx, filt_frame );


        if ( ret < 0 )
        {
            if ( ret == AVERROR( EAGAIN ) || ret == AVERROR_EOF )
                ret = 0;
            av_frame_free( &filt_frame );
            break;
        }

        filt_frame->pict_type = AV_PICTURE_TYPE_NONE;
        
        ret = encode_write_frame( filt_frame, stream_index, NULL );

        if ( ret < 0 )
            break;
    }
    return ret;
}









