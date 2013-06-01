
///////////////////////////////////////////////////////////////////////////////
// SegmentManager.cpp
///////////////////////////////////////////////////////////////////////////////

#include "SegmentManager.h"

#include <fcntl.h>
#include <iostream>
#include <vector>
#include <math.h>

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
SegmentManager::~SegmentManager()
{
	// Drop non permanent segments
	for (auto it=segInv->segments.begin(); it!=segInv->segments.end(); ++it)
		if (!(it->second)->permanent) dropSegment(it->first);
		
	// Clean up components
	delete segInv;
	delete spaceInv;
	delete bm;
}

// _____________________________________________________________________________
Segment* SegmentManager::retrieveSegmentById(uint64_t segId)
{
	return segInv->getSegment(segId);
}

// _____________________________________________________________________________
uint64_t SegmentManager::createSegment(bool visible)
{	
	// See if there is a free extent available with sufficient pages
	uint64_t base = segManConst::baseExtentSize;
	Extent e = spaceInv->getExtent(base);
	
	// If no such extent found, then the size of the database file must be
	// increased to accomodate space for the new segment
	if (e.start == e.end)
	{
		pair<uint64_t, uint64_t> growth = bm->growDB(base);
		Extent grownExtent(growth.first, growth.second);
		spaceInv->registerExtent(grownExtent);
		createSegment(visible);
	}
	
	else
	{
		// e has already been unregistered from the FSI
		uint64_t newId = segInv->getNextId(); 
		Segment* newSeg = new RegularSegment(e, visible, newId);
		bool found = segInv->registerSegment(newSeg);
		if (found)
		{
			cout << "Error creating segment: id already taken" << endl;
			exit(1);
		}
		return newId;
	}
	
	return 0;
}

// _____________________________________________________________________________
uint64_t SegmentManager::growSegment(uint64_t segId)
{
	// TODO: SI, FSI grow automatically -> modify writeToFile
	Segment* toGrow = retrieveSegmentById(segId);
	if (toGrow == nullptr)
	{
		cout << "Error growing segment: no segment with id " << segId 
			 << " was found" << endl;
		exit(1);
	}
	
	// Grow segment according to dynamic extent mapping.
	// Let the base size of an extent be b. If the segment has n > 0 extents,
	// Then the ith extent has size b^(k^(i-1)), where k is the exponential
	// growth constant. Therefore, the extent that will be added now will have
	// size b^(k^(n))
	float numExtents = toGrow->extents.size();
	uint64_t newExtentSize = pow(segManConst::baseExtentSize, 
	                         	 pow(segManConst::extentIncrease, numExtents));
	
	// Request an extent with required capacity   	
	Extent e = spaceInv->getExtent(newExtentSize);
	
	// If no such extent found, then the size of the database file must be
	// increased to accomodate space for the new extent
	if (e.start == e.end)
	{
		pair<uint64_t, uint64_t> growth = bm->growDB(newExtentSize);
		Extent grownExtent(growth.first, growth.second);
		spaceInv->registerExtent(grownExtent);
		growSegment(segId);
	}
	
	else
	{
		// e has already been unregistered from the FSI
		toGrow->extents.push_back(e);
		return e.start;
	}
	
	return 0;
}

// _____________________________________________________________________________
void SegmentManager::dropSegment(uint64_t segId)
{
	Segment* toDrop = retrieveSegmentById(segId);
	if (toDrop == nullptr)
	{
		cout << "Error dropping segment: no segment with id " << segId 
			 << " was found" << endl;
		exit(1);
	}
	
	// Update FSI
	vector<Extent>& toDropExtents = toDrop->extents;
	for (size_t i = 0; i < toDropExtents.size(); i++)
		spaceInv->registerExtent(toDropExtents[i]);
		
	// Update SI
	segInv->unregisterSegment(toDrop);
	
	// Clean memory
	delete toDrop;
}
