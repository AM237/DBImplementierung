
///////////////////////////////////////////////////////////////////////////////
// BufferFrame.h
//////////////////////////////////////////////////////////////////////////////


#ifndef BUFFERFRAME_H
#define BUFFERFRAME_H

#include <gtest/gtest.h>
#include <mutex>


// Class representing a buffer frame in the buffer manager
class BufferFrame
{
 	friend class BufferManager;
 	friend class TwoQueueReplacer;
 	// TODO: avoid list of classes here

public:

	// Constructor, initially points to no data. This is primary indicator
	// whether the rest of the info in the object is valid or not.
	FRIEND_TEST(BufferManagerTest, constructor);
	BufferFrame() { data = NULL; }

	// The id of the page held in the frame.
	uint64_t pageId;
	
	// Whether the page is dirty (true) or not (false)
	bool isDirty;
	
	// Whether the page is fixed in the frame and thus cannot be replaced.
	bool pageFixed;

	// A method giving access to the buffered page
	void* getData() { return data; }


private:

	// Pointer to the page, which is of known size
	FRIEND_TEST(BufferManagerTest, flushFrameToFile);
	void* data;
};


#endif  // BUFFERFRAME_H
