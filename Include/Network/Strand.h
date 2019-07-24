#ifndef __NETWORK_STRAND_H__
#define __NETWORK_STRAND_H__
#include "Socket.h"

namespace Public {
namespace Network {

//让完成事件处理器的调用，在同一时间只有一个
//确保单线程执行
class NETWORK_API Strand
{
public:
	struct NETWORK_API StrandData {
		virtual ~StrandData() {}
	};
	typedef Function1<void, const shared_ptr<StrandData>& > StrandCallback;
public:
	Strand(const shared_ptr<IOWorker>& ioworker);
	virtual ~Strand();

	void post(const Strand::StrandCallback& handler, const shared_ptr<Strand::StrandData>& data);
private:
	struct StrandInternal;
	StrandInternal* internal;
};

}
}


#endif //__NETWORK_STRAND_H__
