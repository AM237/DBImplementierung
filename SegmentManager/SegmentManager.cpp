
///////////////////////////////////////////////////////////////////////////////
// SegmentManager.cpp
///////////////////////////////////////////////////////////////////////////////

#include "SegmentManager.h"

#include <fcntl.h>
#include <iostream>
#include <vector>

using namespace std;

// _____________________________________________________________________________
SegmentManager::SegmentManager(const string& filename)
{
	// Start with three pages, one page for the segment inventory,
	// one page for the space inventory, and one free page
	bm = new BufferManager(filename, segManConst::bufferSize, 3);
	
	// segment inventory always has id = 0, space inventory always has id = 1
	segInv = new SegmentInventory(bm, false, 0);
	spaceInv = new FreeSpaceInventory(bm, false, 1);
	bool found = segInv->registerSegment(spaceInv);
	
	// If the FSI was already registered to the SI, it means that meaningful
	// data (including the FSI and its extents) were found on file, which
	// in turn means that the SI already created the FSI instance and updated
	// its state.
	if (found)
	{
		delete spaceInv;
		spaceInv = (FreeSpaceInventory*)segInv->getSegment(1);
		if (spaceInv == nullptr)
		{
			cout << "Error retrieving FSI from SI" << endl;
			exit(1);
		}
	}
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
Segment* SegmentManager::retrieveSegmentById(uint64_t segId)
{
	return segInv->getSegment(segId);
}



// _____________________________________________________________________________
SegmentManager::~SegmentManager()
{
	delete segInv;
	delete spaceInv;
	delete bm;
}
