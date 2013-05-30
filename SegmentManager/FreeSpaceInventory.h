
///////////////////////////////////////////////////////////////////////////////
// FreeSpaceInventory.h
//////////////////////////////////////////////////////////////////////////////


#ifndef FREESPACEINVENTORY_H
#define FREESPACEINVENTORY_H

#include "../BufferManager/BufferManager.h"
#include "Segment.h"
#include <map>

// The FreeSpaceInventory is a special segment in the DBMS. It manages and
// stores the free extents in the database. As with the SegmentInventory, it
// also has the ability to materialize its state and initialize its state
// from the database.
class FreeSpaceInventory : public Segment
{

public:

	// Constructor/destructor. Created by the SI when initializing from 
	FreeSpaceInventory(BufferManager* bm, bool visible, uint64_t id);
	~FreeSpaceInventory() { }	

	
private:

	// Initializes the free space mapping from the contents given on file
	void initializeFromFile();
	
	// Takes the extents of this segment, and writes the entries found in
	// freeSpace in the following format: numEntries | start1 | end1 | start2 |
	// end2 | ... etc.
	void writeToFile();

	// Mapping of start of extent to end of extent, marking free space
	// Free space is given on interval [start, end)
	std::map<uint64_t, uint64_t> freeSpace;
	
	// Handler to the buffer manager
	BufferManager* bm;
	
	// Number of [start, end) entries managed by the FreeSpaceInventory
	uint64_t numEntries;
	
	// Max FSI entries per page
	uint64_t maxEntries;
};

#endif  // FREESPACEINVENTORY_H
