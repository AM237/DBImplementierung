
///////////////////////////////////////////////////////////////////////////////
// Segment.h
//////////////////////////////////////////////////////////////////////////////


#ifndef SEGMENT_H
#define SEGMENT_H

#include <stdint.h>

namespace constants
{
	// Page size should be a multiple of the size of a page in virtual memory
	// const int pageSize = sysconf(_SC_PAGE_SIZE);
	const int pageSize = 4096;
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
	virtual uint64_t getSize()=0;

	// Returns a pointer to a constants::pageSize data block, representing
	// the next page in this segment's set of pages. Calling this method
	// for the first time gives the first page of the segment, calling it twice
	// gives the second page, and so on. Return value of NULL indicates that
	// no more pages are available.
	virtual void* nextPage()=0;

	
protected:

	// Defines whether this segment is public or private
	bool visible;
	
	// Id of this segment
	uint64_t id;
};


// -----------------------------------------------------------------------------


// The SegmentInventory is a collection of pages, always starting with page 0
// in the database file, which maps segments to the locations of their extents
class SegmentInventory : public Segment
{

public:

	// Constructor/destructor
	SegmentInventory(bool visible, uint64_t id) : Segment(visible, id) { }
	~SegmentInventory() { }	
	
	// override
	uint64_t getSize();

	// override
	void* nextPage();
	
private:

	// Starting from page #0 in the database, scan through #getSize pages,
	// and parse the data into the data structures of this class.
	void initializeFromFile();
	
	// Performs the inverse operation as above: encode data in the local
	// data structures into an agreed format, and write this data to file, 
	// (the first page in the sagement inventory is always page 0).
	void writeToFile();
};

#endif  // SEGMENT_H
