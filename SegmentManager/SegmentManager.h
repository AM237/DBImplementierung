
///////////////////////////////////////////////////////////////////////////////
// SegmentManager.h
//////////////////////////////////////////////////////////////////////////////


#ifndef SEGMENTMANAGER_H
#define SEGMENTMANAGER_H

#include "../BufferManager/BufferManager.h"
#include "SegmentInventory.h"
#include "FreeSpaceInventory.h"
#include "SegManConst.h"

#include <stdio.h>
#include <string>


// Provides basic functions operating on segments, such as create, drop, grow
// or retrieve by id. See constructor for further details.
class SegmentManager
{

public:

	// Constructor: creates and initializes the FSI, SI, and BufferManager.
	// The FSI and SI are special segments that live and are destroyed with
	// the SM. Otherwise, the SM stores RegularSegments only, that is, it has
	// no knowledge of their internal format, only their layout on disk.
	// The Metadata layer is responsible for knowing which specialized segments
	// have which id and to interpret the results returned by the SM accordingly 
	FRIEND_TEST(SegmentManagerTest, initializeNoFile);
	FRIEND_TEST(SegmentManagerTest, initializeWithFile);
	SegmentManager(const std::string& filename);
	~SegmentManager();
	
	// Creates a new segment with one initial extent, and returns its id
	FRIEND_TEST(SegmentManagerTest, createGrowDropSegment);
	uint64_t createSegment(bool visible);
	
	// Drops the segment with the given id. The results in a change in the FSI,
	// where the pages of the dropped segment are now recorded as being free.
	//
	// FRIEND_TEST(SegmentManagerTest, createGrowDropSegment);
	void dropSegment(uint64_t segId);
	
	// Adds an additional extent to the segment with the given id.
	// Method returns the page number of the first page of the new extent.
	//
	// FRIEND_TEST(SegmentManagerTest, createGrowDropSegment);
	uint64_t growSegment(uint64_t segId);
	
	// Returns a pointer to the segment with the given id	
	// If no such segment exists, a nullptr is returned
	//
	// FRIEND_TEST(SegmentManagerTest, initializeWithFile);
	Segment* retrieveSegmentById(uint64_t segId);

	// Returns a reference to the buffer manager.
	BufferManager& getBufferManager();


private:

	// BufferManager handler
	BufferManager* bm;

	// The segment inventory, which contains the concrete mapping of segments
	// (ids) to the locations (page ranges) of their extents.
	SegmentInventory* segInv;
	
	// The inventory managing the free space (and fragmentation) of the database
	FreeSpaceInventory* spaceInv;
	
	// Segment manager parameters
	SegManConst params;
};


#endif  // SEGMENTMANAGER_H
