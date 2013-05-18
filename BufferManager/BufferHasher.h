
///////////////////////////////////////////////////////////////////////////////
// BufferHasher.h
//////////////////////////////////////////////////////////////////////////////


#ifndef BUFFERHASHER_H
#define BUFFERHASHER_H

#include "BufferFrame.h"

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


#endif  // BUFFERHASHER_H
