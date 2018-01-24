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
		MediaPackage*	data;	//当！=NULL 时需要外部delete
		Time			lastCachedTime;	//剩余录像视频时间
	};

	struct InputResult
	{
		Time NextEmptyTime;			///下一个空数据库时间/当==0 一直下载
	};
public:
	MediaCache(const std::string& cachePath,uint32_t devId,int channelID);
	~MediaCache();

	void setSystemHeader(const MediaPackage* header);
	
	///获取系统头，当!= NULL的时候需要用户自己delete
	MediaPackage* getSystemHeader();

	///seek定位、用于getdata的定位获取
	///inputdata不用定位
	SeekStatus seek(const Time& seektime);

	///seek定位、用于getdata的定位获取
	///inputdata不用定位
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
