#ifndef __URLENCODING_H__
#define __URLENCODING_H__
#include "Base/Defs.h"
#include <string>
using namespace std;

namespace Xunmei{
namespace Base{

class BASE_API URLEncoding
{
public:
	URLEncoding(){}
	~URLEncoding(){}

	static std::string encode(const std::string& url);
	static std::string decode(const std::string& enurl);
};

}	
}

#endif //__URLENCODING_H__
