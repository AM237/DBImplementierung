
///////////////////////////////////////////////////////////////////////////////
// SegmentManager.cpp
///////////////////////////////////////////////////////////////////////////////

#include "SegmentManager.h"
#include "RegularSegment.h"
#include "SPSegment.h"

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
	bm = new BufferManager(filename, params.bufferSize, SMConst::dbSize);
	
	// segment inventory always has id = 0, space inventory always has id = 1
	segInv = new SegmentInventory(bm, false, 0);	
	spaceInv = (FreeSpaceInventory*)segInv->getSegment(1);
	if (spaceInv == nullptr) 
	{
		spaceInv = new FreeSpaceInventory(segInv, bm, false, 1);
		segInv->registerSegment(spaceInv);
	}
}


// _____________________________________________________________________________
SegmentManager::~SegmentManager()
{
	// Drop non permanent segments
	for (auto it=segInv->segments.begin(); it!=segInv->segments.end(); ++it)
		if (!(it->second)->permanent) dropSegment(it->first);
	
	// Clean up components
	if (segInv != nullptr) delete segInv;	
	if (spaceInv != nullptr) delete spaceInv;
	if (bm != nullptr) delete bm;
}

// _____________________________________________________________________________
Segment* SegmentManager::retrieveSegmentById(uint64_t segId)
{
	return segInv->getSegment(segId);
}


// _____________________________________________________________________________
BufferManager& SegmentManager::getBufferManager()
{
	return *bm;
}


// _____________________________________________________________________________
uint64_t SegmentManager::createSegment(segTypes type, bool visible)
{	
	// See if there is a free extent available with sufficient pages
	uint64_t base = params.baseExtentSize;
	Extent e = spaceInv->getExtent(base);
	
	// If no such extent found, then the size of the database file must be
	// increased to accomodate space for the new segment
	if (e.start == e.end)
	{
		pair<uint64_t, uint64_t> growth = bm->growDB(base);
		Extent grownExtent(growth.first, growth.second);
		spaceInv->registerExtent(grownExtent);
		return createSegment(type, visible);
	}
	
	// e has already been unregistered from the FSI
	else
	{
		uint64_t newId = segInv->setNextId();
		Segment* newSeg = nullptr;
		switch(type)
		{
			case RG_SGM:
				newSeg = new RegularSegment(visible, newId, &e);
				break;
				
			case SP_SGM:
				newSeg = new SPSegment(bm, visible, newId, &e);
				break;

			default:
				cout << "Segment type not recognized" << endl;
				exit(1);
		}		
		
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
	// Only regular segments grow on demand
	if (segId < 2)
	{
		cout << "Cannot explicitly grow SI, FSI: "
		     << "these segments grow automatically" << endl;
		exit(1);
	}
	
	auto toGrow = dynamic_cast<RegularSegment*>(retrieveSegmentById(segId));
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
	uint64_t newExtentSize = ceil(pow(params.baseExtentSize, 
	                              pow(params.extentIncrease,numExtents)));
	
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
		
		// Notify the SI as well as the respective segment that it has grown
		segInv->notifySegGrowth(toGrow->id, 1);
		toGrow->notifySegGrowth(e);
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
