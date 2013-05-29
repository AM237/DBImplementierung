
///////////////////////////////////////////////////////////////////////////////
// SegmentInventory.cpp
///////////////////////////////////////////////////////////////////////////////


#include "SegmentInventory.h"
#include <fcntl.h>
#include <iostream>
#include <algorithm>
#include <queue>

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
	currentPage = 0;
}

// _____________________________________________________________________________
uint64_t SegmentInventory::nextPage()
{
	uint64_t temp = currentPage;
	
	/*
	// Get all extents for SI
	pair<multimap<uint64_t, Extent>::iterator, 
	     multimap<uint64_t, Extent>::iterator> ret;
    ret = entries.equal_range(0);
    
    for (multimap<uint64_t, Extent>::iterator it=ret.first; it!=ret.second; ++it)
	{
		Extent e = it->second;
		for(uint64_t i = e.start; i <= e.end; i++)
			frames.push(i);
	}*/
}


// _____________________________________________________________________________
void SegmentInventory::getSIExtents(vector<Extent>& accu, BufferFrame& frame, 
                                    uint64_t& counter)
{
	uint64_t* data = reinterpret_cast<uint64_t*>(frame.getData());
	uint64_t limit = min(counter, maxEntries);
	uint64_t offset = accu.size();
	
	for (unsigned int i = 1; i < 3*limit; i=i+3)
	{
		counter--;
		
		// Fill map of segment ids to extents
		Extent ext(data[i+1], data[i+2]);
		entries.insert(std::pair<uint64_t, Extent>(data[i], ext));
		
		// If current entry refers to SI, mark the extent to follow next,
		// update the size of the SI
		if (data[i] == 0)
		{
			accu.push_back(ext);
			size += ext.end - ext.start + 1; 
		}
	}
	
	bm->unfixPage(frame, false);
	uint64_t newSize = accu.size();
	
	// Recursive call, gets called only if additional extents were detected
	for (size_t i = offset; i < newSize; i++)
	{
		Extent e = extents[i];
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
		size = 1;
		Extent segExt(0, 0);
		Extent fsiExt(1, 1);
		entries.insert(std::pair<uint64_t, Extent>(0, segExt));
		entries.insert(std::pair<uint64_t, Extent>(1, fsiExt));
		extents.push_back(segExt);
		
		// reflect current state of data structures to file
		writeToFile();
		bm->unfixPage(bootFrame, true);
		return;
	}
	
	// File contains meaningful data -> Map data to inventory entries. 
	// The SI starts on page 0, so read this page first, and accumulate
	// all references to other pages (tuples of the form <0, x, y>), then
	// recursively read these pages.
	uint64_t entryCounter = numEntries;
	getSIExtents(extents, bootFrame, entryCounter);
	nextId = entries.rbegin()->first + 1;
}

// _____________________________________________________________________________
void SegmentInventory::writeToFile()
{
	// Search for all extents pertaining to segment inventory (id = 0)
	pair<multimap<uint64_t, Extent>::iterator, 
	     multimap<uint64_t, Extent>::iterator> ret;
    ret = entries.equal_range(0);
    
    // Get frames within those extents. Note: there might be more pages
    // in the extents than necessary (i.e. inventory fills up a little more
    // than the first page -> extra extent assigned to the SI, even though
    // only a small portion of it is actually used)
    priority_queue<uint64_t, vector<uint64_t>, greater<uint64_t> > frames;
    for (multimap<uint64_t, Extent>::iterator it=ret.first; it!=ret.second; ++it)
	{
		Extent e = it->second;
		for(uint64_t i = e.start; i <= e.end; i++)
			frames.push(i);
	}
	
	// Accumulate the tupels in a buffer, and when it overflows, write to
	// the next page given by the queue
	vector<uint64_t> buffer;
		
	// Every page in the SI carries this header
	buffer.push_back(numEntries);
	
	// The number of entries that may still be written to page,
	// controls overflow
	uint64_t entryCounter = maxEntries;
	
	// Mark last iteration of the following loop
	multimap<uint64_t, Extent>::iterator final_iter = entries.end();
	--final_iter;
	
	// Loop through all data to be written to file
	for (multimap<uint64_t, Extent>::iterator it = entries.begin();
	     it != entries.end(); ++it)
	{
		buffer.push_back(it->first);
		buffer.push_back((it->second).start);
		buffer.push_back((it->second).end);
		
		entryCounter--;
		if (entryCounter == 0 || it == final_iter)
		{
			// Get the next available frame for the SI
			uint64_t page = frames.top();
			frames.pop();
			
			// write to file
			BufferFrame& bf = bm->fixPage(page, true);
			writeToArray(buffer.data(), bf.getData(), buffer.size(), 0);
			bm->unfixPage(bf, true);
			
			// reset buffer
			buffer.clear();
			buffer.push_back(numEntries);
			
			entryCounter = maxEntries;
		}	
	}
}
