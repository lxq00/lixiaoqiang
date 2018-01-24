#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "Base/MediaCache.h"
#include "Base/File.h"
#include "Base/Mutex.h"
#include "Base/Guard.h"
#include "Base/BaseTemplate.h"
#include "Base/PrintLog.h"
#include "Base/Timer.h"
#include "Base/FileBrowser.h"
#include "Base/String.h"
#include "Base/Singleton.h"
#include <math.h>
#include <vector>
#include <list>
#include <set>
#include <map>
using namespace std;
namespace Xunmei{
namespace Base{

#define MAXGOPSIZE		10
#define GOPABSSIZE		5
class CacheFileName
{
#define FILENAMETIMEFMT		"yyyy-MM-dd HH_mm_ss"
#define FILEINDEXNAME		"cachefileindex.dat"
public:
	CacheFileName(const  std::string& file):filepath(file)
	{
		creatDir(filepath);
	}
	CacheFileName(const  std::string& file,uint32_t did,int ID,Time start):channelID(ID),startTime(start)
		,filepath(file),devid(did)
	{
		creatDir(filepath);
	}
	~CacheFileName(){}
	std::string toString()
	{
		char filename[256];
		snprintf_x(filename,255,"%s%c%d",filepath.c_str(),PATH_SEPARATOR,devid);
		creatDir(filename);

		char timestr[128];
		startTime.format(timestr,FILENAMETIMEFMT);
		snprintf_x(filename,127,"%s%c%d%c%04d-%s.xmcache",filepath.c_str(),PATH_SEPARATOR,devid,PATH_SEPARATOR,channelID,timestr);

		return filename;
	}
	std::string getFileIndex()
	{
		return filepath + std::string("//") + std::string(FILEINDEXNAME);
	}
private:
	void creatDir(const std::string& dir)
	{
		if(!File::access(dir.c_str(),File::accessExist))
		{
			File::makeDirectory(dir.c_str());
		}
	}
private:
	int 		channelID;
	Time 		startTime;
	std::string filepath;
	uint32_t devid;
};
static uint64_t getMediaTimeStamp(const MediaPackage* media)
{
	if(media == NULL)
	{
		return 0;
	}
	MediaExtTime extime;
	if(!((MediaPackage*)media)->getExtraTimeStamp(extime))
	{
		return media->getTimeStamp()/1000;
	}
	Time mtime(extime.year,extime.month,extime.date,extime.hour,extime.minute,extime.second);

	return mtime.makeTime();
}
class CacheFile
{
#define CONTINUEMAXTIME		60
#define FileMaxIndexSize	60*60
#define CacheFileVersion	2
#define XMMDFILE			"XMCACHE"

#define PREVPACKFLAG		-1
#define INVILADFLAG			0

#define ISVIALDFLAG(x)	((x) != INVILADFLAG && (x) != PREVPACKFLAG)		

#define FILEHEADERSTARTPOS	0
#define FILEINDEXSTARTPOS	sizeof(CacheFileHeader)
#define FILESYSHEADERPOS	(sizeof(CacheFileHeader) + sizeof(CacheIndex)*FileMaxIndexSize)
#define FILEMEDIASTARTPOS	(sizeof(CacheFileHeader) + sizeof(CacheIndex)*FileMaxIndexSize + header.SystemHeaderSize)
public:
	struct CacheFileHeader
	{
		char 					Fxmmd[7];
		uint8_t 				Version;
		uint16_t  				SystemHeaderSize;
		uint16_t				Reserved;
		uint32_t				CheckSum;
	};
	struct CacheIndex
	{
		uint32_t 				FrameSN;
		uint32_t				FramePos; //一个小时一个文件/32位足够
	};
public:
	CacheFile(const std::string& file,uint64_t start,const MediaPackage*header):index(FileMaxIndexSize)
	{
		fileName = file;
		startTime = start;
		endTime = startTime + FileMaxIndexSize;

		CacheIndex setIndex = {0, 0};
		index.assign(FileMaxIndexSize, setIndex);

		bool fileReadSuccess = false;
		if(File::access(file.c_str(),File::accessExist))
		{
			if(readFileAndInit())
			{
				fileReadSuccess = true;
			}
		}

		if(!fileReadSuccess)
		{
			initFile(header);
		}

		spacePos = (uint32_t)filefd.getLength();
	}
	~CacheFile()
	{
		Guard locker(mutex);
		filefd.close();
	}
	MediaPackage* getMediaHeader()
	{
		Guard locker(mutex);
		
		if(header.SystemHeaderSize == 0)
		{
			return NULL;
		}

		return readMedia(FILESYSHEADERPOS);
	}

	MediaCache::SeekStatus checkIsHaveData(uint64_t seektime)
	{
		Guard locker(mutex);

		uint64_t indexTime = seektime - startTime;
		if (indexTime > FileMaxIndexSize)
		{
			return MediaCache::SeekStatus_NoData;
		}

		uint32_t readpos = index[(unsigned int)(seektime - startTime)].FramePos;

		return (readpos != 0)? MediaCache::SeekStatus_Ok : MediaCache::SeekStatus_NoData;
	}
	uint64_t getSkipGopTime(uint64_t readTime,bool& isFileEnd)
	{
		Guard locker(mutex);
		isFileEnd = false;
		
		if(readTime >= endTime)
		{
			isFileEnd = true;
			return 0;
		}

		uint64_t haveDataGop = 0;
		uint64_t haveDataTime 	= 0;
		uint64_t haveDataSn 	= 0;
		uint64_t checktimeend = readTime + MAXGOPSIZE*2;
		if(checktimeend > endTime)
		{
			checktimeend = endTime;
		}
		for(uint64_t time = readTime;time < checktimeend;time ++)
		{
			if(index[(unsigned int)(time - startTime)].FramePos > 0)
			{					
				if(haveDataTime == 0)
				{
					haveDataTime = time;
					haveDataSn = index[(unsigned int)(time - startTime)].FrameSN;
				}
				else if(index[(unsigned int)(time - startTime)].FrameSN != haveDataSn)
				{
					haveDataGop = time - haveDataTime;
					break;
				}
			}
			else if(haveDataTime > 0)
			{
				isFileEnd = (checktimeend == endTime);
				
				return 0;
			}
		}
		if(haveDataTime == 0)
		{
			isFileEnd = (checktimeend == endTime);
			return 0;
		}
		if(haveDataTime - readTime > haveDataGop + GOPABSSIZE)
		{
			return 0;
		}
		return haveDataTime;
	}
	bool getMedia(uint64_t readTime,std::list<MediaPackage*>& mediaList,uint64_t& nextTime)
	{
		Guard locker(mutex);

		mediaList.clear();
		if(readTime < startTime || readTime >= endTime)
		{
			return false;
		}
		uint32_t readpos = 0;
		for(uint64_t time = readTime;time < endTime;time ++)
		{
			readpos = index[(unsigned int)(time - startTime)].FramePos;
			if(readpos == INVILADFLAG)
			{
				return false;
			}
			else if(readpos != (uint32_t)PREVPACKFLAG)
			{
				break;
			}
		}


		uint64_t currMediaReadTime = 0;
		while(1)
		{
			MediaPackage* media = readMedia(readpos);
			if(media == NULL)
			{
				break;
			}
			if(media->getFrameType() != FRAMETYPE_IFrame && mediaList.size() == 0)
			{
				SAFE_DELETE(media);
				continue;
			}

			if(media->getFrameType() == FRAMETYPE_IFrame)
			{
				uint64_t mediaTime = getMediaTimeStamp(media);
				if(currMediaReadTime == 0)
				{
					currMediaReadTime = mediaTime;
				}
				else if(mediaTime != currMediaReadTime)
				{
					SAFE_DELETE(media);
					break;
				}
			}

			mediaList.push_back(media);
			readpos += media->size();
		}

		if(mediaList.size() > 0)
		{
			uint32_t possn = index[(unsigned int)(readTime - startTime)].FrameSN;
			nextTime = readTime;
			while(1)
			{
				if(nextTime >= endTime)
				{
					break;
				}
				if(index[(unsigned int)(nextTime - startTime)].FramePos == 0)
				{
					break;
				}
				uint32_t nowSn = index[(unsigned int)(nextTime - startTime)].FrameSN;
				if(nowSn != possn)
				{
					break;
				}
				nextTime ++;
			}
		}

		return mediaList.size() > 0;
	}

	bool settPrevPacketFlag(uint64_t mediaTime)
	{
		if(mediaTime < startTime || mediaTime >= endTime)
		{
			return false;
		}

		for(uint64_t time = startTime;time < mediaTime;time ++)
		{
			index[(unsigned int)(time - startTime)].FramePos = PREVPACKFLAG;
			index[(unsigned int)(time - startTime)].FrameSN = 0;
		}
		return true;
	}

	bool putData(uint64_t writeTime,std::list<MediaPackage*>& mediaList,uint64_t nexttime)
	{
		Guard locker(mutex);
		
		if(writeTime < startTime || writeTime >= endTime)
		{
			return false;
		}

		uint32_t mediapos = index[(unsigned int)(writeTime - startTime)].FramePos;
		if(ISVIALDFLAG((int32_t)mediapos))
		{
			//return false;
		}

		uint32_t wrietPost = spacePos;
		bool frameDataCheck = false;
		uint32_t frameId = 0;

		std::list<MediaPackage*>::iterator iter;
		for(iter = mediaList.begin();iter != mediaList.end();iter ++)
		{
			if((*iter)->getFrameType() == FRAMETYPE_IFrame)
			{
				uint64_t mediaTime = getMediaTimeStamp(*iter);
				if(mediaTime == writeTime && ! frameDataCheck)
				{
					frameDataCheck = true;
					frameId = (*iter)->getSequence();
				}
				else if(mediaTime > writeTime)
				{
					break;
				}
			}
			if(!frameDataCheck)
			{
				continue;
			}
			
			writeMedia(wrietPost,*iter);
			wrietPost += (*iter)->size();
		}

		if(frameDataCheck)
		{
			if(nexttime > endTime)
			{
				nexttime = endTime;
			}
			for(uint64_t time = writeTime; time < nexttime;time ++)
			{
				index[(unsigned int)(time - startTime)].FramePos = spacePos;
				index[(unsigned int)(time - startTime)].FrameSN = frameId;
			
				filefd.seek(FILEINDEXSTARTPOS + (time - startTime)*sizeof(CacheIndex),File::begin);
				filefd.write(&index[(unsigned int)(time - startTime)],sizeof(CacheIndex));
				filefd.flush();
			}
			
			spacePos = wrietPost;
		}
	
		return frameDataCheck;
	}
	uint64_t getNextEmptyBlockTime(uint64_t writetime)
	{
		Guard locker(mutex);

		if(writetime < startTime || writetime >= endTime)
		{
			return 0;
		}
		
		uint64_t lastCacheTime = 0;
		for(uint64_t i = writetime;i < endTime;i ++)
		{
			if(index[(unsigned int)(i - startTime)].FramePos == INVILADFLAG)
			{
				lastCacheTime = i;
				break;
			}
		}

		return lastCacheTime;
	}
	uint64_t getLastCacheTime(uint64_t readtime)
	{
		Guard locker(mutex);
		
		uint64_t lastCacheTime = 0;
		for(uint64_t i = readtime;i < endTime;i ++)
		{
			if(index[(unsigned int)(i - startTime)].FramePos == INVILADFLAG)
			{
				lastCacheTime = i - 1;
				break;
			}
		}

		return lastCacheTime;
	}
private:
	MediaPackage* readMedia(uint32_t pos)
	{
		if(filefd.seek(pos,File::begin) == 0)
		{
			return NULL;
		}
		char mediaheader[MEDIAHEADER_LENGTH];

		if(filefd.read(mediaheader,MEDIAHEADER_LENGTH) != MEDIAHEADER_LENGTH)
		{
			return NULL;
		}

		MediaPackage* media = new(std::nothrow) MediaPackage(mediaheader);
		if(media == NULL || !media->isValid())
		{
			SAFE_DELETE(media);

			return NULL;
		}

		int remainsize;

		while((remainsize = media->getRemainSize()) > 0)
		{
			char* remaindata = new(std::nothrow) char[remainsize];
			if(remaindata != NULL && filefd.read(remaindata,remainsize) != remainsize)
			{
				delete remaindata;
				break;
			}
			media->addRemainData(remaindata,remainsize);
			delete []remaindata;
		}

		return media;
	}

	bool writeMedia(uint32_t pos,const MediaPackage* media)
	{
		if(filefd.seek(pos,File::begin) == 0)
		{
			return false;
		}

		if(filefd.write(media->getBuffer(),((MediaPackage*)media)->size()) != (long)((MediaPackage*)media)->size())
		{
			return false;
		}

		return true;
	}
	bool initFile(const MediaPackage* headerMedia)
	{
		memset(&header,0,sizeof(CacheFileHeader));
		header.Version = CacheFileVersion;
		memcpy(header.Fxmmd,XMMDFILE,strlen(XMMDFILE));

		if(headerMedia != NULL)
		{
			header.SystemHeaderSize = (uint16_t)((MediaPackage*)headerMedia)->size();
		}

		if(!filefd.open(fileName.c_str(),File::modeReadWrite | File::modeCreate))
		{
			return false;
		}

		filefd.write(&header,sizeof(CacheFileHeader));
		for(int i = 0;i < FileMaxIndexSize;i ++)
		{
			filefd.write(&index[i],sizeof(CacheIndex));
		}
		if(headerMedia != NULL)
		{
			filefd.write(headerMedia->getBuffer(),((MediaPackage*)headerMedia)->size());
		}
		filefd.flush();

		return true;
	}
	bool readFileAndInit()
	{
		if(!filefd.open(fileName.c_str(),File::modeReadWrite))
		{
			return false;
		}
		uint64_t totallen =  filefd.getLength();
		if(totallen < sizeof(CacheFileHeader) + sizeof(CacheIndex)*FileMaxIndexSize)
		{
			filefd.close();
			return false;
		}
		filefd.seek(FILEHEADERSTARTPOS,File::begin);
		if(filefd.read(&header,sizeof(CacheFileHeader)) != sizeof(CacheFileHeader))
		{
			filefd.close();
			return false;
		}
		if(totallen < sizeof(CacheFileHeader) + sizeof(CacheIndex)*FileMaxIndexSize + header.SystemHeaderSize)
		{
			filefd.close();
			return false;
		}

		filefd.seek(FILEINDEXSTARTPOS,File::begin);
		for(int i = 0;i < FileMaxIndexSize;i ++)
		{
			if(filefd.read(&index[i],sizeof(CacheIndex)) != sizeof(CacheIndex))
			{
				filefd.close();
				return false;
			}
		}

		if(memcmp(header.Fxmmd,XMMDFILE,strlen(XMMDFILE)) != 0 || header.Version != CacheFileVersion)
		{
			filefd.close();
			return false;
		}

		return true;
	}
private:	
	Mutex 						mutex;
	CacheFileHeader				header;
	vector<CacheIndex>			index;

	std::string 				fileName;
	uint64_t					startTime;
	uint64_t					endTime;

	File						filefd;
	uint32_t					spacePos;
};

class CacheFileManager
{
public:
	struct FileInfo
	{
		uint64_t 				startTime;
		uint64_t				endTime;
	};
public:
	CacheFileManager():initStatus(false)
	{
		autoDeleteTimer = new(std::nothrow) Timer("CacheFileManager");
		if(autoDeleteTimer != NULL)
		{
			Timer::Proc proc(&CacheFileManager::autoManagerTimer,this);
			autoDeleteTimer->start(proc,0,60*1000);
		}
	}
	~CacheFileManager()
	{
		SAFE_DELETE(autoDeleteTimer);
		writeFileIndex();
	}

	CacheFile*	openCacheFile(const std::string& file,uint64_t start,const MediaPackage*header,void* userPtr)
	{
		Guard locker(cacheFileMutex);

		std::map<std::string,FileCacheItem>::iterator iter = cacheFileList.find(file);
		if(iter == cacheFileList.end())
		{
			FileCacheItem item;
			item.file = file;
			item.header = NULL;
			if(header != NULL)
			{
				item.header = ((MediaPackage*)header)->clone();
			}
			item.cacheFile = new(std::nothrow) CacheFile(file,start,header);
			cacheFileList[file] = item;

			iter = cacheFileList.find(file);
		}
		iter->second.usrePtrList.insert(userPtr);

		return iter->second.cacheFile;
	}
	void closeCacheFile(void* userPtr)
	{
		Guard locker(cacheFileMutex);

		std::map<std::string,FileCacheItem>::iterator iter1;
		std::map<std::string,FileCacheItem>::iterator iter;
		for(iter = cacheFileList.begin();iter != cacheFileList.end();iter = iter1)
		{
			iter1 = iter;
			iter1 ++;
			iter->second.usrePtrList.erase(userPtr);
			if(iter->second.usrePtrList.size() == 0)
			{
				SAFE_DELETE(iter->second.cacheFile);
				SAFE_DELETE(iter->second.header);
				cacheFileList.erase(iter);
			}
		}
	}
	
	void init(const std::string& file)
	{
		if(initStatus)
		{
			return;
		}
		Guard locker(cacheFileMutex);
		
		cachePath = file;
		CacheFileName name(cachePath);
		
		std::string filename = name.getFileIndex();

		FILE* fd = fopen(filename.c_str(),"rb");
		if(fd != NULL)
		{
			while(1)
			{
				FileIndexItem item;
				int readlen = fread(&item,1,sizeof(FileIndexItem),fd);
				if(readlen != sizeof(FileIndexItem))
				{
					break;
				}

				cacheFileIndex.insert(pair<std::string,FileIndexItem>(item.name,item));
			}
			fclose(fd);
		}

		initStatus = true;
	}
	bool getFileinfo(uint32_t devid,uint32_t channdlid,const std::string& file,FileInfo& info)
	{
		char buffer[256];
		snprintf_x(buffer,255,"%d_%d_%s",devid,channdlid,file.c_str());
		
		Guard locker(cacheFileMutex);
		std::map<std::string,FileIndexItem>::iterator iter = cacheFileIndex.find(buffer);
		if(iter == cacheFileIndex.end() || iter->second.startTime == 0)
		{
			return false;
		}

		Time nowtime = Time::getCurrentTime();
		
		info.startTime = iter->second.startTime;
		info.endTime = iter->second.endTime;
		iter->second.creatTime = nowtime.makeTime();
		
		return true;
	}

	bool setFileStartFlag(uint32_t devid,uint32_t channdlid,const std::string& file,uint64_t starttime)
	{
		char buffer[256];
		snprintf_x(buffer,255,"%d_%d_%s",devid,channdlid,file.c_str());
		
		Guard locker(cacheFileMutex);
		std::map<std::string,FileIndexItem>::iterator iter = cacheFileIndex.find(buffer);
		if(iter == cacheFileIndex.end())
		{
			FileIndexItem item;
			cacheFileIndex.insert(pair<std::string,FileIndexItem>(buffer,item));
			iter = cacheFileIndex.find(buffer);
		}

		Time nowtime = Time::getCurrentTime();

		snprintf_x(iter->second.name, 255,"%s", buffer);
		iter->second.startTime = starttime;
		iter->second.endTime = 0;
		iter->second.creatTime = nowtime.makeTime();

		return true;
	}

	bool setFileEndFlag(uint32_t devid,uint32_t channdlid,const std::string& file,uint64_t endtime)
	{
		char buffer[256];
		snprintf_x(buffer,255,"%d_%d_%s",devid,channdlid,file.c_str());
		
		Guard locker(cacheFileMutex);
		std::map<std::string,FileIndexItem>::iterator iter = cacheFileIndex.find(buffer);
		if(iter == cacheFileIndex.end() || iter->second.startTime == 0)
		{
			return false;
		}

		iter->second.endTime = endtime;

		return true;
	}
	
	static CacheFileManager* instance()
	{
		return CreateSingleton<CacheFileManager>(SINGLETON_Levl_Base,10);
	}
private:
	void writeFileIndex()
	{
		Guard locker(cacheFileMutex);
		CacheFileName name(cachePath);
		
		std::string filename = name.getFileIndex();

		FILE* fd = fopen(filename.c_str(),"wb+");
		if(fd == NULL)
		{
			return;
		}

		std::map<std::string,FileIndexItem>::iterator iter;
		for(iter = cacheFileIndex.begin();iter != cacheFileIndex.end();iter ++)
		{
			if(iter->second.startTime != 0)
			{
				fwrite(&iter->second,1,sizeof(FileIndexItem),fd);
			}			
		}

		fclose(fd);
	}
	void deleteByCreateTime(const std::string& path,uint64_t maxDay)
	{
		FileBrowser browser(path);
		if(!browser.isExist())
		{
			return;
		}
		browser.order(FileBrowser::OrderType_CrteatTime,FileBrowser::OrderMode_Asc);
		// 1:先删除时间比较老的文件、条件删除大于30天的缓存	MAXCACHEDAYS
		uint64_t nowtime = Time::getCurrentTime().makeTime();
		FileBrowser::Dirent dirent;
		for(uint32_t index = 0;index < FileBrowser::fileTotalSize(path);)
		{
			if(!browser.read(dirent,index))
			{
				break;
			}
			char pathtmp[512];
			snprintf_x(pathtmp,512,"%s%c%s",path.c_str(),PATH_SEPARATOR,dirent.Name.c_str());
			if(dirent.Type == FileBrowser::Dirent::DirentType_Dir)
			{
				deleteByCreateTime(pathtmp,maxDay);
				index ++;
				continue;
			}
			if(strcasecmp(dirent.SuffixName.c_str(),"xmcache") != 0)
			{
				index ++;
				continue;
			}
			if(nowtime - dirent.CreatTime.makeTime() < maxDay)
			{
				//时间比较老，后面的文件都会比较老，停止删除
				break;
			}
			else if(!isuseed(pathtmp))
			{
				logdebug("deleteByCreateTime file %s create and nowTime %llu maxday %llu and delete\r\n",pathtmp,nowtime - dirent.CreatTime.makeTime(),maxDay);
				//未在使用、删除、序号不添加
				bool ret = browser.remove(pathtmp);
				if(ret)
				{
					continue;
				}
			}

			index ++;
		}
	}
	void deleteByFreeSpace(const std::string& path,uint64_t minSpaceSize)
	{
		FileBrowser browser(path);
		if(!browser.isExist())
		{
			return;
		}
		browser.order(FileBrowser::OrderType_CrteatTime,FileBrowser::OrderMode_Asc);
		
		FileBrowser::Dirent dirent;
		uint32_t index = 0;
		while(1)
		{
			uint64_t freesize = 0;
			if(!FileBrowser::getFreeSpaceSize(path,freesize) || freesize < minSpaceSize)
			{
				break;
			}
			if(!browser.read(dirent,index))
			{
				break;
			}
			char pathtmp[512];
			snprintf_x(pathtmp,512,"%s%c%s",path.c_str(),PATH_SEPARATOR,dirent.Name.c_str());
			if(dirent.Type == FileBrowser::Dirent::DirentType_Dir)
			{
				deleteByFreeSpace(pathtmp,minSpaceSize);
				index ++;
				continue;
			}					
			if(strcasecmp(dirent.SuffixName.c_str(), "xmcache") != 0)
			{
				index ++;
				continue;
			}
			if(!isuseed(pathtmp))
			{
				logdebug("deleteByFreeSpace file %s freesize %llu maxfreesize %llu and delete\r\n",pathtmp,freesize,minSpaceSize);
				//未在使用、删除、序号不添加
				bool ret = browser.remove(pathtmp);
				if(ret)
				{
					continue;
				}
			}

			index ++;
		}
	}
	void deleteByUsedMaxSpace(const std::string& path,uint64_t maxSpaceSize)
	{
		FileBrowser browser(path);
		if(!browser.isExist())
		{
			return;
		}
		browser.order(FileBrowser::OrderType_CrteatTime,FileBrowser::OrderMode_Asc);
		
		FileBrowser::Dirent dirent;
		uint32_t index = 0;
		while(1)
		{
			uint64_t totalsize = FileBrowser::fileTotalSize(path);
			if(totalsize <= maxSpaceSize)
			{
				break;
			}
			if(!browser.read(dirent,index))
			{
				break;
			}
			char pathtmp[512];
			snprintf_x(pathtmp,512,"%s%c%s",path.c_str(),PATH_SEPARATOR,dirent.Name.c_str());
			if(dirent.Type == FileBrowser::Dirent::DirentType_Dir)
			{
				deleteByUsedMaxSpace(pathtmp,maxSpaceSize);
				index ++;
				continue;
			}	
			if(strcasecmp(dirent.SuffixName.c_str(), "xmcache") != 0)
			{
				index ++;
				continue;
			}
			if(!isuseed(pathtmp))
			{
				logdebug("deleteByUsedMaxSpace file %s totalsize %llu maxusedsize %llu and delete\r\n",pathtmp,totalsize,maxSpaceSize);
				
				//未在使用、删除、序号不添加
				bool ret = browser.remove(pathtmp);
				if(ret)
				{
					continue;
				}
			}

			index ++;
		}
	}
	void autoManagerTimer(unsigned long time)
	{
#define G  (((uint64_t)1024)*((uint64_t)1024)*((uint64_t)1024))
#define Day  ((uint64_t)(60*60*24));

		uint64_t MAXCACHEDAYS	=	30*Day;
		uint64_t MAXCACHESIZE	=	10*G;
		uint64_t MINSPACESIZE	=	5*G;

		if(!initStatus)
		{
			return;
		}
		
		// 1:先删除时间比较老的文件、条件删除大于30天的缓存	MAXCACHEDAYS
		deleteByCreateTime(cachePath,MAXCACHEDAYS);
		// 2:判断磁盘空间、条件让空间不少于5G		MINSPACESIZE
		deleteByFreeSpace(cachePath,MINSPACESIZE);
		// 3:清除多余缓存文件、条件删除大于10G的文件	MAXCACHESIZE
		deleteByUsedMaxSpace(cachePath,MAXCACHESIZE);
		
		{
			uint64_t nowtime = Time::getCurrentTime().makeTime();
			
			Guard locker(cacheFileMutex);
			std::map<std::string,FileIndexItem>::iterator iter;
			std::map<std::string,FileIndexItem>::iterator iter1;
			for(iter = cacheFileIndex.begin();iter != cacheFileIndex.end();iter = iter1)
			{
				iter1 = iter;
				iter1 ++;
				
				if(nowtime - iter->second.creatTime > MAXCACHEDAYS)
				{
					cacheFileIndex.erase(iter);
				}
			}
		}
		writeFileIndex();
	}
	bool isuseed(const std::string& file)
	{
		//缓存文件是否已经被占用
		Guard locker(cacheFileMutex);

		std::map<std::string,FileCacheItem>::iterator iter = cacheFileList.find(file);
		if(iter != cacheFileList.end())
		{
			return true;
		}

		return false;
	}
private:
	struct FileIndexItem
	{
		char					name[256];
		uint64_t 				startTime;
		uint64_t				endTime;
		uint64_t				creatTime;
	};
	struct FileCacheItem
	{
		std::string 						file;
		MediaPackage*						header;
		CacheFile*							cacheFile;
		std::set<void*>						usrePtrList;
	};
	Mutex 									cacheFileMutex;
	std::map<std::string,FileCacheItem> 	cacheFileList;
	std::map<std::string,FileIndexItem> 	cacheFileIndex;
	bool									initStatus;
	std::string								cachePath;
	Timer*									autoDeleteTimer;
};


struct MediaCache::MediaCacheInternal
{
#define MAXCACHEFRAMEID		100
public:
	MediaCacheInternal(const std::string& path,uint32_t dev,uint32_t channel):cachePath(path),deviceId(dev),channelID(channel)
	{
		header						= NULL;
		readTime					= 0;
		currReadCacheFileEndTime	= 0;
		currReadCacheFile			= NULL;
		writeTime					= 0;
		currWriteCacheFileEndTime	= 0;
		currWriteCacheFile			= NULL;
		gap							= 0;
		fileStartTime				= 0;
		fileEndTime					= 0;
		fileNotExist				= true;
		fileIsEnd					= false;
		getFromWriteFifo			= false;
		getFromWriteFifoDateEnd 	= false;


		CacheFileManager::instance()->init(path);
	}
	~MediaCacheInternal()
	{	
		Guard locker(cacheMutex);
		writeAllWriteListData(writeTime + 1);
		SAFE_DELETE(header);
		cleanAllReadList();
		cleanAllWriteList();

		CacheFileManager::instance()->closeCacheFile(this);		
	}
	void setSystemHeader(const MediaPackage* xmheader)
	{
		Guard locker(cacheMutex);
		if(xmheader != NULL && header == NULL)
		{
			header = ((MediaPackage*)xmheader)->clone();
		}
	}

	MediaPackage* getSystemHeader()
	{
		Guard locker(cacheMutex);
		if(header == NULL)
		{
			return NULL;
		}

		MediaPackage* xmh = header->clone();

		return xmh;
	}

	SeekStatus seek(const Time& seektime)
	{
		Guard locker(cacheMutex);

		uint64_t seektotime = seektime.makeTime();

		notLoadReadData = false;
		nextReadTime = seektotime;
		readTime = seektotime;
		
		getCurrReadFile(seektotime,true);
		
		if(currReadCacheFile == NULL)
		{
			return SeekStatus_NoData;
		}

		return currReadCacheFile->checkIsHaveData(seektotime);
	}
	SeekStatus seek(const std::string& name)
	{
		filename = name;
		CacheFileManager::FileInfo info = {};

		if(!CacheFileManager::instance()->getFileinfo(deviceId,channelID,filename,info))
		{
			return SeekStatus_NoData;
		}
		fileNotExist = false;
		fileIsEnd = (info.endTime != 0)?true:false;
		fileStartTime = info.startTime;
		fileEndTime = info.endTime;

		Time time;
		time.breakTime(info.startTime);



		return seek(time);
	}

	OutputResult getData()
	{
		Guard locker(cacheMutex);

		OutputResult ret = {OutputResult::OutputStatus_CurrHaveNoData,NULL,0};

		if(!loadReadData(ret))
		{
			return ret;
		}
		
		ret.data = readList.front();
		readList.pop_front();

		uint64_t lasttime = 0;
		if(!getFromWriteFifo || getFromWriteFifoDateEnd)
		{
			uint64_t currreadtime = readTime;
			do{
				uint64_t hour				= currreadtime / (60*60);
				uint64_t owntime			= hour*(60*60);

				CacheFileName filename(cachePath,deviceId,channelID,owntime);

				CacheFile* cacheFile = CacheFileManager::instance()->openCacheFile(filename.toString(),owntime,header,this);
				if(cacheFile == NULL)
				{
					lasttime = owntime;
				}
				else
				{
					lasttime = cacheFile->getLastCacheTime(currreadtime);
				}
				if(lasttime == 0)
				{
					currreadtime += 60*60;
				}
			}while(lasttime == 0);
		}
		else
		{
			lasttime = readTime;
		}

		ret.lastCachedTime.breakTime(lasttime);
		ret.status = OutputResult::OutputStatus_OK;

		return ret;
	}

	InputResult putData(MediaPackage* media)
	{
		InputResult ret = {0};
		if(media == NULL)
		{
			return ret;
		}
		Guard locker(cacheMutex);
		if(!checkIsWrite(media))
		{
			return ret;
		}

		MediaPackage* data = media->clone();
		writeList.push_back(data);
		if(getFromWriteFifo && !getFromWriteFifoDateEnd)
		{
			if(readTime == writeTime)
			{
				MediaPackage* data = media->clone();
				readList.push_back(data);
			}
			else
			{
				getFromWriteFifoDateEnd = true;
				nextReadTime = readTime + 1;
			}
		}
		uint64_t currwritetime = writeTime;
		uint64_t lasttime = 0;
		do{
			uint64_t hour				= currwritetime / (60*60);
			uint64_t owntime			= hour*(60*60);

			CacheFileName filename(cachePath,deviceId,channelID,owntime);

			CacheFile* cacheFile = CacheFileManager::instance()->openCacheFile(filename.toString(),owntime,header,this);
			if(cacheFile == NULL)
			{
				lasttime = owntime;
			}
			else
			{
				lasttime = cacheFile->getNextEmptyBlockTime(currwritetime);
			}
			if(lasttime == 0)
			{
				currwritetime += 60*60;
			}
		}while(lasttime == 0);

		ret.NextEmptyTime.breakTime(lasttime);

		return ret;
	}
	void setRecordFileEndFlag()
	{
		fileIsEnd = true;
		fileEndTime = writeTime;
		CacheFileManager::instance()->setFileEndFlag(deviceId,channelID,filename,writeTime);
		writeAllWriteListData(writeTime + 1);
		cleanAllWriteList();
	}
private:
	void writeAllWriteListData(uint64_t mediatime)
	{
		/*
				1:当流媒体数据小于正在写的时间时、我们可以认为是外部SEEK了，
				2:为了保证当前写的内容完整、抛弃当前数据
				3:当流媒体时间大于当前写的时间、可以认为是下一个数据包，也可以
				   当然也可能是seek当当前以后的位置
				4:所以我们用GAP的范围来忽略掉
				5:当GAP不存在时，我们先获取GAP
			*/
			//GAP两个I帧间隔时间、最大为GAPMAXLEN 秒
			//先获取GAP
			
		#define GAPMAXLEN		120
			
		uint64_t midtime = mediatime - writeTime;
		if(midtime < GAPMAXLEN)
		{
			gap = (gap > midtime || gap == 0)?midtime:gap;
		}
		//if(gap != 0)
		{
			//当GAP范围在ABSGAPSIZE秒内都表示为一帧数据
	#define ABSGAPSIZE		30
			if(midtime < ABSGAPSIZE)
			{
				//记录文件信息
				if(fileNotExist)
				{
					fileStartTime = fileStartTime == 0 ? mediatime : (fileStartTime < mediatime ? fileStartTime : mediatime);
					fileEndTime = fileEndTime == 0 ? mediatime : (fileStartTime > mediatime ? fileStartTime : mediatime);
				}
				if(filename != "" && fileStartTime != 0 && fileNotExist)
				{
					CacheFileManager::instance()->setFileStartFlag(deviceId,channelID,filename,fileStartTime);
				}

				//开始写数据
				currWriteCacheFile->putData(writeTime,writeList,mediatime);

				if(mediatime >= currWriteCacheFileEndTime)
				{
					getCurrWriteFile(mediatime);
					currWriteCacheFile->settPrevPacketFlag(mediatime);
				}
			}
		}
		
		if(getFromWriteFifo && !getFromWriteFifoDateEnd)
		{
			getFromWriteFifoDateEnd = true;
			nextReadTime = mediatime;
		}
	}
	bool checkIsWrite(MediaPackage* media)
	{
		if(media->getFrameType() != FRAMETYPE_IFrame)
		{
			if(writeTime != 0)
			{
				return true;
			}
			return false;
		}
		uint64_t mediatime = getMediaTimeStamp(media);
		
		if(mediatime == 0)
		{
			return false;
		}
		if(mediatime == writeTime)
		{
			return true;
		}
		else if(writeTime != 0)// && mediatime > writeTime)
		{
			writeAllWriteListData(mediatime);
		}
		cleanAllWriteList();
		getCurrWriteFile(mediatime);

		return true;
	}
	void getCurrWriteFile(uint64_t mediaTime)
	{
		writeTime = mediaTime;
		if(currWriteCacheFileEndTime == 0 || currWriteCacheFile == NULL || mediaTime >= currWriteCacheFileEndTime || mediaTime < currWriteCacheFileEndTime-60*60)
		{
			uint64_t hour				= mediaTime / (60*60);
			uint64_t owntime			= hour*(60*60);

			CacheFileName filename(cachePath,deviceId,channelID,owntime);

			currWriteCacheFile = CacheFileManager::instance()->openCacheFile(filename.toString(),owntime,header,this);
			currWriteCacheFileEndTime = (hour + 1)*(60*60);
		}
	}
	bool loadReadData(OutputResult& ret)
	{
		if(readTime == 0)
		{
			ret.status = OutputResult::OutputStatus_NotSuccessSeek;
			return false;
		}
		if(notLoadReadData)
		{
			if(readList.size() > 0)
			{
				return true;
			}
			if(getFromWriteFifo && !getFromWriteFifoDateEnd)
			{
				//还在等待写的队列数据
				return false;
			}
			readTime = nextReadTime;
		}
		getFromWriteFifo = false;
		getFromWriteFifoDateEnd = false;
		
		//如果文件存在、并在读到最后了 不能在读了
		if(!fileNotExist && readTime >= fileEndTime && fileIsEnd)
		{
			OutputResult ret1 = {OutputResult::OutputStatus_EndOfFile,NULL,0};

			ret =  ret1;

			return false;
		}
		notLoadReadData = false;

		while(1)
		{
			getCurrReadFile(readTime);

			if(currReadCacheFile->getMedia(readTime,readList,nextReadTime))
			{
				checkReadListIsHaveRead();
				if(readList.size() > 0)
				{
					break;
				}
				readTime = nextReadTime;
				continue;
			}
			else
			{
				//可能情况，读取位置还没到文件结束，已经没数据，所以返回失败，不能在翻文件
				uint64_t hour	= readTime / (60*60);
				if(nextReadTime < (hour+1)*(60*60))
				{
					//没数据了，并且不再文件结束
					if(readTime == writeTime)
					{
						//如果写的队列有读的数据，去读写的队列
						getFromWriteFifo = true;
						getFromWriteFifoDateEnd = false;

						std::list<MediaPackage*>::iterator iter;
						for(iter = writeList.begin();iter != writeList.end();iter ++)
						{
							if(*iter == NULL)
							{
								continue;
							}
							MediaPackage* packet = (*iter)->clone();
							if(packet == NULL)
							{
								continue;
							}
							readList.push_back(packet);								
						}	
						notLoadReadData = true;
					}
					
					return false;
				}
			}

			bool isendfile = false;
			uint64_t nextgoptime = 0;
			uint64_t currReadTime = readTime;
			bool  firstskip = true;
			while(1)
			{
				uint64_t hour				= currReadTime / (60*60);
				uint64_t owntime			= hour*(60*60);
				
				CacheFileName filename(cachePath,deviceId,channelID,owntime);

				CacheFile* cacheFile = CacheFileManager::instance()->openCacheFile(filename.toString(),owntime,header,this);
				if(cacheFile == NULL)
				{
					return false;
				}

				nextgoptime = cacheFile->getSkipGopTime(currReadTime,isendfile);
				if(nextgoptime == 0)
				{
					if(isendfile && firstskip)
					{
						firstskip = false;
						currReadTime = owntime + 60*60;
						continue;
					}
					return false;
				}

				readTime = nextgoptime;
				break;
			}
		}
		notLoadReadData = true;
		return true;
	}
	void getCurrReadFile(uint64_t time,bool seek = false)
	{
		cleanAllReadList();

		if(seek)
		{
			cleanAllReadFlag();
		}

		readTime					= time;
		if(time >= currReadCacheFileEndTime - 60*60 && time < currReadCacheFileEndTime && currReadCacheFile != NULL)
		{			
			return;
		}

		uint64_t hour				= time / (60*60);
		uint64_t owntime			= hour*(60*60);
		
		currReadCacheFileEndTime	= (hour + 1)*(60*60);
		
		CacheFileName filename(cachePath,deviceId,channelID,owntime);

		currReadCacheFile = CacheFileManager::instance()->openCacheFile(filename.toString(),owntime,header,this);
		if(currReadCacheFile == NULL)
		{
			return;
		}
	}
	void cleanAllReadList()
	{
		std::list<MediaPackage*>::iterator iter;
		for(iter = readList.begin();iter != readList.end();iter ++)
		{
			SAFE_DELETE(*iter);
		}
		readList.clear();
		notLoadReadData = false;
	}
	void cleanAllReadFlag()
	{
		vreadFreamSn.clear();
		areadFrameSn.clear();
	}
	void addReadFlag(MediaPackage* media)
	{
		std::list<uint32_t>* list = &vreadFreamSn;
		if(media->getFrameType() == FRAMETYPE_Audio)
		{
			list = &areadFrameSn;
		}
		else
		{
			list = &vreadFreamSn;
		}
		list->push_back(media->getSequence());
		while(list->size() > MAXCACHEFRAMEID)
		{
			list->pop_front();
		}
		
	}
	bool isHaveReadThisMedia(MediaPackage* media)
	{
		std::list<uint32_t>* list = &vreadFreamSn;
		if(media->getFrameType() == FRAMETYPE_Audio)
		{
			return false;
			//list = &areadFrameSn;
		}
		else
		{
			list = &vreadFreamSn;
		}

		std::list<uint32_t>::iterator iter;
		for(iter = list->begin();iter != list->end();iter ++)
		{
			if(*iter == media->getSequence())
			{
				return true;
			}
		}

		return false;
	}
	void checkReadListIsHaveRead()
	{
		while(readList.size() > 0)
		{
			MediaPackage* media = readList.front();
			if(media == NULL || isHaveReadThisMedia(media))
			{
				SAFE_DELETE(media);
				readList.pop_front();
				continue;
			}
			break;
		}
		std::list<MediaPackage*>::iterator iter;
		for(iter = readList.begin();iter != readList.end();iter ++)
		{
			addReadFlag(*iter);
		}
	}
	void cleanAllWriteList()
	{
		std::list<MediaPackage*>::iterator iter;
		for(iter = writeList.begin();iter != writeList.end();iter ++)
		{
			SAFE_DELETE(*iter);
		}
		writeList.clear();
	}
public:
	Mutex				cacheMutex;
	std::string			cachePath;
	uint32_t			deviceId;
	uint32_t			channelID;

	MediaPackage*		header;

	uint64_t				readTime;
	uint64_t  				nextReadTime;
	bool					notLoadReadData;
	std::list<MediaPackage*>	readList;
	std::list<uint32_t>		vreadFreamSn;
	std::list<uint32_t> 	areadFrameSn;

	uint64_t				currReadCacheFileEndTime;
	CacheFile*				currReadCacheFile;


	uint64_t				writeTime;
	std::list<MediaPackage*>	writeList;

	uint64_t				currWriteCacheFileEndTime;
	CacheFile*				currWriteCacheFile;
	uint64_t 				gap; 

	//文件处理
	std::string				filename;
	uint64_t				fileStartTime;
	uint64_t				fileEndTime;
	bool					fileNotExist;
	bool					fileIsEnd;
	bool					getFromWriteFifo;
	bool 					getFromWriteFifoDateEnd;
};

MediaCache::MediaCache(const std::string& cachePath,uint32_t devId,int channelID)
{
	internal = new(std::nothrow) MediaCacheInternal(cachePath,devId,channelID);
}
MediaCache::~MediaCache()
{
	SAFE_DELETE(internal);
}
void MediaCache::setSystemHeader(const MediaPackage* header)
{
	internal->setSystemHeader(header);
}
MediaPackage* MediaCache::getSystemHeader()
{
	return internal->getSystemHeader();
}
MediaCache::SeekStatus MediaCache::seek(const Time& seektime)
{
	return internal->seek(seektime);
}

MediaCache::SeekStatus MediaCache::seek(const std::string& file)
{
	return internal->seek(file);
}

MediaCache::OutputResult MediaCache::getData()
{
	return internal->getData();
}
MediaCache::InputResult MediaCache::putData(const MediaPackage* media)
{
	return internal->putData((MediaPackage*)media);
}

void MediaCache::setRecordFileEndFlag()
{
	return internal->setRecordFileEndFlag();
}

};
};
