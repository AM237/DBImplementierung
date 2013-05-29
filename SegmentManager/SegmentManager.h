
///////////////////////////////////////////////////////////////////////////////
// Segment.h
//////////////////////////////////////////////////////////////////////////////


#ifndef SEGMENTMANAGER_H
#define SEGMENTMANAGER_H

#include "SegmentInventory.h"
#include "FreeSpaceInventory.h"
#include "../BufferManager/BufferManager.h"

#include <stdio.h>
#include <string>


namespace segManConst
{
	// Size of the buffer manager
	uint64_t bufferSize = 100;
	
	// Required for dynamic extent mapping, this is the exponential factor
	// by which the size of extents increases every time a segment is grown.
	const float extentIncrease = 1.2;
	
	// The size of a standard extent
	const uint64_t baseExtentSize = 10;
}




// Provides basic functions operating on segments, such as create, drop, grow
// or retrieve by id
class SegmentManager
{

public:

	// Constructor, destructor
	SegmentManager(const std::string& filename);
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

	// The segment inventory, which contains the concrete mapping of segments
	// (ids) to the locations (page ranges) of their extents.
	SegmentInventory* segInv;
	
	// The inventory managing the free space (and fragmentation) of the database
	FreeSpaceInventory* spaceInv;
	
	// BufferManager handler
	BufferManager* bm;
};


#endif  // SEGMENTMANAGER_H
