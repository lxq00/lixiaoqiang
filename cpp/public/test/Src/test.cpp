#include "Base/Func.h"
using namespace Public::Base;
#include <functional>
#include <memory>
#include "HTTP/HTTPClient.h"
typedef void(*ptrtype)(int);

struct Test
{
	void testfunc(int) {}
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
#include "boost/regex.hpp"
int main()
{
	std::string url = "/api/entities/11";
	std::string math = "^/api/entities/.+";

	boost::regex  regex (math);

	if (boost::regex_match(url, regex))
	{
		int a = 0;
	}
	else
	{
		int b = 0;
	}

	//Public::HTTP::HTTPClient client("http://192.168.0.11");
	//shared_ptr<Public::HTTP::HTTPResponse> respse = client.request("get");

	int a = 0;


	/*ifstream infile;
	infile.open("test.vcxproj.user", std::ofstream::in | std::ofstream::binary);
	if (!infile.is_open()) return false;

	while (!infile.eof())
	{
		char buffer[1024];
		streamsize readlen = infile.readsome(buffer, 1024);
		if (readlen == 0) break;
	}


	infile.close();*/


	//std::shared_ptr<Test> testptr(new Test);

	//Function1<void, int> testfunc = Function1<void, int>(&Test::testfunc, testptr.get());

	//test(1);
	
	//Function2<void,int, int> f = std::bind(&Test::testfunc, std::weak_ptr<Test>(t).lock(), std::placeholders::_1, std::placeholders::_2);

	//Function1<void, int> f1 = [&](int) {
	//	int a = 0;
	//};

	//std::function<void(int)> test1 = f1;

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