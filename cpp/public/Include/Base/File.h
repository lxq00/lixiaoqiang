//
//  Copyright (c)1998-2012, Public Technology
//  All Rights Reserved.
//
//	Description:
//	$Id: File.h 252 2013-12-18 04:40:28Z linyang $
//

#ifndef __BASE_FILE_H__
#define __BASE_FILE_H__

#include <string>
#ifdef WIN32
#include <io.h>
#endif

#include "Base/Defs.h"
#include "Base/IntTypes.h"

namespace Public {
namespace Base {

/// \struct FileInfo
/// \brief �ļ���Ϣ�ṹ
struct FileInfo
{
	enum {maxPathSize = 256};
	char		name[maxPathSize];	///< �ļ���
	uint16_t	attrib;			///< �ļ�����
	uint64_t	timeCreate;	///< �ļ�����ʱ��
	uint64_t	timeAccess;	///< �ļ�����ʱ��
	uint64_t	timeWrite;		///< �ļ��޸�ʱ��
	uint64_t	size;			///< �ļ���С
};

#ifdef WIN32
#define PATH_SEPARATOR '\\'
#else
#define PATH_SEPARATOR '/'
#endif // WIN32

/// \class File
/// \brief �ļ������װ
class BASE_API File
{
	File(File const&);
	File& operator=(File const&);

public:

	/// �ļ��򿪱�־
	enum OpenFlags {
		modeRead =         (int) 0x00000, ///< Ϊ����һ���ļ�,����ļ������ڻ��޷���,Open����ʧ��.
		modeWrite =        (int) 0x00001, ///< Ϊд����һ�����ļ�,����ļ�����,�����ݽ�������.
		modeReadWrite =    (int) 0x00002, ///< Ϊ��д��һ���ļ�,����ļ������ڻ��޷���,Open����ʧ��.
		shareCompat =      (int) 0x00000,
		shareExclusive =   (int) 0x00010,
		shareDenyWrite =   (int) 0x00020,
		shareDenyRead =    (int) 0x00030,
		shareDenyNone =    (int) 0x00040,
		modeNoInherit =    (int) 0x00080,
		modeCreate =       (int) 0x01000, ///< ����ļ�������,�Զ������ļ�,��modeReadWrite���ʹ��.
		modeNoTruncate =   (int) 0x02000, ///< ��modeCreateһ��ʹ��,���Ҫ�������ļ��Ѿ�����,ԭ�������ݱ���.
		typeText =         (int) 0x04000, // typeText and typeBinary are
		typeBinary =       (int) 0x08000, // used in derived classes only
		osNoBuffer =       (int) 0x10000, ///< �ϲ㴦����,��ʱ��д��ƫ�ƺͳ��ȶ���Ҫ��ҳ���С����.
		osWriteThrough =   (int) 0x20000,
		osRandomAccess =   (int) 0x40000,
		osSequentialScan = (int) 0x80000,
	};

	/// �ļ�����
	enum Attribute {
		normal =    0x00,
		readOnly =  0x01,
		hidden =    0x02,
		system =    0x04,
		volume =    0x08,
		directory = 0x10,
		archive =   0x20
	};

	/// �ļ���λ�ο�λ��
	enum SeekPosition
	{
		begin = 0x0,
		current = 0x1,
		end = 0x2
	};

	/// �ļ����ʷ�ʽ
	enum AccessMode
	{
		accessExist = 0,
		accessWrite = 2,
		accessRead = 4,
		accessReadWrite = 6,
	};

	/// ���캯��.
	File();

	/// ��������.������ݻ��廹��Ч,�ᱻ�ͷ�.
	virtual ~File();

	/// ���ļ�.�򿪺��ļ�ָ��ƫ����0��,����modeAppend��־�򿪵��ļ��ļ�ָ�����ļ�ĩβ.
	/// \param  pFileName [in]�ļ���.
	/// \param  dwFlags [in]�򿪱�־,Ĭ��ΪmodeReadWrite.
	/// \retval true  �򿪳ɹ�
	/// \retval false  ��ʧ��,�ļ������ڻ��޷���.
	virtual bool open(const char* pFileName, uint32_t dwFlags = modeReadWrite);

	/// �ر��ļ�.
	virtual void close();

	/// װ������,������ļ�����һ����С�Ļ���,���ļ����ݶ����û���,���ػ���ָ��.
	/// ��unload����һ���ṩ����
	/// \param  pFileName [in] �ļ���.
	/// \retval NULL  װ��ʧ��
	/// \retval !NULL  ���ݻ���ָ��.
	virtual uint8_t* load(const char* pFileName);

	/// �ͷ����ݻ���
	virtual void unload();

	/// ���ļ�����.���������ļ�ָ���Զ��ۼ�.
	/// \param pBuffer [out]  ���ݻ����ָ��.
	/// \param dwCount [in]  Ҫ�������ֽ���
	/// \retval >=0  �������ֽ���
	/// \retval <0 ��ʧ��
	virtual long read(void* pBuffer, size_t dwCount);

	/// д�ļ�����.���������ļ�ָ���Զ��ۼ�.
	/// \param pBuffer [in]  ���ݻ����ָ��.
	/// \param dwCount [in]  Ҫд����ֽ���
	/// \retval >=0  д����ֽ���
	/// \retval <0 дʧ��
	virtual long write(void *pBuffer, size_t dwCount);

	/// ͬ���ļ��ײ㻺��,��д���������,ȷ��д��������Ѿ���������ϵͳ.
	virtual void flush();

	/// �ļ���λ.(Todo:Unix��֧��4G�����ļ�)
	/// \param lOff [in]  ƫ����,�ֽ�Ϊ��λ.
	/// \param nFrom [in]  ƫ�����λ��,���õ���ƫ��ΪlOff+nFrom.
	/// \retval ƫ�ƺ��ļ���ָ��λ��.
	virtual uint64_t seek(int64_t lOff, SeekPosition nFrom);

	/// ��õ�ǰ���ļ�ָ��λ��
	/// \retval �ļ�ָ��λ��.
	virtual uint64_t getPosition();

	/// ����ļ�����
	/// \retval �ļ�����
	virtual uint64_t getLength();

	/// ���ı��ļ���ǰƫ�ƴ���ȡһ���ַ���.���������ļ�ָ���Զ��ۼ�.
	/// \param s [out]  ���ݻ���.
	/// \param size [in]  ��Ҫ��ȡ���ַ�������.
	/// \retval NULL  ��ȡʧ��
	/// \retval !NULL  �ַ���ָ��.
	virtual char* gets(char* s, size_t size);

	/// ���ı��ļ���ǰƫ�ƴ�д��һ���ַ���.д�������ļ�ָ���Զ��ۼ�.
	/// \param s [in] ���ݻ���.
	/// \retval  ʵ��д���ַ�������.
	virtual long puts(const char* s);

	/// �ж��ļ��Ƿ��
	/// \retval true �Ѿ���
	/// \retval false û�д�
	virtual bool isOpen();

	/// ��ȡ��ǰ��ִ�г�����ļ���
	/// \param s [out]  ���ݻ���.
	/// \param size [in]  ��Ҫ��ȡ���ַ�������.
	/// \retval ʵ�ʵõ����ַ�������,ʧ�ܷ���-1
	static size_t getExcutableFileName(char* s, size_t size);

	/// ��ȡ��ǰ��ִ�г����ļ��ľ���·��
	/// \param s [out]  ���ݻ���.
	/// \param size [in]  ��Ҫ��ȡ���ַ�������.
	/// \retval ʵ�ʵõ����ַ�������,ʧ�ܷ���-1
	static size_t getExcutableFileFullPath(char* s, size_t size);

	/// �������ļ�
	/// \param oldName [in] �ɵ��ļ���
	/// \param oldName [in] �µ��ļ���
	/// \retval true  �ɹ�
	/// \retval false ʧ��
	static bool rename(const char* oldName, const char* newName);

	/// ɾ���ļ�
	/// \param fileName [in] �ļ���
	/// \retval true  �ɹ�
	/// \retval false ʧ��
	static bool remove(const char* fileName);

	/// ����Ŀ¼
	/// \param dirName [in] Ŀ¼��
	/// \retval true  �ɹ�
	/// \retval false ʧ��
	static bool makeDirectory(const char* dirName);

	/// ɾ��Ŀ¼
	/// \param dirName [in] Ŀ¼��
	/// \retval true  �ɹ�
	/// \retval false ʧ��
	static bool removeDirectory(const char* dirName);

	/// �ļ�ϵͳͳ��
	/// \param path [in]����·��,��һ���Ƿ�����Ŀ¼.
	/// \param userFreeBytes [out]Ŀ¼�����ļ�ϵͳ��������û���ʣ��ռ��ֽ���
	/// \param totalBytes [out]Ŀ¼�����ļ�ϵͳ�ܵ��ֽ���
	/// \param totalFreeBytes [out]Ŀ¼�����ļ�ϵͳ�ܵ�ʣ��ռ��ֽ���,���ʹ����Ӳ��
	///        �ռ����,userFreeBytes���ܻ��totalFreeBytesС
	/// \retval true  �ɹ�
	/// \retval false ʧ��
	static bool statFS(const char* path,
		uint64_t& userFreeBytes,
		uint64_t& totalBytes,
		uint64_t& totalFreeBytes);

	/// �ж��ļ���Ŀ¼����Ȩ��
	/// \param path [in]�ļ���Ŀ¼��·��.
	/// \param mode [in]����Ȩ��,\see AccessMode
	/// \retval true  ��Ȩ��
	/// \retval false û��Ȩ��
	static bool access(const char* path, int mode);

	/// ����·����ȡ�ļ���Ϣ
	/// \param path [out]�ļ���Ŀ¼��·��.
	/// \param info [out]�ļ���Ϣ,\see FileInfo
	/// \retval true  �ɹ�
	/// \retval false ʧ��
	static bool stat(const char* path, FileInfo& info);

protected:
	struct FileInternal;
	FileInternal* internal;
};

////////////////////////////////////////////////////////////////////////////////

/// \class FileFind
/// \brief �ļ�������,֧��'*','?'ͨ�������
class BASE_API FileFind
{
	FileFind(FileFind const&);
	FileFind& operator=(FileFind const&);

public:
	/// ���캯��
	FileFind();

	/// ��������
	virtual ~FileFind();

	/// ���ҵ�һ���ļ�
	/// \param fileName [in] ����ͨ�����·����
	/// \retval ture �ҵ�
	/// \retval false û���ҵ�
	virtual bool findFile(const char* fileName);

	/// ������һ���ļ�,ʹ�ú��ϴ�findFile��ͬ������,������findFile֮�����(Linux��������)
	/// \retval ture �ҵ�
	/// \retval false û���ҵ�
	virtual bool findNextFile();

	/// �رղ���,�رպ�����ٴε���findFile
	virtual void close();

	/// �õ����ҵ����ļ��ĳ���
	/// \retval �ļ�����
	virtual size_t getLength();

	/// �õ����ҵ����ļ����ļ���
	/// \retval �ļ�����
	virtual std::string getFileName();

	/// �õ����ҵ����ļ���ȫ·��
	/// \retval ȫ·��
	virtual std::string getFilePath();

	/// �Ƿ�Ϊֻ���ļ�
	/// \retval true ֻ��
	/// \retval false ����ֻ��
	virtual bool isReadOnly();

	/// �Ƿ�ΪĿ¼�ļ�
	/// \retval true ��Ŀ¼
	/// \retval false ����Ŀ¼
	virtual bool isDirectory();

	/// �Ƿ�Ϊ�����ļ�
	/// \retval true ��
	/// \retval false ����
	virtual bool isHidden();

	/// �Ƿ�Ϊ��ͨ�ļ�
	/// \retval true ��
	/// \retval false ����	
	virtual bool isNormal();

protected:
	struct FileFindInternal;
	FileFindInternal* internal;
};

} // namespace Base
} // namespace Public

#endif //__BASE_FILE_H__

