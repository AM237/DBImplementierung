
///////////////////////////////////////////////////////////////////////////////
// BufferManager.h
//////////////////////////////////////////////////////////////////////////////


#ifndef BUFFERMANAGER_H
#define BUFFERMANAGER_H

#include <stdint.h>
#include <unistd.h>
#include <vector>
#include <string>
#include <algorithm>
#include <fcntl.h>
#include <iostream>
#include <fstream>
#include <time.h>
#include <queue>

using namespace std;


// ***************************************************************************
// Constants
// ***************************************************************************

namespace
{
	// Page size should be a multiple of the size of a page in virtual memory
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

public:

	// The id of the page held in the frame.
	uint64_t pageId;
	
	// Whether the page is dirty (true) or not (false)
	bool isDirty;

	// A method giving access to the buffered page
	void* getData();

private:

	// The page?
	
};




// Class representing a buffer manager
class BufferManager
{

public:

	// Creates a new instance that manages #size frames and operates on the
	// file 'filename'
	BufferManager(const string& filename, uint64_t size);
	
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

   

};

#endif  // BUFFERMANAGER_H

