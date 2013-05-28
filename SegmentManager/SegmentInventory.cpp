
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

// Writes (copies) data from one array to another
void writeToArray(uint64_t* from, void* to, int elems, int offset)
{
	for (int i = offset; i < offset+elems; i++)
		reinterpret_cast<uint64_t*>(to)[i] = from[i];
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
	
	// File is yet to be initialized and contains no information
	if (numEntries == 0)
	{
		// Create base extents for the segment inventory and the free space
		// inventory, write these values to the boot frame.		
		// Format: 2 = #entries, 0 = seg inventory id, 0 = seg inv. start page,
		// 0 = seg inv. end page, 1 = FSI id,
		// 1 = FSI start page, 1 = FSI end page. 
		vector<uint64_t> dir = { 2, 0, 0, 0, 
		                            1, 1, 1};
		                            
		writeToArray(dir.data(), bootFrame.getData(), dir.size(), 0);
		
		// TODO: update data structures
		return;
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
	
	bm->unfixPage(bootFrame, false);
	
	// Optimize for later searches, possibly with binary search or interpolation
	sort(entries.begin(), entries.end(), invEntryComp);
}

// _____________________________________________________________________________
void SegmentInventory::writeToFile()
{


}
