
///////////////////////////////////////////////////////////////////////////////
// Segment.h
//////////////////////////////////////////////////////////////////////////////


#ifndef SEGMENT_H
#define SEGMENT_H

#include <stdint.h>

namespace segconstants
{
	// Page size should be a multiple of the size of a page in virtual memory
	// const int pageSize = sysconf(_SC_PAGE_SIZE);
	const int pageSize = 4096;
	
	// Required for dynamic extent mapping, this is the exponential factor
	// by which the size of extents increases every time a segment is grown.
	const float extentIncrease = 1.2;
}




// Abstract class, represents a segment in the DBMS
class Segment
{

public:

	// Constructor, destructor
	Segment(bool visible, uint64_t id) 
	{ 
		this->visible = visible; 
		this->id = id;
	}
	
	virtual ~Segment() { }
	
	// Returns whether this segment is public (true) or private (false)
	bool getVisibility() { return visible; }
	
	// Returns the id of this segment
	uint64_t getId() { return id; }
	
	// Returns the size of this segment in frames	
	uint64_t getSize() { return size; }

	// Returns a pointer to a constants::pageSize data block, representing
	// the next page in this segment's set of pages. Calling this method
	// for the first time gives the first page of the segment, calling it twice
	// gives the second page, and so on. Return value of nullptr indicates that
	// no more pages are available.
	virtual void* nextPage()=0;

	
protected:

	// Defines whether this segment is public or private
	bool visible;
	
	// Id of this segment
	uint64_t id;
	
	// the size of this segment in pages
	uint64_t size;
};

#endif  // SEGMENT_H
