
///////////////////////////////////////////////////////////////////////////////
// ScopedLock.h
//////////////////////////////////////////////////////////////////////////////


#ifndef SCOPEDLOCK_H
#define SCOPEDLOCK_H

#include "BufferFrame.h"
#include <mutex>

// Implements RAII on a given lock
struct ScopedMutexLock
{
public:
	ScopedMutexLock(std::mutex& lock) : m(lock) { }
	~ScopedMutexLock() {  m.unlock();  } 
	
private:
	std::mutex& m;
};


#endif  // SCOPEDLOCK_H