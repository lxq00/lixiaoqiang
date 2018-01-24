#ifndef __MEDIACACHE_H__
#define __MEDIACACHE_H__
#include "Base/MediaPackage.h"
#include "Base/Defs.h"
#include "Base/Time.h"
#include <string>
namespace Xunmei{
namespace Base{

class BASE_API MediaCache
{
	struct MediaCacheInternal;
public:
	enum SeekStatus
	{
		SeekStatus_Ok,
		SeekStatus_NoData,
	};

	struct OutputResult
	{
		enum{
			OutputStatus_NotSuccessSeek,
			OutputStatus_CurrHaveNoData,
			OutputStatus_EndOfFile,
			OutputStatus_OK,
		}status;
		MediaPackage*	data;	//����=NULL ʱ��Ҫ�ⲿdelete
		Time			lastCachedTime;	//ʣ��¼����Ƶʱ��
	};

	struct InputResult
	{
		Time NextEmptyTime;			///��һ�������ݿ�ʱ��/��==0 һֱ����
	};
public:
	MediaCache(const std::string& cachePath,uint32_t devId,int channelID);
	~MediaCache();

	void setSystemHeader(const MediaPackage* header);
	
	///��ȡϵͳͷ����!= NULL��ʱ����Ҫ�û��Լ�delete
	MediaPackage* getSystemHeader();

	///seek��λ������getdata�Ķ�λ��ȡ
	///inputdata���ö�λ
	SeekStatus seek(const Time& seektime);

	///seek��λ������getdata�Ķ�λ��ȡ
	///inputdata���ö�λ
	SeekStatus seek(const std::string& file);

	OutputResult getData();

	InputResult putData(const MediaPackage* media);

	void setRecordFileEndFlag();
private:
	MediaCacheInternal*	internal;
};

};
};

#endif //__MEDIACACHE_H__
