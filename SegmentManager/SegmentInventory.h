
///////////////////////////////////////////////////////////////////////////////
// SegmentInventory.h
//////////////////////////////////////////////////////////////////////////////


#ifndef SEGMENTINVENTORY_H
#define SEGMENTINVENTORY_H

#include "../BufferManager/BufferManager.h"
#include "Segment.h"
#include <map>
#include <vector>



// Comparison predicate for map
struct classcomp {
  bool operator() (const uint64_t& lhs, const uint64_t& rhs) const
  {return lhs<rhs;}
};


// The SegmentInventory is a collection of pages, always starting with page 0
// in the database file, which maps segments to the locations of their extents
class SegmentInventory : public Segment
{

public:

	// Constructor. Initializes this SegmentInventory by parsing the contents
	// of the first page of the database into the respective data structures 
	SegmentInventory(BufferManager* bm, bool visible, uint64_t id);
	
	// Destructor
	~SegmentInventory() { }	

	// override
	uint64_t nextPage();
	
private:

	// Starting from page #0 in the database, scan through all of this segment's
	// pages, and parse the data into the data structures of this class.
	// Each page on an SI extent is formatted as follows:
	// totalNumberOfEntries | segmentId | pageNoStart | pageNoEnd |
	// nextSegmentId | nextPageNoStart | nextPageNoEnd | ....
	//
	// If there is no meaningful data on file (e.g. first int = 0), 
	// extents are created and recorded for the segment and 
	// free space inventories.
	void initializeFromFile();
	
	// Retrieves tuples of the form <segmentId, pageStartNo, pageEndNo>
	// on file and stores them in the object's data structures.
	// If SI spans more than one page, this method is called recursively
	// with the approriate buffer frame.
	// 
	// counter decreases every time a tuple is read (should be initialized to
	// the total number of tuples that make up the SI), so as to keep track
	// of how much information is relevant on each page that is read.
	void getSIExtents(std::vector<Extent>& accu, BufferFrame& frame, 
	                  uint64_t& counter);
	
	// Performs the inverse operation as above: look up data encoded in the
	// class data structures, and write it to file. More specifically,
	// look up the SI's extents to get the pages where the information is to be
	// written.
	void writeToFile();


	// Handler to the buffer manager	
	BufferManager* bm;
	
	// Data structure designed to contain the parsed data from file,
	// maps segmentId to a bucket containing all extents for this segment
	std::multimap<uint64_t, Extent, classcomp> entries;
	
	// The SegmentInventory keeps track of the ids that have been assigned to
	// existing segments, and thus knows the next available id.
	uint64_t nextId;
	
	// The total number of entries (see initializeFromFile) in the inventory
	uint64_t numEntries;
	
	// Max SI entries per page
	uint64_t maxEntries;
};


#endif  // SEGMENTINVENTORY_H
