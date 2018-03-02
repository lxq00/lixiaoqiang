#ifndef __SIMPLEIO_REACTOR_H__
#define __SIMPLEIO_REACTOR_H__

#include "simpleio_def.h"

class Reactor_Object;
class Reactor
{
public:
	typedef void* Handle;
public:
	Reactor():usercount(0){}
	virtual ~Reactor() {}

	virtual Handle addSocket(const Public::Base::shared_ptr<Reactor_Object>& obj) = 0;
	virtual void delSocket(Reactor_Object* obj,Handle handle) = 0;

	virtual bool addConnect(const Public::Base::shared_ptr<Reactor_Object>& obj) = 0;
	virtual void delConnect(Reactor_Object* obj) = 0;

	virtual void setInEvent(const Public::Base::shared_ptr<Reactor_Object>& obj, Handle handle) = 0;
	virtual void clearInEvent(const Public::Base::shared_ptr<Reactor_Object>& obj, Handle handle) = 0;
	virtual void setOutEvent(const Public::Base::shared_ptr<Reactor_Object>& obj, Handle handle) = 0;
	virtual void clearOutEvent(const Public::Base::shared_ptr<Reactor_Object>& obj, Handle handle) = 0;
	virtual void setErrEvent(const Public::Base::shared_ptr<Reactor_Object>& obj, Handle handle) = 0;
	virtual void clearErrEvent(const Public::Base::shared_ptr<Reactor_Object>& obj, Handle handle) = 0;

	long long userRef() { return usercount; }
protected:
	AtomicCount		usercount;
};


#endif //__SIMPLEIO_REACTOR_H__
