#ifndef __DYNAMICLIB_H__
#define __DYNAMICLIB_H__

#include "Defs.h"
#include <string>
using namespace std;
namespace Public {
namespace Base {

class BASE_API DynamicLib
{
	struct DynamicLibInternal;
public:
	DynamicLib();
	~DynamicLib();

	bool load(const std::string& libFile);
	void* getProcAddr(const std::string& procName);
private:
	DynamicLibInternal* internal;
};

};
};

#endif //__DYNAMICLIB_H__
