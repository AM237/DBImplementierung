
///////////////////////////////////////////////////////////////////////////////
// SegmentInventory.cpp
///////////////////////////////////////////////////////////////////////////////


#include "SegmentInventory.h"
#include <fcntl.h>
#include <iostream>
#include <algorithm>

using namespace std;

// Comparison function for InventoryEntries
bool invEntryComp (InventoryEntry e1, InventoryEntry e2) 
{ 
	return (e1.segmentId < e2.segmentId); 
}

// _____________________________________________________________________________
SegmentInventory::SegmentInventory(int fd, bool visible, uint64_t id) 
                : Segment(visible, id)
{
	nextId = 2;
	fileDescriptor = fd;
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
	// Read in information availabe in frame #0
	vector<uint64_t> input;
	input.resize(constants::pageSize);

	if (lseek(fileDescriptor, 0, SEEK_SET) < 0 ||
	    read(fileDescriptor, input.data(), constants::pageSize) < 0)
	{
		cout << "Error initializing segment inventory from file" << endl;
		exit(1);
	}
	
	numEntries = input[0];
	if (numEntries == 0) return;
	
	for(size_t i = 1; i < input.size(); i = i+3)
	{
		InventoryEntry entry(input[i], input[i+1], input[i+2]);
		entries.push_back(entry);
	}
	
	// Optimize for later searches, possibly with binary search or interpolation
	sort(entries.begin(), entries.end(), invEntryComp);

}

// _____________________________________________________________________________
void SegmentInventory::writeToFile()
{


}
