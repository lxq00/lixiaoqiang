// Copyright (c) 2007, Google Inc.
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
// 
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// 
// ---
//
// A simple TEMPLATE_Mutex wrapper, supporting locks and read-write locks.
// You should assume the locks are *not* re-entrant.
//
// To use: you should define the following macros in your configure.ac:
//   ACX_PTHREAD
//   AC_RWLOCK
// The latter is defined in ../autoconf.
//
// This class is meant to be internal-only and should be wrapped by an
// internal namespace.  Before you use this module, please give the
// name of your internal namespace for this module.  Or, if you want
// to expose it, you'll want to move it to the Google namespace.  We
// cannot put this class in global namespace because there can be some
// problems when we have multiple versions of TEMPLATE_Mutex in each shared object.
//
// NOTE: by default, we have #ifdef'ed out the TryLock() method.
//       This is for two reasons:
// 1) TryLock() under Windows is a bit annoying (it requires a
//    #define to be defined very early).
// 2) TryLock() is broken for NO_THREADS mode, at least in NDEBUG
//    mode.
// If you need TryLock(), and either these two caveats are not a
// problem for you, or you're willing to work around them, then
// feel free to #define Mutex_TRYLOCK, or to remove the #ifdefs
// in the code below.
//
// CYGWIN NOTE: Cygwin support for rwlock seems to be buggy:
//    http://www.cygwin.com/ml/cygwin/2008-12/msg00017.html
// Because of that, we might as well use windows locks for
// cygwin.  They seem to be more reliable than the cygwin pthreads layer.
//
// TRICKY IMPLEMENTATION NOTE:
// This class is designed to be safe to use during
// dynamic-initialization -- that is, by global constructors that are
// run before main() starts.  The issue in this case is that
// dynamic-initialization happens in an unpredictable order, and it
// could be that someone else's dynamic initializer could call a
// function that tries to acquire this TEMPLATE_Mutex -- but that all happens
// before this TEMPLATE_Mutex's constructor has run.  (This can happen even if
// the TEMPLATE_Mutex and the function that uses the TEMPLATE_Mutex are in the same .cc
// file.)  Basically, because TEMPLATE_Mutex does non-trivial work in its
// constructor, it's not, in the naive implementation, safe to use
// before dynamic initialization has run on it.
//
// The solution used here is to pair the actual TEMPLATE_Mutex primitive with a
// bool that is set to true when the TEMPLATE_Mutex is dynamically initialized.
// (Before that it's false.)  Then we modify all TEMPLATE_Mutex routines to
// look at the bool, and not try to lock/unlock until the bool makes
// it to true (which happens after the TEMPLATE_Mutex constructor has run.)
//
// This works because before main() starts -- particularly, during
// dynamic initialization -- there are no threads, so a) it's ok that
// the TEMPLATE_Mutex operations are a no-op, since we don't need locking then
// anyway; and b) we can be quite confident our bool won't change
// state between a call to Lock() and a call to Unlock() (that would
// require a global constructor in one translation unit to call Lock()
// and another global constructor in another translation unit to call
// Unlock() later, which is pretty perverse).
//
// That said, it's tricky, and can conceivably fail; it's safest to
// avoid trying to acquire a TEMPLATE_Mutex in a global constructor, if you
// can.  One way it can fail is that a really smart compiler might
// initialize the bool to true at static-initialization time (too
// early) rather than at dynamic-initialization time.  To discourage
// that, we set is_safe_ to true in code (not the constructor
// colon-initializer) and set it to true via a function that always
// evaluates to true, but that the compiler can't know always
// evaluates to true.  This should be good enough.
//
// A related issue is code that could try to access the TEMPLATE_Mutex
// after it's been destroyed in the global destructors (because
// the TEMPLATE_Mutex global destructor runs before some other global
// destructor, that tries to acquire the TEMPLATE_Mutex).  The way we
// deal with this is by taking a constructor arg that global
// TEMPLATE_Mutexes should pass in, that causes the destructor to do no
// work.  We still depend on the compiler not doing anything
// weird to a TEMPLATE_Mutex's memory after it is destroyed, but for a
// static global variable, that's pretty safe.

#ifndef GOOGLE_TEMPLATE_Mutex_H_
#define GOOGLE_TEMPLATE_Mutex_H_

#include <ctemplate/config.h>
#if defined(NO_THREADS)
  typedef int TEMPLATE_MutexType;      // to keep a lock-count
#elif defined(_WIN32) || defined(__CYGWIN32__) || defined(__CYGWIN64__)
# ifndef WIN32_LEAN_AND_MEAN
#   define WIN32_LEAN_AND_MEAN  // We only need minimal includes
# endif
# ifndef NOMINMAX
#   define NOMINMAX             // Don't want windows to override min()/max()
# endif
# ifdef Mutex_TRYLOCK
  // We need Windows NT or later for TryEnterCriticalSection().  If you
  // don't need that functionality, you can remove these _WIN32_WINNT
  // lines, and change TryLock() to assert(0) or something.
#   ifndef _WIN32_WINNT
#     define _WIN32_WINNT 0x0400
#   endif
# endif
# include <windows.h>
  typedef CRITICAL_SECTION TEMPLATE_MutexType;
#elif defined(HAVE_PTHREAD) && defined(HAVE_RWLOCK)
  // Needed for pthread_rwlock_*.  If it causes problems, you could take it
  // out, but then you'd have to unset HAVE_RWLOCK (at least on linux -- it
  // *does* cause problems for FreeBSD, or MacOSX, but isn't needed
  // for locking there.)
# ifdef __linux__
#   if _XOPEN_SOURCE < 500      // including not being defined at all
#     undef _XOPEN_SOURCE
#     define _XOPEN_SOURCE 500  // may be needed to get the rwlock calls
#   endif
# endif
#if defined(HAVE_PTHREAD) && !defined(NO_THREADS)
# include <pthread.h>
#endif
  typedef pthread_rwlock_t TEMPLATE_MutexType;
#elif defined(HAVE_PTHREAD)
#if defined(HAVE_PTHREAD) && !defined(NO_THREADS)
# include <pthread.h>
#endif
  typedef pthread_TEMPLATE_Mutex_t TEMPLATE_MutexType;
#else
# error Need to implement TEMPLATE_Mutex.h for your architecture, or #define NO_THREADS
#endif

#include <assert.h>
#include <stdlib.h>      // for abort()

_START_GOOGLE_NAMESPACE_

namespace base {
// This is used for the single-arg constructor
enum LinkerInitialized { LINKER_INITIALIZED };
}

class TEMPLATE_Mutex {
 public:
  // Create a TEMPLATE_Mutex that is not held by anybody.  This constructor is
  // typically used for TEMPLATE_Mutexes allocated on the heap or the stack.
  inline TEMPLATE_Mutex();
  // This constructor should be used for global, static TEMPLATE_Mutex objects.
  // It inhibits work being done by the destructor, which makes it
  // safer for code that tries to acqiure this TEMPLATE_Mutex in their global
  // destructor.
  inline TEMPLATE_Mutex(base::LinkerInitialized);

  // Destructor
  inline ~TEMPLATE_Mutex();

  inline void Lock();    // Block if needed until free then acquire exclusively
  inline void Unlock();  // Release a lock acquired via Lock()
#ifdef Mutex_TRYLOCK
  inline bool TryLock(); // If free, Lock() and return true, else return false
#endif
  // Note that on systems that don't support read-write locks, these may
  // be implemented as synonyms to Lock() and Unlock().  So you can use
  // these for efficiency, but don't use them anyplace where being able
  // to do shared reads is necessary to avoid deadlock.
  inline void ReaderLock();   // Block until free or shared then acquire a share
  inline void ReaderUnlock(); // Release a read share of this TEMPLATE_Mutex
  inline void WriterLock() { Lock(); }     // Acquire an exclusive lock
  inline void WriterUnlock() { Unlock(); } // Release a lock from WriterLock()

 private:
  TEMPLATE_MutexType TEMPLATE_Mutex_;
  // We want to make sure that the compiler sets is_safe_ to true only
  // when we tell it to, and never makes assumptions is_safe_ is
  // always true.  volatile is the most reliable way to do that.
  volatile bool is_safe_;
  // This indicates which constructor was called.
  bool destroy_;

  inline void SetIsSafe() { is_safe_ = true; }

  // Catch the error of writing TEMPLATE_Mutex when intending TEMPLATE_MutexLock.
  TEMPLATE_Mutex(TEMPLATE_Mutex* /*ignored*/) {}
  // Disallow "evil" constructors
  TEMPLATE_Mutex(const TEMPLATE_Mutex&);
  void operator=(const TEMPLATE_Mutex&);
};

// We will also define GoogleOnceType, GOOGLE_ONCE_INIT, and
// GoogleOnceInit, which are portable versions of pthread_once_t,
// PTHREAD_ONCE_INIT, and pthread_once.

// Now the implementation of TEMPLATE_Mutex for various systems
#if defined(NO_THREADS)

// When we don't have threads, we can be either reading or writing,
// but not both.  We can have lots of readers at once (in no-threads
// mode, that's most likely to happen in recursive function calls),
// but only one writer.  We represent this by having TEMPLATE_Mutex_ be -1 when
// writing and a number > 0 when reading (and 0 when no lock is held).
//
// In debug mode, we assert these invariants, while in non-debug mode
// we do nothing, for efficiency.  That's why everything is in an
// assert.

TEMPLATE_Mutex::TEMPLATE_Mutex() : TEMPLATE_Mutex_(0) { }
TEMPLATE_Mutex::TEMPLATE_Mutex(base::LinkerInitialized) : TEMPLATE_Mutex_(0) { }
TEMPLATE_Mutex::~TEMPLATE_Mutex()            { assert(TEMPLATE_Mutex_ == 0); }
void TEMPLATE_Mutex::Lock()         { assert(--TEMPLATE_Mutex_ == -1); }
void TEMPLATE_Mutex::Unlock()       { assert(TEMPLATE_Mutex_++ == -1); }
#ifdef Mutex_TRYLOCK
bool TEMPLATE_Mutex::TryLock()      { if (TEMPLATE_Mutex_) return false; Lock(); return true; }
#endif
void TEMPLATE_Mutex::ReaderLock()   { assert(++TEMPLATE_Mutex_ > 0); }
void TEMPLATE_Mutex::ReaderUnlock() { assert(TEMPLATE_Mutex_-- > 0); }

typedef int GoogleOnceType;
const GoogleOnceType GOOGLE_ONCE_INIT = 0;
inline int GoogleOnceInit(GoogleOnceType* once_control,
                          void (*init_routine)(void)) {
  if ((*once_control)++ == 0)
    (*init_routine)();
  return 0;
}

#elif defined(_WIN32) || defined(__CYGWIN32__) || defined(__CYGWIN64__)

TEMPLATE_Mutex::TEMPLATE_Mutex() : destroy_(true) {
  InitializeCriticalSection(&TEMPLATE_Mutex_);
  SetIsSafe();
}
TEMPLATE_Mutex::TEMPLATE_Mutex(base::LinkerInitialized) : destroy_(false) {
  InitializeCriticalSection(&TEMPLATE_Mutex_);
  SetIsSafe();
}
TEMPLATE_Mutex::~TEMPLATE_Mutex()            { if (destroy_) DeleteCriticalSection(&TEMPLATE_Mutex_); }
void TEMPLATE_Mutex::Lock()         { if (is_safe_) EnterCriticalSection(&TEMPLATE_Mutex_); }
void TEMPLATE_Mutex::Unlock()       { if (is_safe_) LeaveCriticalSection(&TEMPLATE_Mutex_); }
#ifdef Mutex_TRYLOCK
bool TEMPLATE_Mutex::TryLock()      { return is_safe_ ?
                                 TryEnterCriticalSection(&TEMPLATE_Mutex_) != 0 : true; }
#endif
void TEMPLATE_Mutex::ReaderLock()   { Lock(); }      // we don't have read-write locks
void TEMPLATE_Mutex::ReaderUnlock() { Unlock(); }

// We do a simple spinlock for pthread_once_t.  See
//    http://www.ddj.com/cpp/199203083?pgno=3
#ifdef INTERLOCKED_EXCHANGE_NONVOLATILE
typedef LONG GoogleOnceType;
#else
typedef volatile LONG GoogleOnceType;
#endif
const GoogleOnceType GOOGLE_ONCE_INIT = 0;
inline int GoogleOnceInit(GoogleOnceType* once_control,
                          void (*init_routine)(void)) {
  while (1) {
    LONG prev = InterlockedCompareExchange(once_control, 1, 0);
    if (prev == 2) {            // We've successfully initted in the past.
      return 0;
    } else if (prev == 0) {     // No init yet, but we have the lock.
      (*init_routine)();
      InterlockedExchange(once_control, 2);
      return 0;
    } else {                    // Someone else is holding the lock, so wait.
      assert(1 == prev);
      Sleep(1);                 // sleep for 1ms
    }
  }
  return 1;                     // unreachable
}

#elif defined(HAVE_PTHREAD) && defined(HAVE_RWLOCK)

#define SAFE_PTHREAD(fncall)  do {   /* run fncall if is_safe_ is true */  \
  if (is_safe_ && fncall(&TEMPLATE_Mutex_) != 0) abort();                           \
} while (0)

TEMPLATE_Mutex::TEMPLATE_Mutex() : destroy_(true) {
  SetIsSafe();
  if (is_safe_ && pthread_rwlock_init(&TEMPLATE_Mutex_, NULL) != 0) abort();
}
TEMPLATE_Mutex::TEMPLATE_Mutex(base::LinkerInitialized) : destroy_(false) {
  SetIsSafe();
  if (is_safe_ && pthread_rwlock_init(&TEMPLATE_Mutex_, NULL) != 0) abort();
}
TEMPLATE_Mutex::~TEMPLATE_Mutex()       { if (destroy_) SAFE_PTHREAD(pthread_rwlock_destroy); }
void TEMPLATE_Mutex::Lock()    { SAFE_PTHREAD(pthread_rwlock_wrlock); }
void TEMPLATE_Mutex::Unlock()  { SAFE_PTHREAD(pthread_rwlock_unlock); }
#ifdef Mutex_TRYLOCK
bool TEMPLATE_Mutex::TryLock()      { return is_safe_ ?
                               pthread_rwlock_trywrlock(&TEMPLATE_Mutex_) == 0 : true; }
#endif
void TEMPLATE_Mutex::ReaderLock()   { SAFE_PTHREAD(pthread_rwlock_rdlock); }
void TEMPLATE_Mutex::ReaderUnlock() { SAFE_PTHREAD(pthread_rwlock_unlock); }
#undef SAFE_PTHREAD

typedef pthread_once_t GoogleOnceType;
const GoogleOnceType GOOGLE_ONCE_INIT = PTHREAD_ONCE_INIT;
inline int GoogleOnceInit(GoogleOnceType* once_control,
                          void (*init_routine)(void)) {
  return pthread_once(once_control, init_routine);
}

#elif defined(HAVE_PTHREAD)

#define SAFE_PTHREAD(fncall)  do {   /* run fncall if is_safe_ is true */  \
  if (is_safe_ && fncall(&TEMPLATE_Mutex_) != 0) abort();                           \
} while (0)

TEMPLATE_Mutex::TEMPLATE_Mutex() : destroy_(true) {
  SetIsSafe();
  if (is_safe_ && pthread_TEMPLATE_Mutex_init(&TEMPLATE_Mutex_, NULL) != 0) abort();
}
TEMPLATE_Mutex::TEMPLATE_Mutex(base::LinkerInitialized) : destroy_(false) {
  SetIsSafe();
  if (is_safe_ && pthread_TEMPLATE_Mutex_init(&TEMPLATE_Mutex_, NULL) != 0) abort();
}
TEMPLATE_Mutex::~TEMPLATE_Mutex()       { if (destroy_) SAFE_PTHREAD(pthread_TEMPLATE_Mutex_destroy); }
void TEMPLATE_Mutex::Lock()    { SAFE_PTHREAD(pthread_TEMPLATE_Mutex_lock); }
void TEMPLATE_Mutex::Unlock()  { SAFE_PTHREAD(pthread_TEMPLATE_Mutex_unlock); }
#ifdef Mutex_TRYLOCK
bool TEMPLATE_Mutex::TryLock() { return is_safe_ ?
                               pthread_TEMPLATE_Mutex_trylock(&TEMPLATE_Mutex_) == 0 : true; }
#endif
void TEMPLATE_Mutex::ReaderLock()   { Lock(); }
void TEMPLATE_Mutex::ReaderUnlock() { Unlock(); }
#undef SAFE_PTHREAD

typedef pthread_once_t GoogleOnceType;
const GoogleOnceType GOOGLE_ONCE_INIT = PTHREAD_ONCE_INIT;
inline int GoogleOnceInit(GoogleOnceType* once_control,
                          void (*init_routine)(void)) {
  return pthread_once(once_control, init_routine);
}

#endif

// --------------------------------------------------------------------------
// Some helper classes

// TEMPLATE_MutexLock(mu) acquires mu when constructed and releases it when destroyed.
class TEMPLATE_MutexLock {
 public:
  explicit TEMPLATE_MutexLock(TEMPLATE_Mutex *mu) : mu_(mu) { mu_->Lock(); }
  ~TEMPLATE_MutexLock() { mu_->Unlock(); }
 private:
  TEMPLATE_Mutex * const mu_;
  // Disallow "evil" constructors
  TEMPLATE_MutexLock(const TEMPLATE_MutexLock&);
  void operator=(const TEMPLATE_MutexLock&);
};

// ReaderTEMPLATE_MutexLock and WriterTEMPLATE_MutexLock do the same, for rwlocks
class ReaderTEMPLATE_MutexLock {
 public:
  explicit ReaderTEMPLATE_MutexLock(TEMPLATE_Mutex *mu) : mu_(mu) { mu_->ReaderLock(); }
  ~ReaderTEMPLATE_MutexLock() { mu_->ReaderUnlock(); }
 private:
  TEMPLATE_Mutex * const mu_;
  // Disallow "evil" constructors
  ReaderTEMPLATE_MutexLock(const ReaderTEMPLATE_MutexLock&);
  void operator=(const ReaderTEMPLATE_MutexLock&);
};

class WriterTEMPLATE_MutexLock {
 public:
  explicit WriterTEMPLATE_MutexLock(TEMPLATE_Mutex *mu) : mu_(mu) { mu_->WriterLock(); }
  ~WriterTEMPLATE_MutexLock() { mu_->WriterUnlock(); }
 private:
  TEMPLATE_Mutex * const mu_;
  // Disallow "evil" constructors
  WriterTEMPLATE_MutexLock(const WriterTEMPLATE_MutexLock&);
  void operator=(const WriterTEMPLATE_MutexLock&);
};

// Catch bug where variable name is omitted, e.g. TEMPLATE_MutexLock (&mu);
#define TEMPLATE_MutexLock(x) COMPILE_ASSERT(0, TEMPLATE_Mutex_lock_decl_missing_var_name)
#define ReaderTEMPLATE_MutexLock(x) COMPILE_ASSERT(0, rTEMPLATE_Mutex_lock_decl_missing_var_name)
#define WriterTEMPLATE_MutexLock(x) COMPILE_ASSERT(0, wTEMPLATE_Mutex_lock_decl_missing_var_name)

_END_GOOGLE_NAMESPACE_

#endif  /* #define GOOGLE_TEMPLATE_Mutex_H__ */
