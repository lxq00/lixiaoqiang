//
//  Copyright (c)1998-2012, Public Technology
//  All Rights Reserved.
//
//
//	Description:
//	$Id: AtomicCount.h 80 2013-04-11 07:05:56Z  $
//
#include "Base/AtomicCount.h"

namespace Public {
namespace Base {
	
/// \class AtomicCount
/// \brief 原子计数类

inline AtomicCount::AtomicCount():count(0){}

inline AtomicCount::AtomicCount( ValueType v ):count(v){}

inline AtomicCount::~AtomicCount(){}

inline AtomicCount::ValueType AtomicCount::value() const
{
	return count;
}

inline AtomicCount::operator AtomicCount::ValueType() const
{
	return count;
}

inline bool AtomicCount::operator !() const
{
	return count == 0;
}

#ifdef WIN32
inline AtomicCount::ValueType AtomicCount::operator ++()
{
	return InterlockedIncrement(&count);
}
inline AtomicCount::ValueType AtomicCount::operator ++(int)
{
	AtomicCount::ValueType result = InterlockedIncrement(&count);

	return --result;
}
inline AtomicCount::ValueType AtomicCount::operator --()
{
	return InterlockedDecrement(&count);
}
inline AtomicCount::ValueType AtomicCount::operator --(int)
{
	AtomicCount::ValueType result = InterlockedDecrement(&count);

	return ++result;
}
#else
inline AtomicCount::ValueType AtomicCount::operator ++()
{
	return __sync_add_and_fetch(&count,1);
}
inline AtomicCount::ValueType AtomicCount::operator ++(int)
{
	return __sync_fetch_and_add(&count,1);
}
inline AtomicCount::ValueType AtomicCount::operator --()
{
	return __sync_sub_and_fetch(&count,1);
}
inline AtomicCount::ValueType AtomicCount::operator --(int)
{
	return __sync_fetch_and_sub(&count,1);
}
#endif

} // namespace Base
} // namespace Public


