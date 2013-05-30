
///////////////////////////////////////////////////////////////////////////////
// SegmentInventory.h
//////////////////////////////////////////////////////////////////////////////


#ifndef SEGMENTINVENTORY_H
#define SEGMENTINVENTORY_H

#include "../BufferManager/BufferManager.h"
#include "RegularSegment.h"
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <vector>


// Comparison predicate for map
struct comp {
  bool operator() (const uint64_t& lhs, const uint64_t& rhs) const
  {return lhs<rhs;}
};


// A SegmentInventory is a special segment in the DBMS. Internally, it manages
// the storage of segments and has the ability to materialize its state onto
// a segment in the database, as well as initialize its state from that segment.
class SegmentInventory : public Segment
{

public:

	// Constructor. Initializes this SegmentInventory by materializing the data
	// found initially on page 0 of the database into Segments in main memory,
	// with their respectively defined extents
	SegmentInventory(BufferManager* bm, bool visible, uint64_t id);
	
	// Destructor
	~SegmentInventory();
	
private:

	// Starting from page #0 in the database, scan through all of this segment's
	// pages, and materialize the data into Segment and Extent objects.
	// Each page on an SI extent is formatted as follows:
	// totalNumberOfEntries | segmentId | pageNoStart | pageNoEnd |
	// nextSegmentId | nextPageNoStart | nextPageNoEnd | ....
	//
	// If there is no meaningful data on file (e.g. first int = 0), 
	// an extent is created and recorded for the segment inventory.
	void initializeFromFile();
	
	// Accumulates tuples of the form <segmentId, pageStartNo, pageEndNo>
	// on file and stores them in into a multimap
	// If SI spans more than one page, this method is called recursively
	// with the approriate buffer frame.
	// 
	// counter decreases every time a tuple is read (should be initialized to
	// the total number of tuples that make up the SI), so as to keep track
	// of how much information is relevant on each page that is read.
	void parseSIExtents(std::multimap<uint64_t, Extent, comp>& mapping, 
	                    BufferFrame& frame, uint64_t& counter);
	
	// Performs the inverse operation as above: look up data encoded in the
	// class data structures, and write it to file. More specifically,
	// look up the SI's extents to get the pages where the information is to be
	// written.
	void writeToFile();

	// Handler to the buffer manager	
	BufferManager* bm;
	
	// Data structure mapping a segment id to a segment
	std::map<uint64_t, Segment*> segments;
	
	// The SegmentInventory keeps track of the ids that have been assigned to
	// existing segments, and thus knows the next available id.
	uint64_t nextId;
	
	// The total number of entries (see initializeFromFile) in the inventory
	uint64_t numEntries;
	
	// Max SI entries per page
	uint64_t maxEntries;
};


#endif  // SEGMENTINVENTORY_H
