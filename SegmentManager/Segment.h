
///////////////////////////////////////////////////////////////////////////////
// Segment.h
//////////////////////////////////////////////////////////////////////////////


#ifndef SEGMENT_H
#define SEGMENT_H

#include <gtest/gtest.h>
#include <stdint.h>

// An object representing an extent in a segment. Bound given as [start, end)
struct Extent
{
public: 
	Extent(uint64_t start, uint64_t end) : start(start), end(end) { }
	uint64_t start;
	uint64_t end;
};

// Specialized segment types
enum segTypes { segTypes_begin, RG_SGM = segTypes_begin, SP_SGM, segTypes_end };


// Abstract class, represents a segment in the DBMS
class Segment
{
	friend class SegmentInventory;
	friend class SegmentManager;

public:

	// Constructor, destructor. Allows for an option extent to be added
	// in the construction process.
	FRIEND_TEST(SegmentManagerTest, initializeNoFile);
	FRIEND_TEST(SegmentManagerTest, initializeWithFile);
	Segment(bool permanent, bool visible, uint64_t id, Extent* ex = nullptr);
	
	virtual ~Segment() { }

	// Returns whether this segment is public (true) or private (false)
	bool getVisibility() { return visible; }

	// Returns true iff the given page belongs to this segment
	bool inSegment(uint64_t pageId);
	
	// Returns the id of this segment
	uint64_t getId() { return id; }
	
	// Returns the size of this segment in frames
	//
	// FRIEND_TEST(SegmentManagerTest, initializeWithFile);	
	uint64_t getSize();

	// Returns the page id of the first page in the segment.
	uint64_t firstPage();

	// Returns the page id of the next page in the segment as of the given 
	// counter. Calling this method for the first time gives the first page 
	// of the segment, calling it twice gives the second page, and so on. 
	// If no more pages are available, the id of the last available page 
	// is returned. Example:
	//
	// int nextPage = 5;
	// nextPage(nextPage) -> gives id of 5th page of segment
	// nextPage(nextPage) -> gives id of 6th page of segment
	// nextPage(nextPage) -> gives id of 7th page of segment 
	// ...
	//
	// FRIEND_TEST(SegmentManagerTest, initializeWithFile);
	uint64_t nextPage(uint64_t& nextPageCounter);

	
protected:

	// Writes (copies) data from one array to another
	//
	// FRIEND_TEST(SegmentManagerTest, initializeWithFile);
	void writeToArray(uint64_t* from, void* to, int elems, int offset);

	// The extents in this segment
	std::vector<Extent> extents;

	// Id of this segment
	uint64_t id;

	// Defines whether this segment is public or private
	bool visible;
	
	// Defines whether this segment is permanent or not. Non permanent 
	// segments are dropped at the end of the SegmentManager life cycle.
	bool permanent;
};

#endif  // SEGMENT_H