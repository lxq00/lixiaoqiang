#pragma once

#include "Base/Base.h"
using namespace Public::Base;

#ifdef WIN32
#ifdef STREAMCONVER_EXPORTS
#define STREAMCONVER_API __declspec(dllexport)
#else
#define STREAMCONVER_API __declspec(dllimport)
#endif
#else
#define STREAMCONVER_API
#endif


namespace Public {
namespace StreamConver {

class ConverInternal;

///��ƵԴ
class STREAMCONVER_API Source
{
	friend class ConverInternal;
public:
	typedef Function3<void, Source*,void*, uint32_t> DataCallback;
	typedef Function1<bool, Source*> StartCallback;
	typedef Function1<void, bool> EndoffileCallback;

	//return > 0 ,readlen, == 0 wait , < 0 endoffile
	typedef Function2<int, char*, int> ReadCallback;
public:
	//buffer ��ʽ
	Source(const StartCallback& startcallback);
	void input(void* data, uint32_t len);
	void setEndOfFile();
	//�ļ���ʽ
	Source(const std::string& filename);
	//��ȡ��ʽ
	Source(const ReadCallback& readcallback);

	virtual ~Source();

private:
	void setCallback(const DataCallback& dcallback,const EndoffileCallback& eofcallback);
	bool start();
	bool stop();
	std::string filename() const;
private:
	struct SourceInternal;
	SourceInternal* internal;
};

//��Ƶ���ģ��
class STREAMCONVER_API Target
{
	friend class ConverInternal;
public:
	typedef enum {
		StreamFormat_FLV,
		StreamFormat_AVI,
		StreamFormat_MP4,
		StreamFormat_MKV,
		StreamFormat_MPG,
	}StreamFormat;/*
	typedef enum {
		StreamType_Video,
		StreamType_Audio,
	}StreamType;*/
	typedef enum {
		OutputType_File,
		OutputType_Buffer,
	}OutputType;
	//typedef Function5<void, Target*,StreamType /*type*/, uint32_t /*timestmap*/, void* /*data*/, uint32_t /*len*/> DataCallback;
	typedef Function3<void, Target*, void* /*data*/, uint32_t /*len*/> DataCallback;
public:	
	//���ķ�ʽ
	Target(const DataCallback& datacallback, StreamFormat streamformat);
	//�ļ��ķ�ʽ
	Target(const std::string& filename, StreamFormat streamformat);
	virtual ~Target();
private:
	bool start();
	bool stop();

	virtual void input(/*StreamType type,uint32_t timestmap,*/void* data,uint32_t len);

	StreamFormat format() const;
	OutputType   type() const;
	std::string filename() const;
	DataCallback callback() const;
private:
	struct TargetInternal;
	TargetInternal* internal;
};


//��Ƶת��ģ��
class STREAMCONVER_API Conver
{
public:
	typedef Function1<void, Conver*> EndOfFileCallback;
public:
	Conver(const shared_ptr<Source>& source,const shared_ptr<Target>& target);
	virtual ~Conver();

	bool start(const EndOfFileCallback& eofcallback);
	bool stop();
private:
	ConverInternal * internal;
};

}
}