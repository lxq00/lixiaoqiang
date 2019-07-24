#include "OnvifClient/OnvifClient.h"
#include "HTTP/HTTPClient.h"
#include "protocol/OnvifProtocol.h"
using namespace Public::HTTP;

namespace Public {
namespace Onvif {


struct OnvifClientManager::OnvifClientManagerInternal
{
	std::string useragent;
	shared_ptr<IOWorker> worker;
};


OnvifClientManager::OnvifClientManager(const shared_ptr<IOWorker>& worker, const std::string& userContent)
{
	internal = new OnvifClientManagerInternal;
	internal->useragent = userContent;
	internal->worker = worker;
	if (internal->worker == NULL)
	{
		internal->worker = make_shared<IOWorker>(2);
	}
}
OnvifClientManager::~OnvifClientManager()
{
	SAFE_DELETE(internal);
}

shared_ptr<OnvifClient> OnvifClientManager::create(const URL& url)
{
	return shared_ptr<OnvifClient>(new OnvifClient(internal->worker, url, internal->useragent));
}

}
}

