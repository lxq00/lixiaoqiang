#include "boost/asio.hpp"
#include "boost/enable_shared_from_this.hpp"
#include "boost/make_shared.hpp"
#include "boost/bind.hpp"

#include "Network/Strand.h"


namespace Public {
namespace Network {

struct StrandInternalCallbackObj
{
	Strand::StrandCallback handler;
	shared_ptr<Strand::StrandData> data;

	static void strandCallback(const shared_ptr<StrandInternalCallbackObj>& object)
	{
		shared_ptr<StrandInternalCallbackObj> tmp = object;
		if (tmp == NULL) return;

		tmp->handler(tmp->data);
	}
};
struct Strand::StrandInternal
{
	boost::asio::io_service::strand strand;
	StrandInternal(const shared_ptr<IOWorker>& ioworker) :strand(*(boost::asio::io_service*)ioworker->getBoostASIOIOServerPtr()){}
};
Strand::Strand(const shared_ptr<IOWorker>& ioworker)
{
	internal = new StrandInternal(ioworker);
}
Strand::~Strand()
{
	SAFE_DELETE(internal);
}

void Strand::post(const Strand::StrandCallback& handler, const shared_ptr<Strand::StrandData>& data)
{
	shared_ptr< StrandInternalCallbackObj> object = make_shared<StrandInternalCallbackObj>();
	object->handler = handler;
	object->data = data;

	internal->strand.post(boost::bind(StrandInternalCallbackObj::strandCallback, object));
}

}
}


