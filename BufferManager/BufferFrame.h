
///////////////////////////////////////////////////////////////////////////////
// BufferFrame.h
//////////////////////////////////////////////////////////////////////////////


#ifndef BUFFERFRAME_H
#define BUFFERFRAME_H

#include <pthread.h>
#include <gtest/gtest.h>


// ***************************************************************************
// Classes
// ***************************************************************************


// Class representing a buffer frame in the buffer manager
class BufferFrame
{
 	friend class BufferManager;
 	friend class TwoQueueReplacer;

public:

	// Constructor, initially points to no data. This is primary indicator
	// whether the rest of the info in the object is valid or not.
	FRIEND_TEST(BufferManagerTest, constructor);
	BufferFrame() 
	{ 
		data = nullptr; 
		lock = PTHREAD_RWLOCK_INITIALIZER;
	}

	// Destructor. Does not take responsiblity for deleting data pointer.
	~BufferFrame(){ }

	// A method giving access to the buffered page
	void* getData() { return data; }

	// Lock this frame in shared or exclusive mode.
	void lockFrame(bool write) 
	{ 
		if (write) pthread_rwlock_wrlock(&lock);
		else  	   pthread_rwlock_rdlock(&lock);
	}

	// Lock this frame in shared or exclusive mode.
	bool tryLockFrame(bool write) 
	{ 
		if (write) return pthread_rwlock_trywrlock(&lock) == 0 ? true : false;
		else  	   return pthread_rwlock_tryrdlock(&lock) == 0 ? true : false;
	}

	// Unlock this frame.
	void unlockFrame() 
	{  
		pthread_rwlock_unlock(&lock); 
	}

	// The id of the page held in the frame.
	uint64_t pageId;
	
	// Whether the page is dirty (true) or not (false)
	bool isDirty;
	
	// Whether the page is fixed in the frame and thus cannot be replaced.
	bool pageFixed;


private:

	// Handle concurrent access
	pthread_rwlock_t lock;

	// Pointer to the page, which is of known size. 
	FRIEND_TEST(BufferManagerTest, flushFrameToFile);
	void* data;
};


#endif  // BUFFERFRAME_H
