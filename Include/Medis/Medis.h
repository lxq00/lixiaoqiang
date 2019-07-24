#pragma once
#include "Defs.h"
#include "Base/Base.h"
#include "Network/Network.h"

namespace Public {
namespace Medis {

using namespace Public::Base;
using namespace Public::Network;


class MEDIS_API Service
{
	struct ServiceInternal;
public:
	Service();
	~Service();
	bool start(const shared_ptr<IOWorker>& worker, uint32_t port);
	bool stop();
private:
	ServiceInternal* internal;
};

}
}
