
///////////////////////////////////////////////////////////////////////////////
// FreeSpaceInventory.h
//////////////////////////////////////////////////////////////////////////////


#ifndef FREESPACEINVENTORY_H
#define FREESPACEINVENTORY_H

#include "Segment.h"

// The FreeSpaceInventory is a special segment in the DBMS. It manages and
// stores the free extents in the database. As with the SegmentInventory, it
// also has the ability to materialize its state and initialize its state
// from the database.
class FreeSpaceInventory : public Segment
{

public:

	// Constructor/destructor
	FreeSpaceInventory(bool visible, uint64_t id);
	~FreeSpaceInventory() { }	

	// override
	//uint64_t nextPage();
	
private:

};

#endif  // FREESPACEINVENTORY_H
