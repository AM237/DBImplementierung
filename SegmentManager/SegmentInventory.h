
///////////////////////////////////////////////////////////////////////////////
// SegmentInventory.h
//////////////////////////////////////////////////////////////////////////////


#ifndef SEGMENTINVENTORY_H
#define SEGMENTINVENTORY_H

#include "../BufferManager/BufferManager.h"
#include "RegularSegment.h"
#include "SegManConst.h"

#include <gtest/gtest.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <map>


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
	friend class SegmentManager;

public:

	// Constructor. Initializes this SegmentInventory by materializing the data
	// found initially on page 0 of the database into Segments in main memory,
	// with their respectively defined extents. See initializeFromFile
	FRIEND_TEST(SegmentManagerTest, initializeNoFile);
	FRIEND_TEST(SegmentManagerTest, initializeWithFile);
	SegmentInventory(BufferManager* bm, bool visible, uint64_t id, 
	                 Extent* ex = NULL);
	
	// Destructor. Deletes only regular segments managed, the SegmentManager 
	// should take care of deleting the SI, FSI, and as well the BufferManager
	~SegmentInventory();
	
	// Registers the given segment with the segment inventory.
	// Takes place after the initialization of the inventory, that is to say,
	// the data structures of the class already hold the dirty data that will 
	// eventually be materialized to file.
	// 
	// Returns true iff segment already known to SI. In this case the method
	// returns immediately. This method performs no checks on the extents
	// of the given segment.
	bool registerSegment(Segment* seg);
	
	// Unregisters the given segment with the segment inventory.
	// Returns false iff segment not known to SI. In this case the method
	// returns immediately.
	bool unregisterSegment(Segment* seg);
	
	// Returns the segment with the given id. If no such segment is found,
	// nullptr is returned.
	Segment* getSegment(uint64_t id);
	
	// Notifies the SI that the given segment has grown by the given offset.
	// This allows the SI to allocate a new extent for itself if not enough
	// space is available to log all the extents of the newly grown segment. 
	void notifySegGrowth(uint64_t id, uint64_t offset);
	
	// Returns the next free segment id
	uint64_t getNextId();
	
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
	
	// Look up the SI's extents to get the pages where the information is to be
	// written, and write tupels describing the mapping of segment ids to 
	// extents (see initializeFromFile)
	void writeToFile();
	
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
	
	// Adds an extent to the SI. Whereas regular segments are grown on demand
	// (see SegmentManager::growSegment), the SI grows automatically.    
	void grow();

	// Handler to the buffer manager	
	BufferManager* bm;
	
	// Segment manager parameters
	SegManConst params;
	
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
