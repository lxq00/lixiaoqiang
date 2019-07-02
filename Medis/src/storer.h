#pragma once

#include "Base/Base.h"
using namespace Public::Base;

#define STOREFLAG	"mdis"

#define MINSPACELEN	 10

//存储器对象
class Storer
{
	struct EmptyInfo
	{
		uint32_t		pos;
		uint32_t		len;

		bool operator < (const EmptyInfo& info) const
		{
			return pos < info.pos;
		}
	};

	struct StoreHeader
	{
		char flag[4];
		enum {
			HeaderType_None,
			HeaderType_Data,
		}type;
		uint32_t len;
		uint32_t headerlen;
		uint32_t datalen;

		StoreHeader()
		{
			memcpy(flag, STOREFLAG, 4);
			len = headerlen = datalen = 0;
		}
	};
public:
	Storer() :fd(NULL) {}
	~Storer()
	{
		close();
	}

	bool open(const std::string& filename)
	{
		Guard locker(mutex);

		fd = fopen(filename.c_str(), "rb+");
		if (fd == NULL)
		{
			File::save(filename, "");
			fd = fopen(filename.c_str(), "rb+");
		}
		if (fd == NULL) return false;

		return true;
	}
	bool close()
	{
		Guard locker(mutex);

		if (fd != NULL)
		{
			fclose(fd);
			fd = NULL;
		}

		return true;
	}

	bool scanHeader(std::map<uint32_t, std::string>& headers)
	{
		Guard locker(mutex);

		if (fd == NULL) return false;

		fseek(fd, 0, SEEK_END);
		uint32_t totalsize = ftell(fd);
		
		uint32_t nowpos = 0;
		while (1)
		{
			fseek(fd, nowpos, SEEK_SET);

			StoreHeader header;
			int readlen = fread(&header, 1, sizeof(StoreHeader), fd);
			if (readlen != sizeof(StoreHeader)) break;

			if (memcmp(header.flag, STOREFLAG, 4) != 0) break;
			if (header.len > totalsize -  nowpos) break;
			if (header.len < header.datalen + header.headerlen + sizeof(StoreHeader))
			{
				assert(0);
				break;
			}
			if (header.len == 0)
			{
				assert(0);
				break;
			}
			if (header.type == StoreHeader::HeaderType_None)
			{
				EmptyInfo info;
				info.pos = nowpos;
				info.len = header.len;

				emptylist.push_back(info);
			}
			else
			{
				if (header.len < header.headerlen + header.datalen + sizeof(StoreHeader)) break;

				char* buffertmp = new char[header.headerlen];
				if (buffertmp == NULL) break;

				int readlen = fread(buffertmp, 1, header.headerlen, fd);
				if (readlen != header.headerlen)
				{
					SAFE_DELETEARRAY(buffertmp);
					break;
				}

				headers[nowpos] = std::string(buffertmp, header.headerlen);
				SAFE_DELETEARRAY(buffertmp);
			}

			nowpos += header.len;
		}

		return true;
	}
	uint32_t write(const std::string& header,const std::string& data, uint32_t pos)
	{
		Guard locker(mutex);

		uint32_t nowneedlen = sizeof(StoreHeader) + header.length() + data.length();

		uint32_t spacelen = 0;
		if (pos == -1 || !checkCurrPosSpaceCanSave(pos, nowneedlen, spacelen))
		{
			if (!allocSapce(nowneedlen, pos, spacelen))
			{
				return -1;
			}
		}

		if (spacelen < nowneedlen || !saveToSpace(header, data, pos, spacelen))
		{
			assert(0);
			return -1;
		}
		return pos;
	}
	uint32_t read(uint32_t pos, uint32_t len, std::string& data)
	{
		Guard locker(mutex);

		fseek(fd, pos, SEEK_SET);

		StoreHeader header;
		int readlen = fread(&header, 1, sizeof(StoreHeader), fd);
		if (readlen != sizeof(StoreHeader)) return 0;

		if (memcmp(header.flag, STOREFLAG, 4) != 0) return 0;
		if (header.type == StoreHeader::HeaderType_None) return 0;
		if (header.len < header.headerlen + header.datalen) return 0;
		if (header.datalen != len) return 0;

		fseek(fd, pos + sizeof(StoreHeader) + header.headerlen, SEEK_SET);

		char buffer[1024];
		
		while (data.length() < header.datalen)
		{
			uint32_t canreadlen = min(1024, header.datalen - data.length());
			int readlen = fread(buffer, 1, canreadlen, fd);
			if (readlen <= 0)
			{
				return 0;
			}

			data += std::string(buffer, readlen);
		}

		return data.length();
	}
	bool remove(uint32_t pos)
	{
		Guard locker(mutex);

		fseek(fd, pos, SEEK_SET);

		StoreHeader header;
		int readlen = fread(&header, 1, sizeof(StoreHeader), fd);
		if (readlen != sizeof(StoreHeader)) return false;

		if (memcmp(header.flag, STOREFLAG, 4) != 0) return false;
		if (header.type == StoreHeader::HeaderType_None) return false;
		

		{
			fseek(fd, pos, SEEK_SET);

			header.type = StoreHeader::HeaderType_None;
			fwrite(&header, 1, sizeof(StoreHeader), fd);
			fflush(fd);
		}
		
		{
			EmptyInfo info;
			info.pos = pos;
			info.len = header.len;
			emptylist.push_back(info);
		}

		return true;
	}
	void mergeFreeSpace()
	{
		Guard locker(mutex);

		emptylist.sort();
		for (std::list<EmptyInfo>::iterator iter = emptylist.begin(); iter != emptylist.end();)
		{
			std::list<EmptyInfo>::iterator curriter = iter;
			std::list<EmptyInfo>::iterator nextiter = ++iter;
			if (nextiter == emptylist.end()) break;

			//合并
			if (curriter->pos + curriter->len == nextiter->pos)
			{
				curriter->len = curriter->len + nextiter->len;

				{
					StoreHeader header;
					header.type = StoreHeader::HeaderType_None;
					header.len = curriter->len;
					fseek(fd, curriter->pos, SEEK_SET);
					fwrite(&header, 1, sizeof(StoreHeader), fd);
					fflush(fd);
				}
				emptylist.erase(nextiter);

				iter = curriter;
			}
		}
	}
private:
	bool checkCurrPosSpaceCanSave(uint32_t pos, uint32_t needlen, uint32_t& totallen)
	{
		fseek(fd, pos, SEEK_SET);

		StoreHeader header;
		int readlen = fread(&header, 1, sizeof(StoreHeader), fd);
		if (readlen != sizeof(StoreHeader)) return false;

		if (memcmp(header.flag, STOREFLAG, 4) != 0) return false;
		if (header.type == StoreHeader::HeaderType_None) return false;

		if (header.len < needlen) return false;

		if (header.len - needlen > sizeof(StoreHeader) + MINSPACELEN)
		{
			uint32_t freepos = pos + needlen;

			//currheader
			{
				StoreHeader fileheader = header;
				fileheader.len = needlen;
				fseek(fd, pos, SEEK_SET);
				fwrite(&fileheader, 1, sizeof(StoreHeader), fd);
			}
			//next freeheader
			{
				StoreHeader fileheader;
				fileheader.type = StoreHeader::HeaderType_None;
				fileheader.len = header.len - needlen;
				fseek(fd, freepos, SEEK_SET);
				fwrite(&fileheader, 1, sizeof(StoreHeader), fd);

				//add emptyinfo
				{
					EmptyInfo info;
					info.pos = freepos;
					info.len = fileheader.len;
					emptylist.push_back(info);
				}
			}
			fflush(fd);

			totallen = needlen;
		}
		else
		{
			totallen = header.len;
		}
		
		return true;
	}
	bool allocSapce(uint32_t needlen, uint32_t& pos,uint32_t& totallen)
	{
		fseek(fd, 0, SEEK_END);
		pos = ftell(fd);
		totallen = needlen;

		return true;
	}
	bool saveToSpace(const std::string& headerstr, const std::string& data, uint32_t pos, uint32_t totallen)
	{
		StoreHeader header;
		header.type = StoreHeader::HeaderType_Data;
		header.len = totallen;
		header.headerlen = headerstr.length();
		header.datalen = data.length();

		fseek(fd, pos, SEEK_SET);
		int ret = fwrite(&header, 1, sizeof(StoreHeader), fd);
		ret = fwrite(headerstr.c_str(), 1, headerstr.length(), fd);
		ret = fwrite(data.c_str(), 1, data.length(), fd);
		fflush(fd);

		return true;
	}
private:
	Mutex						mutex;
	std::list<EmptyInfo>		emptylist;
	FILE*						fd;
};
