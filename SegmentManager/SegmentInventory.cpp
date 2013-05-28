
///////////////////////////////////////////////////////////////////////////////
// SegmentInventory.cpp
///////////////////////////////////////////////////////////////////////////////


#include "SegmentInventory.h"
#include <fcntl.h>
#include <iostream>
#include <algorithm>

using namespace std;

// Comparison function for InventoryEntries, used to sort data structure
// containing all inventory entries for faster access to a segment with any
// given id
bool invEntryComp (InventoryEntry e1, InventoryEntry e2) 
{ 
	return (e1.segmentId < e2.segmentId); 
}

// _____________________________________________________________________________
SegmentInventory::SegmentInventory(BufferManager* bm, bool visible, uint64_t id) 
                : Segment(visible, id)
{
	nextId = 2;
	this->bm = bm;
	initializeFromFile();

}

// _____________________________________________________________________________
void* SegmentInventory::nextPage()
{
	return nullptr;
}

// _____________________________________________________________________________
void SegmentInventory::initializeFromFile()
{
	// Read in information available in frame #0
	BufferFrame& bootFrame = bm->fixPage(0, true);
	uint64_t numEntries = reinterpret_cast<uint64_t*>(bootFrame.getData())[0];
	
	// TODO
	if (numEntries == 0)
	{
	
	}
	
	// Each entry is composed of 3 unsigned integers, for a total of 24 bytes.
	// Compute how many pages the segment inventory takes up
	this->size = (numEntries * 24) / constants::pageSize;
	
	// Map data to inventory entries. Assumption: inventory takes up only 1 page
	// TODO: inventory can take up more pages
	uint64_t* inv = reinterpret_cast<uint64_t*>(bootFrame.getData());
	for (unsigned int i = 1; i < 3*numEntries; i=i+3)
	{
		InventoryEntry entry(inv[i], inv[i+1], inv[i+2]);
		entries.push_back(entry);
	}
	
	// Optimize for later searches, possibly with binary search or interpolation
	sort(entries.begin(), entries.end(), invEntryComp);
}

// _____________________________________________________________________________
void SegmentInventory::writeToFile()
{


}
