
///////////////////////////////////////////////////////////////////////////////
// SegmentManager.cpp
///////////////////////////////////////////////////////////////////////////////

#include "SegmentManager.h"

#include <fcntl.h>
#include <iostream>
#include <vector>

using namespace std;

// _____________________________________________________________________________
SegmentManager::SegmentManager(const std::string& filename)
{
	// Start with two pages, one page for the segment inventory,
	// and one page for the space inventory
	bm = new BufferManager(filename, 100, 2);
	
	// segment inventory always has id = 0, space inventory always has id = 1
	//segInv = new SegmentInventory(fileno(dbFile), false, 0);
	//spaceInv = new FreeSpaceInventory(fileno(dbFile), false, 1);
}

// _____________________________________________________________________________
uint64_t SegmentManager::createSegment()
{
	return 0;
}

// _____________________________________________________________________________
void SegmentManager::dropSegment(uint64_t segId)
{

}

// _____________________________________________________________________________
uint64_t SegmentManager::growSegment(uint64_t segId)
{
	return 0;
}

// _____________________________________________________________________________
Segment& SegmentManager::retrieveSegmentById(uint64_t segId)
{
	
}



// _____________________________________________________________________________
SegmentManager::~SegmentManager()
{
	delete bm;
	delete segInv;
	delete spaceInv;
}
