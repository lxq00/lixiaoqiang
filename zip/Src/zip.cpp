#include "zip/zip.h"
#include "miniZip/zip.h"

#define READBUFFERSIZE 4096
#define PATHFLAG "\\"

namespace Public {
namespace Zip {

struct ZipObject::ZipObjectInternal
{
public:
	ZipObjectInternal() 
	{
		m_zfile = NULL;
	}

	~ZipObjectInternal() 
	{
		if (m_zfile != NULL)
		{
			zipCloseFileInZip(m_zfile);
			zipClose(m_zfile, NULL);
		}

		File::remove(m_zipName);
	}

	bool create(const std::string& passwd)
	{
		if (m_zfile != NULL)
		{
			zipClose(m_zfile, NULL);
		}

		m_currBinPath = File::getExcutableFileFullPath();
		m_zipName = m_currBinPath + PATHFLAG + Guid::createGuid().getStringStream() + ".zip";

		m_zfile = zipOpen64(m_zipName.c_str(), NULL);
		if (m_zfile == NULL)
		{
			return false;
		}

		m_password = passwd;

		return true;
	}
	bool saveToMem(std::string& data)
	{
		if (m_zfile != NULL)
		{
			zipClose(m_zfile, NULL);
			m_zfile = NULL;
		}

		FILE* pfile = fopen(m_zipName.c_str(), "rb");
		if (pfile == NULL)
		{
			return false;
		}

		char buffer[READBUFFERSIZE] = {};
		int readSize = 0;
		bool result = true;
		do 
		{
			readSize = fread(buffer, 1, READBUFFERSIZE, pfile);
			if (readSize > 0)
			{
				data += string(buffer, readSize);
			}

			if (readSize < 0)
			{
				result = false;
				break;
			}
		} while (readSize > 0);

		fclose(pfile);

		File::remove(m_zipName);

		return result;
	}
	bool saveToFile(const std::string& filename)
	{
		if (m_zfile != NULL)
		{
			zipClose(m_zfile, NULL);
			m_zfile = NULL;
		}
		
		std::vector<string> vec = String::split(filename, PATHFLAG);
		int pos = String::lastIndexOf(filename, PATHFLAG);
		if (vec.size() == 1 && pos == filename.length() - 1)
		{
			//filename 直接是文件名，保存在当前目录；
			File::rename(m_zipName, m_currBinPath + PATHFLAG + filename);
			return true;
		}

		if (vec.size() != 1)
		{
			//filename带着目录,挨个创建, vec.back()是文件名
			std::string des_path;
			for (uint32_t i = 1; i < vec.size() - 1; i++)
			{
				des_path += vec[i];
				if (!File::access(des_path, File::accessExist))
				{
					File::makeDirectory(des_path);
				}
			}

			std::string des_filename = vec.back();
		
			File::copy(m_zipName, filename);
			File::remove(m_zipName);
		}

		return true;
	}

	bool addItemFromMem(const std::string& itemname, const std::string& data)
	{
		if (m_zfile == NULL)
		{
			return false;
		}

		zip_fileinfo fileinfo = {};
		_filetime("" ,fileinfo.tmz_date);

		uint32_t crc = 0;
		if (m_password != "")
		{
			_getCrc(itemname.c_str(), itemname.length(), crc);
		}

		int err = zipOpenNewFileInZip4(m_zfile, itemname.c_str(), &fileinfo, NULL, 0, NULL, 0, NULL, Z_DEFLATED, 8, 0, -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY, m_password.c_str(), crc, 0, 0);
		if (err != ZIP_OK)
		{
			return false;
		}

		err = zipWriteInFileInZip(m_zfile, data.c_str(), data.length());
		if (err != ZIP_OK)
		{
			return false;
		}

		zipCloseFileInZip(m_zfile);

		return true;
	}

	bool addItemFromFile(const std::string& itemname, const std::string& filename)
	{
		if (m_zfile == NULL)
		{
			return false;
		}

		zip_fileinfo fileinfo = {};
		_filetime(filename, fileinfo.tmz_date);
		uint32_t crc = 0;
		if (m_password != "")
		{
			_getFileCrc(filename.c_str(), crc);
		}
		
		int err = zipOpenNewFileInZip4(m_zfile, itemname.c_str(), &fileinfo, NULL, 0, NULL, 0, NULL, Z_DEFLATED, 8, 0, -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY, m_password.c_str(), crc, 0, 0);
		if (err != ZIP_OK)
		{
			return false;
		}

		FILE* pfile = fopen(filename.c_str(), "rb");
		if (pfile == NULL)
		{
			return false;
		}

		char buffer[READBUFFERSIZE] = {};
		int readSize = 0;
		bool result = true;
		do
		{
			readSize = fread(buffer, 1, READBUFFERSIZE, pfile);
			if (readSize > 0)
			{
				err = zipWriteInFileInZip(m_zfile, buffer, readSize);
				if (err != ZIP_OK)
				{
					result = false;
					break;
				}
			}

			if (readSize < 0)
			{
				result = false;
				break;
			}
		} while (readSize > 0);

		fclose(pfile);
		zipCloseFileInZip(m_zfile);

		return true;
	}
private:
	bool _filetime(const std::string& filePath, tm_zip& fileTime)
	{
		Time time = Time::getCurrentTime();
		if (filePath != "")
		{
			FileInfo info;
			if (!File::stat(filePath, info))
			{
				return false;
			}

			time = info.timeWrite;
		}
	
		fileTime.tm_year = time.year;
		fileTime.tm_mon = time.month - 1;
		fileTime.tm_mday = time.day;
		fileTime.tm_hour = time.hour;
		fileTime.tm_min = time.minute;
		fileTime.tm_sec = time.second;
		return true;
	}

	uint32_t _getCrc(const char* buffer, int buffersize, uint32_t& result_crc)
	{
		result_crc = crc32(result_crc, (const Bytef*)buffer, buffersize);

		return result_crc;
	}

	bool _getFileCrc(const char* filenameinzip, uint32_t& result_crc)
	{
		uint32_t  calculate_crc = 0;
		FILE * pfile = fopen(filenameinzip, "rb");
		if (pfile == NULL)
		{
			return false;
		}

		unsigned long size_read = 0;

		char * buffer = new char[512 * 1024 * 1024];
		do
		{
			memset(buffer, 0, 512 * 1024 * 1024);
			size_read = fread(buffer, 1, 512 * 1024 * 1024, pfile);
			if (size_read > 0)
			{
				calculate_crc = _getCrc(buffer, size_read, calculate_crc);
			}

		} while (size_read > 0);

		if (pfile != NULL)
		{
			fclose(pfile);
		}
		SAFE_DELETE(buffer);

		result_crc = calculate_crc;
		return true;

	}
private:
	zipFile     m_zfile;
	std::string m_currBinPath;
	std::string m_zipName;
	std::string m_password;
};

ZipObject::ZipObject()
{
	internal = new ZipObjectInternal();
}

ZipObject::~ZipObject()
{
	SAFE_DELETE(internal);
}

bool ZipObject::create(const std::string& passwd)
{
	return internal->create(passwd);
}
bool ZipObject::saveToMem(std::string& data)
{
	return internal->saveToMem(data);
}
bool ZipObject::saveToFile(const std::string& filename)
{
	return internal->saveToFile(filename);
}

bool ZipObject::addItemFromMem(const std::string& itemname, const std::string& data)
{
	return internal->addItemFromMem(itemname, data);
}

bool ZipObject::addItemFromFile(const std::string& itemname, const std::string& filename)
{
	return internal->addItemFromFile(itemname, filename);
}

}
}