
///////////////////////////////////////////////////////////////////////////////
// BufferHasher.h
//////////////////////////////////////////////////////////////////////////////


#ifndef BUFFERHASHER_H
#define BUFFERHASHER_H

#include "BufferFrame.h"
#include <mutex>
#include <vector>



// Lookup proxy component for external classes, keeps track of
// pages / buffer frames in the page / frame pool.
// Implements a hash table as a lookup mechanism
class BufferHasher
{

public:

	// Constructor, defines how many buckets in the table
	FRIEND_TEST(BufferManagerTest, constructor);
	BufferHasher(uint64_t tableSize);
	
	// Destructor, deletes dynamic storage used for frames
	~BufferHasher();
	
	// Given a page id returns the index of the bucket in the
	// hash table, in the range [0, tableSize)
	uint64_t hash(uint64_t pageId);
	
	// Add an association between the given pageId and a BufferFrame
	void insert(uint64_t pageId, BufferFrame* bf);
	
	// Removes the association between the given pageId and the
	// stored BufferFrame for this pageId, if such BufferFrame exists
	void remove(uint64_t pageId);
	
	// Returns references to all of the BufferFrames associated with
	// the given pageId
	std::vector<BufferFrame*>* lookup(uint64_t pageId);

	// Locks the bucket corresponding to the given non hashed value
	void lockBucket(uint64_t value);

	// Unlocks the bucket corresponding to the given non hashed value
	void unlockBucket(uint64_t value);

	// Iterate through frames (cyclic) managed by this hasher
	BufferFrame* nextFrame();

	std::mutex lock;
	

private:

	// Hash table, at all times contains pointers to all fixed frames
	FRIEND_TEST(BufferManagerTest, fixPageNoReplaceAndDestructor);
	std::vector< std::vector<BufferFrame*> > hashTable;
	
    // Handle concurrent access
	std::vector<std::mutex>* locks;

	// A vector containing all fixed as well as unfixed frames
	std::vector<BufferFrame*> framePool;

	// The number of frames managed by this hasher
	uint64_t size;

	// The current iterator index
	uint64_t currentFrameIndex;
};


#endif  // BUFFERHASHER_H
