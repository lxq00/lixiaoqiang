#ifndef __NETWORK_STRAND_H__
#define __NETWORK_STRAND_H__
#include "Socket.h"

namespace Public {
namespace Network {

//������¼��������ĵ��ã���ͬһʱ��ֻ��һ��
//ȷ�����߳�ִ��
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
