///////////////////////////////////////////////////////////////////////////////
// BufferFrame.cpp
///////////////////////////////////////////////////////////////////////////////


#include "BufferFrame.h"
#include <pthread.h>

using namespace std;

//______________________________________________________________________________
BufferFrame::BufferFrame()
{
	data = nullptr;
	pthread_rwlock_init(&lock, NULL);
}

//______________________________________________________________________________
void* BufferFrame::getData()
{
	return data;
}

//______________________________________________________________________________
void BufferFrame::lockFrame(bool write)
{
	if (write) pthread_rwlock_wrlock(&lock);
	else  	   pthread_rwlock_rdlock(&lock);
	clients.insert(this_thread::get_id());
}


//______________________________________________________________________________
bool BufferFrame::tryLockFrame(bool write)
{
	if (write) 
	{
		if (pthread_rwlock_trywrlock(&lock) == 0)
			{ clients.insert(this_thread::get_id()); return true; } 
		else return false;
	}

	else
	{
		if (pthread_rwlock_tryrdlock(&lock) == 0)
			{ clients.insert(this_thread::get_id()); return true; } 
		else return false;
	}
}
/*
//______________________________________________________________________________
pair<bool,bool> BufferFrame::tryLockFrame(bool write)
{
	if (clients.find(this_thread::get_id()) != clients.end())
		return pair<bool, bool>(false, true);
	else
		if (write) 
			{
				return pthread_rwlock_trywrlock(&lock) == 0 ?
						pair<bool, bool>(true, true) : pair<bool, bool>(false, false);
			}
		else 
			{
				return pthread_rwlock_tryrdlock(&lock) == 0 ?
						pair<bool, bool>(true, true) : pair<bool, bool>(false, false);
			}
	
	if (write) 
	{
		if (pthread_rwlock_trywrlock(&lock) == 0)
		{
			clients.insert(this_thread::get_id());
			return pair<bool, bool>(true, true);
		}
		else return clients.find(this_thread::get_id()) != clients.end() ? 
				pair<bool, bool>(false, true) : pair<bool, bool>(false, false);

	} 
	else
	{
		if (pthread_rwlock_tryrdlock(&lock) == 0)
		{
			clients.insert(this_thread::get_id());
			return pair<bool, bool>(true, true);
		}
		else return clients.find(this_thread::get_id()) != clients.end() ? 
				pair<bool, bool>(false, true) : pair<bool, bool>(false, false);
	} 
}*/

//______________________________________________________________________________
void BufferFrame::unlockFrame()
{
	clients.erase(this_thread::get_id());
	pthread_rwlock_unlock(&lock);
}

//______________________________________________________________________________
bool BufferFrame::isClient()
{
	return clients.find(this_thread::get_id()) != clients.end() ? true : false;
}