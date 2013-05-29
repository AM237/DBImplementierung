
///////////////////////////////////////////////////////////////////////////////
// Segment.h
//////////////////////////////////////////////////////////////////////////////


#ifndef SEGMENT_H
#define SEGMENT_H

#include <stdint.h>
#include <vector>


// An object representing an extent in a segment
struct Extent
{
public: 
	Extent(uint64_t start, uint64_t end) : start(start), end(start) { }
	
	//uint64_t segmentId;
	uint64_t start;
	uint64_t end;
};

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

	// Returns the page id of the next page in the segment. Calling this method
	// for the first time gives the first page of the segment, calling it twice
	// gives the second page, and so on. If no more pages are available, the
	// id of the last available page is returned.
	virtual uint64_t nextPage()=0;

	
protected:

	// The extents (page boundaries) in this segment
	std::vector<Extent> extents;

	// Defines whether this segment is public or private
	bool visible;
	
	// Id of this segment
	uint64_t id;
	
	// the size of this segment in pages
	uint64_t size;
	
	// the current page in the segment
	uint64_t currentPage;
};

#endif  // SEGMENT_H
