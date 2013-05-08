
///////////////////////////////////////////////////////////////////////////////
// BufferManager.cpp
///////////////////////////////////////////////////////////////////////////////


#include "BufferManager.h"
#include <unistd.h>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <queue>

using namespace std;

//_____________________________________________________________________________
BufferManager::BufferManager(const string& filename, uint64_t size)
{
	// Initialize class fields
	numFrames = size;
	file = filename;
	
	// Initialize hasher. In terms of the size of the underlying hash table,
	// the worst case is given when each page is mapped to its own unique
	// bucket, so the max. number of required buckets is that of the 
	// manager's frame capacity
	hasher = new BufferHasher(numFrames);
	
	// Initialize buffer frame pool
	for (unsigned int i = 0; i < size; i++)
		framePool.push_back(new BufferFrame());
}

//_____________________________________________________________________________
BufferFrame& BufferManager::fixPage(uint64_t pageId, bool exclusive)
{
	// Case: page with pageId is buffered -> return page directly
	// TODO
	
	// Case: page with pageId not buffered and space in buffer
	// -> read from file into a free buffer frame.
	// TODO
	
	// Case: page with pageId not buffered and buffer full
	// -> use replacement strategy to replace an unfixed page in buffer
	// TODO
}


//_____________________________________________________________________________
void BufferManager::unfixPage(BufferFrame& frame, bool isDirty)
{
	// Set the page as a candidate for replacement, consider
	// whether to implement force or no force, and whether page is dirty or not
	// TODO

}

//_____________________________________________________________________________
BufferManager::~BufferManager()
{
	// Write all dirty frames to disk
	// TODO

	// Free all resources
	delete[] hasher;	
	for (size_t i = 0; i < framePool.size(); i++)
		delete[] framePool[i];
}
