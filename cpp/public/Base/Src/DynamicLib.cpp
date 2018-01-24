#include "Base/DynamicLib.h"
#ifdef WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace Public {
namespace Base {

struct DynamicLib::DynamicLibInternal
{
#ifdef WIN32
	HMODULE		handle;
#else
	void*		handle;
#endif
};

DynamicLib::DynamicLib()
{
	internal = new DynamicLibInternal;
	internal->handle = NULL;
}
DynamicLib::~DynamicLib()
{
	if(internal->handle != NULL)
	{
#ifdef WIN32
		FreeLibrary(internal->handle);
#else
		dlclose(internal->handle);
#endif
	}
	delete internal;
}

bool DynamicLib::load(const std::string& libFile)
{
	if(internal->handle != NULL)
	{
		return false;
	}
#ifdef WIN32
	internal->handle = LoadLibraryExA(libFile.c_str(),NULL,LOAD_WITH_ALTERED_SEARCH_PATH);
#else
	internal->handle = dlopen(libFile.c_str(),RTLD_LAZY);
#endif
	
	return internal->handle != NULL;
}
void* DynamicLib::getProcAddr(const std::string& procName)
{
	if(internal->handle == NULL)
	{
		return NULL;
	}
#ifdef WIN32
	return GetProcAddress(internal->handle,procName.c_str());
#else
	return dlsym(internal->handle,procName.c_str());
#endif
}
};
};

