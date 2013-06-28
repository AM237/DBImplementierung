
///////////////////////////////////////////////////////////////////////////////
// FreeSpaceInventory.h
//////////////////////////////////////////////////////////////////////////////


#ifndef FREESPACEINVENTORY_H
#define FREESPACEINVENTORY_H

#include "../BufferManager/BufferManager.h"
#include "SegmentInventory.h"
#include "SMConst.h"
#include <map>

// The FreeSpaceInventory is a special segment in the DBMS. It manages and
// stores the free extents in the database. As with the SegmentInventory, it
// also has the ability to materialize its state and initialize its state
// from the database.
class FreeSpaceInventory : public Segment
{

public:

	// Constructor/destructor. Created by the SI when initializing from file
	FRIEND_TEST(SegmentManagerTest, initializeNoFile);
	FRIEND_TEST(SegmentManagerTest, initializeWithFile);
	FreeSpaceInventory(SegmentInventory* si, BufferManager* bm,
	                   bool visible, uint64_t id, Extent* ex = NULL);
	                   
	~FreeSpaceInventory() { writeToFile(); }
	
	// Give an extent to the FSI, which then incorporates its pages
	// to the inventory of total free pages
	FRIEND_TEST(SegmentManagerTest, createGrowDropSegment);
	void registerExtent(Extent e);

	// Returns an extent (page number limits) with #numPages. If none is
	// currently available due to the size of the database, an extent with
	// start = end is returned. Otherwise, this method unregisters the extent
	// from the FSI.
	//
	// FRIEND_TEST(SegmentManagerTest, createGrowDropSegment);
	Extent getExtent(uint64_t numPages);

private:

	// Initializes the free space mapping from the contents given on file
	//
	// FRIEND_TEST(SegmentManagerTest, initializeNoFile);
	// FRIEND_TEST(SegmentManagerTest, initializeWithFile);
	void initializeFromFile();
	
	// For a given page, read the data and parse it into the FSI's structures.
	//
	// FRIEND_TEST(SegmentManagerTest, initializeWithFile);
	void parseFSIExtents(uint64_t frame, uint64_t& counter);
	
	// Takes the extents of this segment, and writes the entries found in
	// freeSpace in the following format: numEntries | start1 | end1 | start2 |
	// end2 | ... etc.
	//
	// FRIEND_TEST(SegmentManagerTest, initializeNoFile);
	// FRIEND_TEST(SegmentManagerTest, initializeWithFile);
	void writeToFile();
	
	// Adds an additional extent to the FSI. Whereas regular segments are grown 
	// on demand (see SegmentManager::growSegment), the FSI grows automatically.    
	void grow();
	
	// Mapping of start of extent to end of extent, marking free space
	// Free space is given on interval [start, end)
	std::map<uint64_t, uint64_t> forwardMap;
	std::map<uint64_t, uint64_t> reverseMap;
	
	// Handler to the segment inventory
	SegmentInventory* si;
	
	// Handler to the buffer manager
	BufferManager* bm;
	
	// Segment manager parameters
	SMConst params;
	
	// Number of [start, end) entries managed by the FreeSpaceInventory
	uint64_t numEntries;
	
	// Max FSI entries per page
	uint64_t maxEntries;
};

#endif  // FREESPACEINVENTORY_H
