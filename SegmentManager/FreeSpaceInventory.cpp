
///////////////////////////////////////////////////////////////////////////////
// FreeSpaceInventory.cpp
///////////////////////////////////////////////////////////////////////////////


#include "FreeSpaceInventory.h"
#include <queue>

using namespace std;

// _____________________________________________________________________________
FreeSpaceInventory::FreeSpaceInventory(BufferManager* bm, bool visible,
                    uint64_t id) : Segment(visible, id)
{
	this->bm = bm;
	maxEntries = (constants::pageSize-sizeof(uint64_t)) / (2*sizeof(uint64_t));
	initializeFromFile();
}



// _____________________________________________________________________________
void FreeSpaceInventory::initializeFromFile()
{
	// Read in information available starting in frame #1
	BufferFrame& bootFrame = bm->fixPage(1, true);
	numEntries = reinterpret_cast<uint64_t*>(bootFrame.getData())[0];
	
	// File is yet to be initialized and contains no information
	if (numEntries == 0)
	{
		bm->unfixPage(bootFrame, false);
		
		// update extent data
		size = 1;
		Extent ext(1, 2);
		extents.push_back(ext);
		
		// page #2 is free
		numEntries = 1;
		freeSpace.insert(pair<uint64_t, uint64_t>(2, 3));
		
		// reflect current state of fsi to file
		writeToFile();
		return;
	}
	
	
	// File contains meaningful data -> the SI has already initialized the
	// FSI and set its extents. Therefore, there is nothing else to do here.
	else 
	{
		if (extents.size() == 0)
		{
			cout << "Error initializing FSI: SI loaded data from file, but "
			     << "FSI has no defined extents" << endl;
			exit(1);
		}
	
		return;
	} 
	
	/*
	uint64_t entryCounter = numEntries;
	multimap<uint64_t, Extent, comp> mapping;
	parseSIExtents(mapping, bootFrame, entryCounter);
	
	// Now that the mapping of segment ids to extents is complete, create and 
	// store the actual segments   
    for (auto it = mapping.begin(); it != mapping.end(); ++it )
	{
		// probe segment id, add extent if segment already stored, otherwise
		// create new segment and store extent.
		uint64_t segId = it->first;
		auto segIt = segments.find(segId);
		if (segIt != segments.end()) 
			segIt->second->extents.push_back(it->second);
			
		else {
		
			Segment* newSeg = nullptr;
			if (segId == 0) extents.push_back(it->second);
			if (segId == 1) newSeg = new FreeSpaceInventory(false, segId);
			else            newSeg = new RegularSegment(true, segId);
			
			if (newSeg != nullptr) 
			{
				newSeg->extents.push_back(it->second);
				segments.insert(pair<uint64_t, Segment*>(segId, newSeg));
			}
		}
	}*/
}

//______________________________________________________________________________
void FreeSpaceInventory::writeToFile()
{
    // Get frames within all of this segment's extents. 
    // Note: there might be more pages, in the extents than necessary 
    // (i.e. inventory fills up a little more than one first page -> 
    // extra extent assigned, even though only a small portion of 
    // it is actually used)
	priority_queue<uint64_t, vector<uint64_t>, greater<uint64_t> > frames;
	for (size_t i = 0; i < extents.size(); i++)
	{
		Extent e = extents[i];
		for(uint64_t i = e.start; i < e.end; i++)
			frames.push(i);
	}

	// Accumulate the ranges in a buffer, and when it overflows, write to
	// the next page given by the queue
	vector<uint64_t> buffer;
	
	// Every page in the FSI carries this header
	buffer.push_back(numEntries);
	
	// The number of entries that may still be written to page,
	// controls overflow
	uint64_t entryCounter = maxEntries;
	
	// Mark last iteration of the following outer loop
	auto final_iter = freeSpace.end();
	--final_iter;
	
	// Loop through all data to be written to file
	for (auto it=freeSpace.begin(); it!=freeSpace.end(); ++it)
	{
		uint64_t start = it->first;
		uint64_t end = it->second;
		
		buffer.push_back(it->first);
		buffer.push_back(it->second);
	
		entryCounter--;
			
		// If no more tuples fit on page or this is the final iteration
		// on both loops, flush the buffer
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


