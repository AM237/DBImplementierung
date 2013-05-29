
///////////////////////////////////////////////////////////////////////////////
// SegmentInventory.cpp
///////////////////////////////////////////////////////////////////////////////


#include "SegmentInventory.h"
#include <fcntl.h>
#include <iostream>
#include <algorithm>

using namespace std;


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
	this->bm = bm;
	maxEntries = (constants::pageSize-sizeof(uint64_t)) / (3*sizeof(uint64_t));
	initializeFromFile();
}

// _____________________________________________________________________________
void* SegmentInventory::nextPage()
{
	return nullptr;
}


// _____________________________________________________________________________
void SegmentInventory::getSIExtents(vector<Extent>& accu, BufferFrame& frame, 
                                    uint64_t& counter)
{
	uint64_t* data = reinterpret_cast<uint64_t*>(frame.getData());
	uint64_t limit = min(counter, maxEntries);
	size_t offset = accu.size(); 
	
	for (unsigned int i = 1; i < 3*limit; i=i+3)
	{
		counter--;
		
		// Fill map of segment ids to extents
		Extent ext(data[i+1], data[i+2]);
		entries.insert(std::pair<uint64_t, Extent>(data[i], ext));
		
		// If current entry refers to SI, mark the extent to follow next
		if (data[i] == 0)
			accu.push_back(ext);
	}
	
	bm->unfixPage(frame, false);
	size_t newSize = accu.size(); 
	
	// Recursive call, gets called only if size of accu increased above
	for (size_t i = offset; i < newSize; i++)
	{
		Extent e = accu[i];
		for (uint64_t j = e.start; j <= e.end; j++)
		{
			BufferFrame& bf = bm->fixPage(j, true);
			getSIExtents(accu, bf, counter);
		}
	}
}

// _____________________________________________________________________________
void SegmentInventory::initializeFromFile()
{
	// Read in information available in frame #0
	BufferFrame& bootFrame = bm->fixPage(0, true);
	numEntries = reinterpret_cast<uint64_t*>(bootFrame.getData())[0];
	
	// File is yet to be initialized and contains no information
	if (numEntries == 0)
	{
		// update data structures
		nextId = 2;
		numEntries = 2;
		Extent segExt(0, 0);
		Extent fsiExt(1, 1);
		entries.insert(std::pair<uint64_t, Extent>(0, segExt));
		entries.insert(std::pair<uint64_t, Extent>(1, fsiExt));
		
		// reflect current state of data structures to file
		writeToFile(bootFrame);
		return;
	}
	
	// File contains meaningful data -> Map data to inventory entries. 
	// The SI starts on page 0, so read this page first, and accumulate
	// all references to other pages (tuples of the form <0, x, y>), then
	// recursively read these pages.
	vector<Extent> sIExtents;
	uint64_t entryCounter = numEntries;
	getSIExtents(sIExtents, bootFrame, entryCounter);
	nextId = entries.rbegin()->first + 1;
}

// _____________________________________________________________________________
void SegmentInventory::writeToFile(BufferFrame& frame)
{
	vector<uint64_t> dir;
	
	// Header: number of entries in inventory
	dir.push_back(numEntries);
	
	// Body: segmentId + start / end offsets
	for (multimap<uint64_t,Extent>::iterator it=entries.begin(); 
	     it!=entries.end(); ++it)
	{
		Extent ext = (*it).second;
		dir.push_back((*it).first);
		dir.push_back(ext.start);
		dir.push_back(ext.end);
	}
	
	writeToArray(dir.data(), frame.getData(), dir.size(), 0);
}
