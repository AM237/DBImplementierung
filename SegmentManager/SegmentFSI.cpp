
///////////////////////////////////////////////////////////////////////////////
// SegmentFSI.cpp
///////////////////////////////////////////////////////////////////////////////


#include "SegmentFSI.h"
#include <math.h> 

using namespace std;

// _____________________________________________________________________________
SegmentFSI::SegmentFSI(uint64_t numPages, uint64_t pageStart)
{ 
	// Initialize inventory (marking empty pages)
	for (int i = 0; i < ceil(numPages/2); i++)
	{
		FreeSpaceEntry e = {10, 10};
		inventory.push_back(e);
	}

	// Initialize extents
	Extent e = {pageStart, pageStart+1};
	extents.push_back(e);

	// Initialize size
	this->size = sizeof(*this);		
}


