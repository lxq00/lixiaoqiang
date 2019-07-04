#if 0
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
#endif

#if 0
#include "Medis/Medis.h"
using namespace Public::Medis;

#define WORKPORT 6379

shared_ptr<IOWorker>		worker;
shared_ptr<Service>			service;


void systemExit(void*, BaseSystem::CloseEvent)
{
	service = NULL;
	worker = NULL;


	Public::Network::NetworkSystem::uninit();
	Public::Base::BaseSystem::uninit();

	Public::Base::BaseSystem::autoExitDelayer(30);
}
int main(int argc, const char* argv[])
{
	Public::Base::BaseSystem::init(systemExit);
	Public::Network::NetworkSystem::init();


	worker = make_shared<IOWorker>(16);
	service = make_shared<Service>();

	if (!service->start(worker, WORKPORT))
	{
		int a = 0;
		return -1;
	}

	ConsoleCommand cmd;
	cmd.run("medis");

	return 0;
}
#endif

#if 1
#include "OnvifClient/OnvifClient.h"
using namespace Public::Onvif;

int main()
{
	shared_ptr<IOWorker> worker = make_shared<IOWorker>(4);

	shared_ptr<OnvifClientManager> manager = make_shared<OnvifClientManager>(worker,"test");

	shared_ptr<OnvifClient> client = manager->create(URL("admin:admin@192.168.13.33"));

//-	shared_ptr<OnvifClientDefs::Info> info = client->getInfo();

//-	shared_ptr<OnvifClientDefs::Capabilities> cap = client->getCapabities();	//获取设备能力集合，错误信息使用XM_GetLastError捕获

	//shared_ptr<OnvifClientDefs::Scopes> scopes = client->getScopes(); //获取描述信息，错误信息使用XM_GetLastError捕获


//-	shared_ptr<OnvifClientDefs::Profiles> profile = client->getProfiles(); //获取配置信息，错误信息使用XM_GetLastError捕获
//	std::string getStreamUrl(const std::string& streamtoken, int timeoutms = 10000); //获取六信息,错误信息使用XM_GetLastError捕获
//	std::string getSnapUrl(const std::string& snaptoken, int timeoutms = 10000);	//获取截图信息，错误信息使用XM_GetLastError捕获

	shared_ptr<OnvifClientDefs::NetworkInterfaces> network = client->getNetworkInterfaces();//网络信息，错误信息使用XM_GetLastError捕获
	shared_ptr<OnvifClientDefs::VideoEncoderConfigurations> enc = client->getVideoEncoderConfigurations(); //获取视频编码信息，错误信息使用XM_GetLastError捕获
	shared_ptr<OnvifClientDefs::ContinuousMove> move = client->getContinuousMove(); //错误信息使用XM_GetLastError捕获
	shared_ptr<OnvifClientDefs::AbsoluteMove> abs = client->getAbsoluteMove(); //错误信息使用XM_GetLastError捕获
	shared_ptr<OnvifClientDefs::_PTZConfig> config = client->getConfigurations(); //错误信息使用XM_GetLastError捕获
	shared_ptr<OnvifClientDefs::ConfigurationOptions> opt = client->getConfigurationOptions(); //错误信息使用XM_GetLastError捕获
	shared_ptr<Time> time = client->GetSystemDatetime(); //错误信息使用XM_GetLastError捕获

	getchar();

	return 0;
}
#endif