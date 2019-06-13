#include "boost/asio.hpp"
#include "boost/enable_shared_from_this.hpp"
#include "boost/make_shared.hpp"
#include "boost/bind.hpp"

#include "Network/Strand.h"


namespace Public {
namespace Network {

struct StrandInternalCallbackObj :public boost::enable_shared_from_this<StrandInternalCallbackObj>
{
	Mutex						mutex;
	std::list<Function0<void> > handerlist;

	void strandCallback()
	{
		Function0<void> hander;

		{
			Guard locker(mutex);
			if (handerlist.size() <= 0) return;
			hander = handerlist.front();
			handerlist.pop_front();
		}

		hander();
	}
};
struct Strand::StrandInternal
{
	boost::asio::io_service::strand strand;
	boost::shared_ptr<StrandInternalCallbackObj> callbackobj;

	StrandInternal(const shared_ptr<IOWorker>& ioworker) :strand(**(boost::shared_ptr<boost::asio::io_service>*)ioworker->getBoostASIOIOServerSharedptr())
	{
		callbackobj = boost::make_shared<StrandInternalCallbackObj>();
	}
	~StrandInternal() { callbackobj = NULL; }
};
Strand::Strand(const shared_ptr<IOWorker>& ioworker)
{
	internal = new StrandInternal(ioworker);
}
Strand::~Strand()
{
	SAFE_DELETE(internal);
}

void Strand::post(const Function0<void>& handler)
{
	Guard locker(internal->callbackobj->mutex);
	internal->callbackobj->handerlist.push_back(handler);

	internal->strand.post(boost::bind(&StrandInternalCallbackObj::strandCallback, internal->callbackobj->shared_from_this()));
}

}
}


