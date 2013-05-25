
///////////////////////////////////////////////////////////////////////////////
// SegmentInventory.h
//////////////////////////////////////////////////////////////////////////////


#ifndef SEGMENTINVENTORY_H
#define SEGMENTINVENTORY_H

#include "Segment.h"
#include <vector>



// An object representing an entry in the segment inventory
struct InventoryEntry
{
public: 
	InventoryEntry(uint64_t segmentId, uint64_t extentStart, uint64_t extentEnd)
	: segmentId(segmentId), extentStart(extentStart), extentEnd(extentEnd) { }
	
	uint64_t segmentId;
	uint64_t extentStart;
	uint64_t extentEnd;
};


// The SegmentInventory is a collection of pages, always starting with page 0
// in the database file, which maps segments to the locations of their extents
class SegmentInventory : public Segment
{

public:

	// Constructor. Initializes this SegmentInventory by parsing the contents
	// of the first page of the database into the respective data structures 
	SegmentInventory(int fd, bool visible, uint64_t id);
	
	// Destructor
	~SegmentInventory() { }	

	// override
	void* nextPage();
	
private:

	// Starting from page #0 in the database, scan through all of this segment's
	// pages, and parse the data into the data structures of this class.
	// The extents on these pages is formatted as follows:
	// totalNumberOfEntries | segmentId | pageNoStart | pageNoEnd |
	// nextSegmentId | nextPageNoStart | nextPageNoEnd | ....
	void initializeFromFile();
	
	// Performs the inverse operation as above: encode data in the local
	// data structures into an agreed format, and write this data to file, 
	// (the first page in the sagement inventory is always page 0).
	void writeToFile();
	
	// Data structure designed to contain the parsed data from file
	std::vector<InventoryEntry> entries;
	
	// The SegmentInventory keeps track of the ids that have been assigned to
	// existing segments, and thus knows the next available id.
	uint64_t nextId;
	
	// The total number of entries (see initializeFromFile) in the inventory
	uint64_t numEntries;
	
	// Handler to the database file
	int fileDescriptor;
};


#endif  // SEGMENTINVENTORY_H
