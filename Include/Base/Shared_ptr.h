#ifndef __shared_ptr_H__
#define __shared_ptr_H__
//
//  Copyright (c)1998-2012, Public Technology
//  All Rights Reserved.
//
//	Description:
//	$Id: shared_ptr.h 3 2013-01-21 06:57:38Z  $
//   智能指针
#include "Base/AtomicCount.h"
#include "Base/IntTypes.h"
#include "Base/Defs.h"
#include <assert.h>


#ifdef GCCSUPORTC11
#include <memory>
#endif

namespace Public{
namespace Base{

#ifndef GCCSUPORTC11
#ifdef WIN32
#if _MSC_VER >= 1600  //VS2012
#define BUILDERVERNEWTHEN2012
#endif
#else
#define _NOEXCEPT	throw ()
#define _STD ::std::
#define _TRY_BEGIN	try {
#define _CATCH(x)	} catch (x) {
#define _CATCH_ALL	} catch (...) {
#define _CATCH_END	}
#define _RERAISE	throw
#endif

class _Ref_count_base
{	
private:
	virtual void _Destroy() = 0;
	virtual void _Delete_this() = 0;

private:
	AtomicCount _Uses;
	AtomicCount _Weaks;

protected:
	_Ref_count_base():_Uses(1),_Weaks(1){}
public:
	virtual ~_Ref_count_base() _NOEXCEPT {}

	unsigned int _Get_uses() const{	return (int)(_Uses);}
	void _Incref()	{++_Uses;}
	void _Incwref()	{++_Weaks;}
	void _Decref()
	{
		if (--_Uses == 0)
		{	
			_Destroy();
			_Decwref();
		}
	}

	void _Decwref()
	{
		if (--_Weaks == 0) 
		{
			_Delete_this();
		}
	}

	long _Use_count() const {return (_Get_uses());}

	bool _Expired() const
	{
		return (_Get_uses() == 0);
	}
};

template<class _Ty>
class _Ref_count: public _Ref_count_base
{
public:
	_Ref_count(_Ty *_Px): _Ref_count_base(), _Ptr(_Px){	}

private:
	virtual void _Destroy()
	{
		delete _Ptr;
	}

	virtual void _Delete_this()
	{
		delete this;
	}

	_Ty * _Ptr;
};

template<class _Ty>
class weak_ptr;
template<class _Ty>
class shared_ptr;
template<class _Ty>
class enable_shared_from_this;

template<class _Ty1,class _Ty2>
void _Do_enable(_Ty1 *, enable_shared_from_this<_Ty2> *,_Ref_count_base *);

template<class _Ty>
inline void _Enable_shared(_Ty *_Ptr, _Ref_count_base *_Refptr,	typename _Ty::_EStype * = 0)
{
	if (_Ptr)
		_Do_enable(_Ptr,
		(enable_shared_from_this<typename _Ty::_EStype>*)_Ptr, _Refptr);
}

inline void _Enable_shared(const volatile void *, const volatile void *)
{
}

template<class _Ty>
class _Ptr_base
{
public:
	typedef _Ptr_base<_Ty> _Myt;
	typedef _Ty _Elem;
	typedef _Elem element_type;

	_Ptr_base(): _Ptr(0), _Rep(0){}		

	long use_count() const _NOEXCEPT
	{
		return (_Rep ? _Rep->_Use_count() : 0);
	}

	void _Swap(_Ptr_base& _Right)
	{
		_STD swap(_Rep, _Right._Rep);
		_STD swap(_Ptr, _Right._Ptr);
	}

	_Ty *_Get() const
	{
		return (_Ptr);
	}

	bool _Expired() const
	{
		return (!_Rep || _Rep->_Expired());
	}

	void _Decref()
	{
		if (_Rep != 0)
			_Rep->_Decref();
	}

	void _Reset()
	{
		_Reset(0, 0);
	}

	template<class _Ty2>
	void _Reset(const _Ptr_base<_Ty2>& _Other)
	{
		//  这里强制转换 有崩溃的风险
		_Reset((_Elem *)(_Other._Ptr), _Other._Rep);
	}

	void _Reset(_Ty *_Other_ptr, _Ref_count_base *_Other_rep)
	{
		if (_Other_rep)
			_Other_rep->_Incref();
		_Reset0(_Other_ptr, _Other_rep);
	}

	void _Reset0(_Ty *_Other_ptr, _Ref_count_base *_Other_rep)
	{
		if (_Rep != 0)
			_Rep->_Decref();
		_Rep = _Other_rep;
		_Ptr = _Other_ptr;
	}

	void _Decwref()
	{
		if (_Rep != 0)
			_Rep->_Decwref();
	}

	void _Resetw()
	{
		_Resetw((_Elem *)0, 0);
	}

	template<class _Ty2>
	void _Resetw(const _Ptr_base<_Ty2>& _Other)
	{
		_Resetw(_Other._Ptr, _Other._Rep);
	}

	template<class _Ty2>
	void _Resetw(const _Ty2 *_Other_ptr, _Ref_count_base *_Other_rep)
	{
		_Resetw(const_cast<_Ty2*>(_Other_ptr), _Other_rep);
	}

	template<class _Ty2>
	void _Resetw(_Ty2 *_Other_ptr, _Ref_count_base *_Other_rep)
	{
		if (_Other_rep)
			_Other_rep->_Incwref();
		if (_Rep != 0)
			_Rep->_Decwref();
		_Rep = _Other_rep;
		_Ptr = _Other_ptr;
	}

private:
	_Ty *_Ptr;
	_Ref_count_base *_Rep;
	template<class _Ty0>
	friend class _Ptr_base;
};

template<class _Ty>
class shared_ptr: public _Ptr_base<_Ty>
{
public:
	typedef shared_ptr<_Ty> _Myt;
	typedef _Ptr_base<_Ty> _Mybase;

	shared_ptr()
	{
	}

	template<class _Ux>
	explicit shared_ptr(_Ux *_Px)
	{
		_Resetp(_Px);
	}
	template<class _Ty2>
	explicit shared_ptr(const weak_ptr<_Ty2>& _Other)
	{
		this->_Reset(_Other);
	}

	shared_ptr(const _Myt& _Other)
	{
		this->_Reset(_Other);
	}

	template<class _Ty2>
	shared_ptr(const shared_ptr<_Ty2>& _Other)
	{
		this->_Reset(_Other);
	}
		
	~shared_ptr() _NOEXCEPT
	{
		this->_Decref();
	}

	_Myt& operator=(const _Myt& _Right) _NOEXCEPT
	{
		shared_ptr(_Right).swap(*this);
		return (*this);
	}

	template<class _Ty2>
	_Myt& operator=(const shared_ptr<_Ty2>& _Right) _NOEXCEPT
	{
		shared_ptr(_Right).swap(*this);
		return (*this);
	}
		
	/*template <class Y>
	_Myt& operator = (Y* ptr)_NOEXCEPT
	{
		_Resetp(ptr);

		return (*this);
	}
	*/
	_Myt& operator = (uint64_t iptr)_NOEXCEPT
	{
		_Ty* ptr = static_cast<_Ty*>((void*)iptr);
		_Resetp(ptr);

		return (*this);
	}

	_Ty& operator *()_NOEXCEPT
	{
		return *get();
	}
	
	void reset() _NOEXCEPT
	{
		shared_ptr().swap(*this);
	}

	void swap(_Myt& _Other) _NOEXCEPT
	{
		this->_Swap(_Other);
	}

	_Ty *get() const _NOEXCEPT
	{
		return (this->_Get());
	}
	_Ty *operator->() const _NOEXCEPT
	{
		return (this->_Get());
	}
	bool isNull() const _NOEXCEPT
	{
		return get() == 0;
	}
	template <class Y>
	bool operator == (const shared_ptr<Y>& ptr) const _NOEXCEPT
	{
		return get() == ptr.get();
	}
	/*template <class Y>
	bool operator == (Y* ptr) const _NOEXCEPT
	{
		return get() == ptr;
	}

	bool operator == (uint64_t iptr) const _NOEXCEPT
	{
		return get() == (void*)iptr;
	}*/

	template <class Y>
	bool operator != (const shared_ptr<Y>& ptr) const _NOEXCEPT
	{
		return get() != ptr.get();
	}

	template <class Y>
	bool operator != (Y* ptr) const _NOEXCEPT
	{
		return get() != ptr;
	}

	bool operator != (uint64_t iptr) const _NOEXCEPT
	{
		return get() != (void*)iptr;
	}	
	template <class Y>
	bool operator < (const shared_ptr<Y>& ptr) const
	{
		return get() < ptr.get();
	}
private:
	template<class _Ux>
	void _Resetp(_Ux *_Px)
	{
		_TRY_BEGIN	
			_Resetp0(_Px, new _Ref_count<_Ux>(_Px));
		_CATCH_ALL
			delete _Px;
		_RERAISE;
		_CATCH_END
	}
public:
	template<class _Ux>
	void _Resetp0(_Ux *_Px, _Ref_count_base *_Rx)
	{	
		this->_Reset0(_Px, _Rx);
		_Enable_shared(_Px, _Rx);
	}
};
	
template<class _Ty>
class weak_ptr: public _Ptr_base<_Ty>
{
	typedef typename _Ptr_base<_Ty>::_Elem _Elem;

public:
	weak_ptr() _NOEXCEPT
	{	
	}

	template<class _Ty2>
	weak_ptr(const shared_ptr<_Ty2>& _Other) _NOEXCEPT
	{
		this->_Resetw(_Other);
	}

	weak_ptr(const weak_ptr& _Other) _NOEXCEPT
	{
		this->_Resetw(_Other);
	}

	~weak_ptr() _NOEXCEPT
	{
		this->_Decwref();
	}

	weak_ptr& operator=(const weak_ptr& _Right) _NOEXCEPT
	{
		this->_Resetw(_Right);
		return (*this);
	}

	template<class _Ty2>
	weak_ptr& operator=(const weak_ptr<_Ty2>& _Right) _NOEXCEPT
	{
		this->_Resetw(_Right.lock());
		return (*this);
	}

	template<class _Ty2>
	weak_ptr& operator=(const shared_ptr<_Ty2>& _Right) _NOEXCEPT
	{
		this->_Resetw(_Right);
		return (*this);
	}

	void reset() _NOEXCEPT
	{
		this->_Resetw();
	}

	bool expired() const _NOEXCEPT
	{
		return (this->_Expired());
	}

	shared_ptr<_Ty> lock() const _NOEXCEPT
	{
		return expired() ? shared_ptr<_Elem>() : shared_ptr<_Elem>(*this);
	}
};

template<class _Ty>
class enable_shared_from_this
{
public:
	typedef _Ty _EStype;

	shared_ptr<_Ty> shared_from_this()
	{	
		return (shared_ptr<_Ty>(_Wptr));
	}

	shared_ptr<const _Ty> shared_from_this() const
	{	
		return (shared_ptr<const _Ty>(_Wptr));
	}

protected:
	enable_shared_from_this() _NOEXCEPT
	{
	}

	enable_shared_from_this(const enable_shared_from_this&) _NOEXCEPT
	{
	}

	enable_shared_from_this&
		operator=(const enable_shared_from_this&) _NOEXCEPT
	{
		return (*this);
	}

	~enable_shared_from_this() _NOEXCEPT
	{
	}

private:
	template<class _Ty1,class _Ty2>
	friend void _Do_enable(	_Ty1 *,	enable_shared_from_this<_Ty2>*,	_Ref_count_base *);

	mutable weak_ptr<_Ty> _Wptr;
};

template<class _Ty1,class _Ty2>
inline void _Do_enable(	_Ty1 *_Ptr,	enable_shared_from_this<_Ty2> *_Es,	_Ref_count_base *_Refptr)
{
	_Es->_Wptr._Resetw(_Ptr, _Refptr);
}
	


template<class T,class Y>
shared_ptr<T> make_shared()
{
	Y* ptr = new Y();

	return shared_ptr<T>(ptr);
}

template<class T>
shared_ptr<T> make_shared()
{
	T* ptr = new T();

	return shared_ptr<T>(ptr);
}

template<class T,class Y,typename A>
#ifdef BUILDERVERNEWTHEN2012
shared_ptr<T> make_shared(A&& a)
#else
shared_ptr<T> make_shared(const A& a)
#endif
{
	Y* ptr = new Y(a);

	return shared_ptr<T>(ptr);
}

template<class T,typename A>
#ifdef BUILDERVERNEWTHEN2012
shared_ptr<T> make_shared(A&& a)
#else
shared_ptr<T> make_shared(const A& a)
#endif
{
	T* ptr = new T(a);

	return shared_ptr<T>(ptr);
}

template<class T,class Y,typename A1,typename A2>
#ifdef BUILDERVERNEWTHEN2012
shared_ptr<T> make_shared(A1&& a1,A2 && a2)
#else
shared_ptr<T> make_shared(const A1& a1,const A2& a2)
#endif
{
	Y* ptr = new Y(a1,a2);

	return shared_ptr<T>(ptr);
}

template<class T,typename A1,typename A2>
#ifdef BUILDERVERNEWTHEN2012
shared_ptr<T> make_shared(A1&& a1,A2 && a2)
#else
shared_ptr<T> make_shared(const A1& a1,const A2& a2)
#endif
{
	T* ptr = new T(a1,a2);

	return shared_ptr<T>(ptr);
}

template<class T,class Y,typename A1,typename A2,typename A3>
#ifdef BUILDERVERNEWTHEN2012
shared_ptr<T> make_shared(A1&& a1,A2 && a2,A3 && a3)
#else
shared_ptr<T> make_shared(const A1& a1,const A2& a2,const A3& a3)
#endif
{
	Y* ptr = new Y(a1,a2,a3);

	return shared_ptr<T>(ptr);
}

template<class T,typename A1,typename A2,typename A3>
#ifdef BUILDERVERNEWTHEN2012
shared_ptr<T> make_shared(A1&& a1,A2 && a2,A3 && a3)
#else
shared_ptr<T> make_shared(const A1& a1,const A2& a2,const A3& a3)
#endif
{
	T* ptr = new T(a1,a2,a3);

	return shared_ptr<T>(ptr);
}

template<class T,class Y,typename A1,typename A2,typename A3,typename A4>
#ifdef BUILDERVERNEWTHEN2012
shared_ptr<T> make_shared(A1&& a1,A2 && a2,A3 && a3,A4 && a4)
#else
shared_ptr<T> make_shared(const A1& a1,const A2& a2,const A3& a3,const A4& a4)
#endif
{
	Y* ptr = new Y(a1,a2,a3,a4);

	return shared_ptr<T>(ptr);
}

template<class T,typename A1,typename A2,typename A3,typename A4>
#ifdef BUILDERVERNEWTHEN2012
shared_ptr<T> make_shared(A1&& a1,A2 && a2,A3 && a3,A4 && a4)
#else
shared_ptr<T> make_shared(const A1& a1,const A2& a2,const A3& a3,const A4& a4)
#endif
{
	T* ptr = new T(a1,a2,a3,a4);

	return shared_ptr<T>(ptr);
}

template<class T,class Y,typename A1,typename A2,typename A3,typename A4,typename A5>
#ifdef BUILDERVERNEWTHEN2012
shared_ptr<T> make_shared(A1&& a1,A2 && a2,A3 && a3,A4 && a4,A5 && a5)
#else
shared_ptr<T> make_shared(const A1& a1,const A2& a2,const A3& a3,const A4& a4,const A5& a5)
#endif
{
	Y* ptr = new Y(a1,a2,a3,a4,a5);

	return shared_ptr<T>(ptr);
}

template<class T,typename A1,typename A2,typename A3,typename A4,typename A5>
#ifdef BUILDERVERNEWTHEN2012
shared_ptr<T> make_shared(A1&& a1,A2 && a2,A3 && a3,A4 && a4,A5 && a5)
#else
shared_ptr<T> make_shared(const A1& a1,const A2& a2,const A3& a3,const A4& a4,const A5& a5)
#endif
{
	T* ptr = new T(a1,a2,a3,a4,a5);

	return shared_ptr<T>(ptr);
}

template<class T,class Y,typename A1,typename A2,typename A3,typename A4,typename A5,typename A6>
#ifdef BUILDERVERNEWTHEN2012
shared_ptr<T> make_shared(A1&& a1,A2 && a2,A3 && a3,A4 && a4,A5 && a5,A6 && a6)
#else
shared_ptr<T> make_shared(const A1& a1,const A2& a2,const A3& a3,const A4& a4,const A5& a5,const A6& a6)
#endif
{
	Y* ptr = new Y(a1,a2,a3,a4,a5,a6);

	return shared_ptr<T>(ptr);
}

template<class T,typename A1,typename A2,typename A3,typename A4,typename A5,typename A6>
#ifdef BUILDERVERNEWTHEN2012
shared_ptr<T> make_shared(A1&& a1,A2 && a2,A3 && a3,A4 && a4,A5 && a5,A6 && a6)
#else
shared_ptr<T> make_shared(const A1& a1,const A2& a2,const A3& a3,const A4& a4,const A5& a5,const A6& a6)
#endif
{
	T* ptr = new T(a1,a2,a3,a4,a5,a6);

	return shared_ptr<T>(ptr);
}

template<class T,class Y,typename A1,typename A2,typename A3,typename A4,typename A5,typename A6,typename A7>
#ifdef BUILDERVERNEWTHEN2012
shared_ptr<T> make_shared(A1&& a1,A2 && a2,A3 && a3,A4 && a4,A5 && a5,A6 && a6,A7&& a7)
#else
shared_ptr<T> make_shared(const A1& a1,const A2& a2,const A3& a3,const A4& a4,const A5& a5,const A6& a6,const A7& a7)
#endif
{
	Y* ptr = new Y(a1,a2,a3,a4,a5,a6,a7);

	return shared_ptr<T>(ptr);
}

template<class T,typename A1,typename A2,typename A3,typename A4,typename A5,typename A6,typename A7>
#ifdef BUILDERVERNEWTHEN2012
shared_ptr<T> make_shared(A1&& a1,A2 && a2,A3 && a3,A4 && a4,A5 && a5,A6 && a6,A7&& a7)
#else
shared_ptr<T> make_shared(const A1& a1,const A2& a2,const A3& a3,const A4& a4,const A5& a5,const A6& a6,const A7& a7)
#endif
{
	T* ptr = new T(a1,a2,a3,a4,a5,a6,a7);

	return shared_ptr<T>(ptr);
}

template<class T,class Y,typename A1,typename A2,typename A3,typename A4,typename A5,typename A6,typename A7,typename A8>
#ifdef BUILDERVERNEWTHEN2012
shared_ptr<T> make_shared(A1&& a1,A2 && a2,A3 && a3,A4 && a4,A5 && a5,A6 && a6,A7&& a7,A8&& a8)
#else
shared_ptr<T> make_shared(const A1& a1,const A2& a2,const A3& a3,const A4& a4,const A5& a5,const A6& a6,const A7& a7,const A8& a8)
#endif
{
	Y* ptr = new Y(a1,a2,a3,a4,a5,a6,a7,a8);

	return shared_ptr<T>(ptr);
}

template<class T,typename A1,typename A2,typename A3,typename A4,typename A5,typename A6,typename A7,typename A8>
#ifdef BUILDERVERNEWTHEN2012
shared_ptr<T> make_shared(A1&& a1,A2 && a2,A3 && a3,A4 && a4,A5 && a5,A6 && a6,A7&& a7,A8&& a8)
#else
shared_ptr<T> make_shared(const A1& a1,const A2& a2,const A3& a3,const A4& a4,const A5& a5,const A6& a6,const A7& a7,const A8& a8)
#endif
{
	T* ptr = new T(a1,a2,a3,a4,a5,a6,a7,a8);

	return shared_ptr<T>(ptr);
}

template<class T,class Y,typename A1,typename A2,typename A3,typename A4,typename A5,typename A6,typename A7,typename A8,typename A9>
#ifdef BUILDERVERNEWTHEN2012
shared_ptr<T> make_shared(A1&& a1,A2 && a2,A3 && a3,A4 && a4,A5 && a5,A6 && a6,A7&& a7,A8&& a8,A9&& a9)
#else
shared_ptr<T> make_shared(const A1& a1,const A2& a2,const A3& a3,const A4& a4,const A5& a5,const A6& a6,const A7& a7,const A8& a8,const A9& a9)
#endif
{
	Y* ptr = new Y(a1,a2,a3,a4,a5,a6,a7,a8,a9);

	return shared_ptr<T>(ptr);
}

template<class T,typename A1,typename A2,typename A3,typename A4,typename A5,typename A6,typename A7,typename A8,typename A9>
#ifdef BUILDERVERNEWTHEN2012
shared_ptr<T> make_shared(A1&& a1,A2 && a2,A3 && a3,A4 && a4,A5 && a5,A6 && a6,A7&& a7,A8&& a8,A9&& a9)
#else
shared_ptr<T> make_shared(const A1& a1,const A2& a2,const A3& a3,const A4& a4,const A5& a5,const A6& a6,const A7& a7,const A8& a8,const A9& a9)
#endif
{
	T* ptr = new T(a1,a2,a3,a4,a5,a6,a7,a8,a9);

	return shared_ptr<T>(ptr);
}

class nullptr_t
{
public:
	nullptr_t() {}
	~nullptr_t() {}

	template<typename T>
	operator T*()
	{
		return (T*)NULL;
	}

	template<typename T>
	operator shared_ptr<T>()
	{
		return shared_ptr<T>();
	}
};

//typedef  nullptr_t nullptr;

template<typename T>
bool operator != (const nullptr_t& pointer, const shared_ptr<T>& ptr)
{
	return ptr != (shared_ptr<T>)pointer;
}

template<typename T>
bool operator != (const shared_ptr<T>& ptr, const nullptr_t& pointer)
{
	return ptr != (shared_ptr<T>)pointer;
}

template<typename T>
bool operator == (const nullptr_t& pointer, const shared_ptr<T>& ptr)
{
	return ptr == (shared_ptr<T>)pointer;
}

template<typename T>
bool operator == (const shared_ptr<T>& ptr, const nullptr_t& pointer)
{
	return ptr == (shared_ptr<T>)pointer;
}

template<typename T>
bool operator != (void* pointer, const shared_ptr<T>& ptr)
{
	return ptr.get() != pointer;
}

template<typename T>
bool operator != (int pointer, const shared_ptr<T>& ptr)
{
	return ptr.get() != (void*)pointer;
}

template<typename T>
bool operator != (long int pointer, const shared_ptr<T>& ptr)
{
	return ptr.get() != (void*)pointer;
}




#endif //SUPORTHAVESHAREDPTR


template<typename T>
bool operator == (const shared_ptr<T>& ptr, void* pointer)
{
	return ptr.get() == pointer;
}

template<typename T>
bool operator == (const shared_ptr<T>& ptr, int pointer)
{
	size_t ptrtmpval = pointer;
	return ptr.get() == (void*)ptrtmpval;
}

template<typename T>
bool operator == (const shared_ptr<T>& ptr, long int pointer)
{
	return ptr.get() == (void*)pointer;
}

template<typename T>
bool operator == (void* pointer,const shared_ptr<T>& ptr)
{
	return ptr.get() == pointer;
}

template<typename T>
bool operator == (int pointer,const shared_ptr<T>& ptr)
{
	return ptr.get() == (void*)pointer;
}

template<typename T>
bool operator == (long int pointer, const shared_ptr<T>& ptr)
{
	return ptr.get() == (void*)pointer;
}


}
}


#endif //__shared_ptr_H__
