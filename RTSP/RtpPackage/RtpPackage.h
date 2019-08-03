#ifndef _RTPPACKAGE_H__
#define _RTPPACKAGE_H__
#include <boost/function.hpp>
#include <string>
class RtpPackage
{
public:
	typedef boost::function<void(char* data, int size, void*, bool)> RtpPackageCallback;

	RtpPackage(const RtpPackageCallback &callback);

	~RtpPackage();

	void SetFrameRate(int framerate); 

	int GetFrameRate();

	void SetSamplingRate(int samplingRate);

	int GetSamplingRate();

	bool startOfFile(const std::string &filename);

	/// 输入数据必须为一个完整帧
	bool InputVideoData(unsigned char* data, int size);

	bool InputAudioData(unsigned char* data, int size);
private:
	RtpPackageCallback rtpcallback;
	char *pVideoBuffer;
	int nVideobufSize;
	int video_seq_num;
	int nFrameRate;
	unsigned long curtimestamp;

	int audio_seq_num;
	char *pAudioBuffer;
	int nAudiobufSize;
	int nSamplingRate; 
};

#endif