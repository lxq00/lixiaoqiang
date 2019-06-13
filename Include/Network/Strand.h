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
	Strand(const shared_ptr<IOWorker>& ioworker);
	virtual ~Strand();

	void post(const Function0<void>& handler);
private:
	struct StrandInternal;
	StrandInternal* internal;
};

}
}


#endif //__NETWORK_STRAND_H__
