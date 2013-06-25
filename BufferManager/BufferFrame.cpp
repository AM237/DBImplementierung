
///////////////////////////////////////////////////////////////////////////////
// BufferFrame.cpp
///////////////////////////////////////////////////////////////////////////////


#include "BufferFrame.h"
#include <pthread.h>

using namespace std;

//______________________________________________________________________________
BufferFrame::BufferFrame()
{
	pthread_rwlock_init(&syslock, NULL);
	pthread_rwlock_init(&userlock, NULL);
}

//______________________________________________________________________________
void* BufferFrame::getData()
{
	return data;
}

//______________________________________________________________________________
void BufferFrame::lockFrame(bool write, bool sys)
{
	if (sys)
	if (write) pthread_rwlock_wrlock(&syslock);
	else  	   pthread_rwlock_rdlock(&syslock);

	else
	if (write) pthread_rwlock_wrlock(&userlock);
	else  	   pthread_rwlock_rdlock(&userlock);
}

//______________________________________________________________________________
bool BufferFrame::tryLockFrame(bool write, bool sys)
{
	if (sys) 
	if (write) return pthread_rwlock_trywrlock(&syslock) == 0 ? true:false;
	else  	   return pthread_rwlock_tryrdlock(&syslock) == 0 ? true:false;

	else
	if (write) return pthread_rwlock_trywrlock(&userlock) == 0 ? true:false;
	else  	   return pthread_rwlock_tryrdlock(&userlock) == 0 ? true:false;
}

//______________________________________________________________________________
void BufferFrame::unlockFrame(bool sys)
{
	if(sys) pthread_rwlock_unlock(&syslock); 
	else 	pthread_rwlock_unlock(&userlock);
}

