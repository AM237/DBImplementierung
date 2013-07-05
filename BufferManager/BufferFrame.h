///////////////////////////////////////////////////////////////////////////////
// BufferFrame.h
//////////////////////////////////////////////////////////////////////////////


#ifndef BUFFERFRAME_H
#define BUFFERFRAME_H

#include <unordered_set>
#include <unordered_map>
#include <gtest/gtest.h>
#include <thread>


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
	BufferFrame();

	// Destructor. Does not take responsiblity for deleting data pointer.
	~BufferFrame(){ }

	// A method giving access to the buffered page
	void* getData();

	// Lock this frame in shared or exclusive mode. No checks are made, call
	// blocks if lock cannot be granted.
	void lockFrame(bool write);

	// Lock this frame in shared or exclusive mode. If returns true iff lock
	// can be granted in the given mode. If lock cannot be granted, second
	// bool is true iff this was because the calling thread already owns the
	// lock.
	bool tryLockFrame(bool write);

	// Unlock this frame.
	void unlockFrame();

	// Returns true iff the calling thread has a lock on this frame.
	bool isClient();

	// The id of the page held in the frame.
	uint64_t pageId;

	// Whether the page is dirty (true) or not (false)
	bool isDirty;

	// Whether the page is fixed in the frame and thus cannot be replaced.
	bool pageFixed;

private:

	// Threads that currenty refer to this frame
	std::unordered_set<std::thread::id> clients;
	
	// Handle concurrent access
	pthread_rwlock_t lock;

	// Pointer to the page, which is of known size. 
	FRIEND_TEST(BufferManagerTest, flushFrameToFile);
	void* data;
};


#endif  // BUFFERFRAME_H