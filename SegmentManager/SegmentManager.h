
///////////////////////////////////////////////////////////////////////////////
// Segment.h
//////////////////////////////////////////////////////////////////////////////


#ifndef SEGMENTMANAGER_H
#define SEGMENTMANAGER_H

#include "SegmentInventory.h"
#include "FreeSpaceInventory.h"

#include <stdio.h>

// Provides basic functions operating on segments, such as create, drop, grow
// or retrieve by id
class SegmentManager
{

public:

	// Constructor, destructor
	SegmentManager(char* filename);
	~SegmentManager();
	
	// Creates a new segment with one initial extent, and returns its id
	uint64_t createSegment();
	
	// Drops the segment with the given id. The results in a change in the FSI,
	// where the pages of the dropped segment are now recorded as being free.
	void dropSegment(uint64_t segId);
	
	// Adds an additional extent to the segment with the given id.
	// Method returns the page number of the first page of the new extent.
	uint64_t growSegment(uint64_t segId);
	
	// Returns a reference to the segment with the given id	
	Segment& retrieveSegmentById(uint64_t segId);

private:

	// If no file with the name constants::dbname exists in path, create a file
	// with two initial pages, one for the SegmentInventory, and one for the
	// FreeSpaceInventory.
	void initializeDatabase(char* filename);

	// The segment inventory, which contains the concrete mapping of segments
	// (ids) to the locations (page ranges) of their extents.
	SegmentInventory* segInv;
	
	// The inventory managing the free space (and fragmentation) of the database
	FreeSpaceInventory* spaceInv;
	
	// Handler to the database file
	FILE* dbFile;
};


#endif  // SEGMENTMANAGER_H
