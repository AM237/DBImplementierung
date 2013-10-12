
///////////////////////////////////////////////////////////////////////////////
// FrameReplacer.h
//////////////////////////////////////////////////////////////////////////////


#ifndef FRAMEREPLACER_H
#define FRAMEREPLACER_H


#include "FrameReplacer.h"
#include <list>
#include <mutex>

// Abstract class, represents a frame replacement strategy for BufferManager
class FrameReplacer
{

public:

	// Constructor, destructor
	// Requires reference to hashing element to update the lookup
	// mechanism in the buffer manager, if necessary
	FrameReplacer(BufferHasher* bh) { hasher = bh; }
	virtual ~FrameReplacer() { }
	
	// This method is called if a page is requested, and it is not buffered,
	// and there is still an empty frame available, so no frame replacement
	// must take place.
	virtual void pageFixedFirstTime(BufferFrame* frame)=0;
	
	// This method is called if a page is requested, and the page is already
	// buffered in a buffer frame.
	virtual void pageFixedAgain(BufferFrame* frame)=0;
	
	// Returns a valid pointer to the frame that has been chosen for replacement
	// The frame is cleaned of old data, the data pointer is set to NULL.
	// Method fails (segfaults) if there are no unfixed frames available
	// and all frames contain valid data
	virtual BufferFrame* replaceFrame()=0;
	

protected:

	BufferHasher* hasher;
};


// -----------------------------------------------------------------------------



// Replacer mechanism implementing the 2Q Replacement strategy
class TwoQueueReplacer : public FrameReplacer
{

public:

	// Constructor, destructor
	FRIEND_TEST(BufferManagerTest, constructor);
	TwoQueueReplacer(BufferHasher* hasher) : FrameReplacer(hasher) { }
	~TwoQueueReplacer() { }
	
	// override
	void pageFixedFirstTime(BufferFrame* frame);
	
	// override
	void pageFixedAgain(BufferFrame* frame);
	
	// override
	BufferFrame* replaceFrame();
	
	
private:
	
	FRIEND_TEST(BufferManagerTest, fixPageNoReplaceAndDestructor);
	
	// The FIFO queue
	std::list<BufferFrame*> fifo;
	
	// The LRU queue
	std::list<BufferFrame*>  lru;

	// Concurrent access control
	std::mutex replacerLock;
};



#endif  // FRAMEREPLACER_H
