#include "Base/Func.h"
using namespace Public::Base;
#include <functional>
#include <memory>

typedef void(*ptrtype)(int);

struct Test
{
	void testfunc(int,int) {}
};

template <typename Function>
struct function_traits : public function_traits < decltype(&Function::operator()) >
{

};

template <typename ClassType, typename ReturnType, typename Args, typename Args2>
struct function_traits < ReturnType(ClassType::*)(Args, Args2) const >
{
	typedef ReturnType(*pointer)(Args, Args2);
	typedef std::function<ReturnType(Args, Args2)> function;
};

int main()
{
	std::shared_ptr<Test> t(new Test);

	//std::function<void(int)> test = std::bind(&Test::testfunc,t, std::placeholders::_1);

	//test(1);
	
	Function2<void,int, int> f = std::bind(&Test::testfunc, std::weak_ptr<Test>(t).lock(), std::placeholders::_1, std::placeholders::_2);

	Function1<void, int> f1 = [&](int) {
		int a = 0;
	};

	std::function<void(int)> test1 = f1;

	/*ptrtype ptrtmp = test;

	if (test == test1)
	{
		int a = 0;
	}
	if (test != test1)
	{
		int b = 0;
	}*/
	//std::map < std::string, Host::NetworkInfo > infos;
	//std::string defaultMac;

	//Host::getNetworkInfos(infos, defaultMac);

	return 0;
}