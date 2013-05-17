
///////////////////////////////////////////////////////////////////////////////
// BufferManager.h
//////////////////////////////////////////////////////////////////////////////


#ifndef BUFFERMANAGER_H
#define BUFFERMANAGER_H

#include <string>
#include <vector>
#include <stdint.h>
#include <list>
#include <gtest/gtest.h>


// ***************************************************************************
// Constants
// ***************************************************************************

namespace constants
{
	// Page size should be a multiple of the size of a page in virtual memory
	// const int pageSize = sysconf(_SC_PAGE_SIZE);
	const int pageSize = 4096;
}

// ***************************************************************************
// Structs, templates, types
// ***************************************************************************



// ***************************************************************************
// Core Classes
// ***************************************************************************



// Class representing a buffer frame in the buffer manager
class BufferFrame
{
	friend class BufferManager;

public:

	// Constructor, initially points to no data. This is primary indicator
	// whether the rest of the info in the object is valid or not.
	FRIEND_TEST(BufferManagerTest, constructorDestructor);
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



// 2Queues
class TwoQueues
{

public:

	// Constructor
	FRIEND_TEST(BufferManagerTest, constructorDestructor);
	TwoQueues(){}

    // A page is fixed first time, added to Fifo queue
    void pageFixedFirstTime(BufferFrame *frame);

    // After page is fixed again and moved to LRU queue if in Fifo queue
    void pageFixedAgain(BufferFrame *frame);
	
    // replace a Frame in the 2q
    BufferFrame* replaceFrame();

private:

	// The FIFO queue
	std::list<BufferFrame*> fifo;
	
	// The FIFO queue
	std::list<BufferFrame*>  lru;
};




// Lookup proxy component for external classes, keeps track of
// pages / buffer frames in the page / frame pool.
// Implements a hash table as a lookup mechanism
class BufferHasher
{

public:

	// Constructor, defines how many buckets in the table
	FRIEND_TEST(BufferManagerTest, constructorDestructor);
	BufferHasher(uint64_t tableSize) 
	{ 
		size = tableSize;
		for(unsigned int i = 0; i < size; i++)
		{
			std::vector<BufferFrame*> initial;
			hashTable.push_back(initial);
		}
	}

	// Given a page id returns the index of the bucket in the
	// hash table, in the range [0, tableSize)
	uint64_t hash(uint64_t pageId) { return pageId % size; }
	
	// Add an association between the given pageId and a BufferFrame
	void insert(uint64_t pageId, BufferFrame* bf)
	{
		hashTable[hash(pageId)].push_back(bf);
	}
	
	// Returns references to all of the BufferFrames associated with
	// the given pageId
	std::vector<BufferFrame*>* lookup(uint64_t pageId)
	{
		return &hashTable[hash(pageId)]; 
	}
	
	// Removes the association between the given pageId and the
	// stored BufferFrame for this pageId, if such BufferFrame exists
	void remove(uint64_t pageId)
	{
		std::vector<BufferFrame*>* frames = lookup(pageId);
		for (size_t i = 0; i < frames->size(); i++)
			if(frames->at(i)->pageId == pageId)
			{
				frames->erase(frames->begin()+i);
				break;
			}
	}


private:

	uint64_t size;

	// Hash table
	std::vector< std::vector<BufferFrame*> > hashTable;
};





// Manages page IO to/in main memory
class BufferManager
{

public:

	// Creates a new instance that manages #size frames and operates on the
	// file 'filename'
	FRIEND_TEST(BufferManagerTest, constructorDestructor);
	BufferManager(const std::string& filename, uint64_t size);

	// A method to retrieve frames given a page ID and indicating whether the
	// page will be held exclusively by this thread or not. The method can fail
	// if no free frame is available and no used frame can be freed.
	BufferFrame& fixPage(uint64_t pageId, bool exclusive);
	
	// Return a frame to the buffer manager indicating whether it is dirty or
	// not. If dirty, the page manager must write it back to disk. It does not
	// have to write it back immediately, but must not write it back before
	// unfixPage is called.
	void unfixPage(BufferFrame& frame, bool isDirty);
	
	// Destructor. Write all dirty frames to disk and free all resources.
	~BufferManager();
                  
private:
	
	// Reads page with pageID into frame, updates hash table. The page becomes
	// fixed in the frame, and is not dirty.
	FRIEND_TEST(BufferManagerTest, readPageIntoFrame);
    void readPageIntoFrame(uint64_t pageId, BufferFrame* frame );
    
    // Writes the page #frame back to disk. Assumes frame.pageId * pageSize
    // is a valid offset inside the file (i.e. is an offset that is followed
    // by at least one page)
    FRIEND_TEST(BufferManagerTest, flushFrameToFile);
    void flushFrameToFile(BufferFrame& frame);

	// The number of frames to be managed
	uint64_t numFrames;
	
	// Handler to file with pages on disk. The file is assumed to contain
	// a multiple of constants::pageSize bytes. The pages
	// are assumed to be numered 0 ... n.
	int fileDescriptor;
	
	// The pool of buffer frames, is instantiated and filled
	// on construction of the BufferManager object
	std::vector<BufferFrame*> framePool;

	// TwoQueues to manage replacements
	TwoQueues* twoQueues;
	
	// Hash proxy, supporting queries for pages given their id
	BufferHasher* hasher;
	
	
};

#endif  // BUFFERMANAGER_H

