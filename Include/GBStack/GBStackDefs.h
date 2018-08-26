#ifndef __GBSTACK_DEFS_H__
#define __GBSTACK_DEFS_H__


namespace Public{
namespace GB28181{


#ifdef GB28181_BUILD
#define GB28181_API __declspec(dllexport)
#else 
#ifdef GB28181_USED
#define GB28181_API __declspec(dllimport)
#else
#define GB28181_API
#endif
#endif


};
};



#endif //__GBSTACK_DEFS_H__
