#ifndef __SIPTRANSACTION_H__
#define __SIPTRANSACTION_H__
#include "SIPStack/SIPStackDefs.h"
#include "SIPStack/SIPDispatcher.h"
#include "Base/Base.h"
#include "Network/Network.h"

namespace Public{
namespace SIPStack {

class SIPDispatcher;
class SIP_API SIPTransaction
{
	struct SIPTransactionInternal;
public:
	enum SIPTransactionType{
			SIPTransactionType_UDP,
			SIPTransactionType_TCP,
		};
public:
	SIPTransaction(SIPTransactionType type,uint32_t myport,const std::string& uAgent,const weak_ptr<SIPDispatcher>& dispatcher);
	~SIPTransaction();	
	uint32_t getMyPort() const;
	SIPTransactionType getTransType() const;
	void* getExosip() const;
	bool	checkSocketInAlive(int sock);
private:
	SIPTransactionInternal* internal;
};

};
};

#endif //__SIPTRANSACTION_H__
