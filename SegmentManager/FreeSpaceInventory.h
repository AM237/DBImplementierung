
///////////////////////////////////////////////////////////////////////////////
// FreeSpaceInventory.h
//////////////////////////////////////////////////////////////////////////////


#ifndef FREESPACEINVENTORY_H
#define FREESPACEINVENTORY_H

#include "Segment.h"

// The SegmentInventory is a collection of pages, always starting with page 0
// in the database file, which maps segments to the locations of their extents
class FreeSpaceInventory : public Segment
{

public:

	// Constructor/destructor
	FreeSpaceInventory(int fd, bool visible, uint64_t id);
	~FreeSpaceInventory() { }	

	// override
	uint64_t nextPage();
	
private:

};

#endif  // FREESPACEINVENTORY_H
