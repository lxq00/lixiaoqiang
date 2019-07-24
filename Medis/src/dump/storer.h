#pragma once

#include "Base/Base.h"
using namespace Public::Base;

#define STOREFLAG	"mdis"

#define MINSPACELEN	 10

//´æ´¢Æ÷¶ÔÏó
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
		uint32_t headerlen;
		uint32_t datalen;

		StoreHeader()
		{
			memcpy(flag, STOREFLAG, 4);
			headerlen = datalen = 0;
		}
	};
public:
	Storer() :fd(NULL) {}
	~Storer()
	{
		close();
	}

	bool open(const std::string& filename,bool save)
	{
		fd = fopen(filename.c_str(), save ? "wb+" : "rb");
		if (fd == NULL)
		{
			return false;
		}

		return true;
	}
	bool close()
	{
		if (fd != NULL)
		{
			fclose(fd);
			fd = NULL;
		}

		return true;
	}

	bool load(std::map<std::string, RedisString>& data)
	{
		if (fd == NULL) return false;

		while (1)
		{
			StoreHeader header;
			int readlen = fread(&header, 1, sizeof(StoreHeader), fd);
			if (readlen != sizeof(StoreHeader)) break;

			if (memcmp(header.flag, STOREFLAG, 4) != 0) break;
			
			std::string headerstr;
			RedisString datastr;
			{
				char* buffertmp = new char[header.headerlen];
				if (buffertmp == NULL) break;

				int readlen = fread(buffertmp, 1, header.headerlen, fd);
				if (readlen != header.headerlen)
				{
					SAFE_DELETEARRAY(buffertmp);
					break;
				}

				headerstr = std::string(buffertmp, header.headerlen);
				SAFE_DELETEARRAY(buffertmp);
			}
			{
				char* buffertmp = new char[header.datalen];
				if (buffertmp == NULL) break;

				int readlen = fread(buffertmp, 1, header.datalen, fd);
				if (readlen != header.datalen)
				{
					SAFE_DELETEARRAY(buffertmp);
					break;
				}

				datastr = std::string(buffertmp, header.datalen);
				SAFE_DELETEARRAY(buffertmp);
			}

			data[headerstr] = datastr;
		}

		return true;
	}
	bool write(const std::string& headerstr,const RedisString& data)
	{
		if (fd == NULL) return false;
		StoreHeader header;
		header.headerlen = headerstr.length();
		header.datalen = data.length();

		int ret = fwrite(&header, 1, sizeof(StoreHeader), fd);
		ret = fwrite(headerstr.c_str(), 1, headerstr.length(), fd);

		if(data.length() > 0)
			ret = fwrite(data.ptr(), 1, data.length(), fd);

		return true;
	}
private:
	FILE*						fd;
};
