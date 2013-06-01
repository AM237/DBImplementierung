
///////////////////////////////////////////////////////////////////////////////
// SegmentInventory.cpp
///////////////////////////////////////////////////////////////////////////////


#include "SegmentInventory.h"
#include "FreeSpaceInventory.h"
#include "RegularSegment.h"
#include <queue>
#include <fcntl.h>
#include <iostream>
#include <algorithm>
#include <unordered_set>

using namespace std;

// _____________________________________________________________________________
SegmentInventory::SegmentInventory(BufferManager* bm, bool visible, uint64_t id) 
                : Segment(true, visible, id)
{
	this->bm = bm;
	maxEntries = (constants::pageSize-sizeof(uint64_t)) / (3*sizeof(uint64_t));
	initializeFromFile();
}

// _____________________________________________________________________________
SegmentInventory::~SegmentInventory()
{	
	for (auto it=segments.begin(); it!=segments.end(); ++it)
		delete it->second;
}

//______________________________________________________________________________
bool SegmentInventory::registerSegment(Segment* seg)
{
	if (segments.find(seg->id) != segments.end()) return true;
	segments.insert(pair<uint64_t, Segment*>(seg->id, seg));
	numEntries=numEntries+seg->extents.size();
	return false;
}

//______________________________________________________________________________
bool SegmentInventory::unregisterSegment(Segment* seg)
{
	if (segments.find(seg->id) == segments.end()) return false;
	segments.erase(seg->id);
	numEntries=numEntries-seg->extents.size();
	return true;
}

//______________________________________________________________________________
Segment* SegmentInventory::getSegment(uint64_t id)
{
	auto seg = segments.find(id);
	if (seg == segments.end()) return nullptr;
	return seg->second;
}

//______________________________________________________________________________
uint64_t SegmentInventory::getNextId()
{
	return nextId++;
}


// _____________________________________________________________________________
void SegmentInventory::parseSIExtents(multimap<uint64_t, Extent, comp>& mapping, 
                                     BufferFrame& frame, uint64_t& counter)
{
	uint64_t* data = reinterpret_cast<uint64_t*>(frame.getData());
	uint64_t limit = min(counter, maxEntries);
	vector<Extent> exts;
	
	for (unsigned int i = 1; i < 3*limit; i=i+3)
	{
		counter--;
		uint64_t id = data[i];
		
		// Fill map of segment ids to extents
		Extent ext(data[i+1], data[i+2]);
		mapping.insert(pair<uint64_t, Extent>(id, ext));
		
		if (id == 0) exts.push_back(ext);
		if (id >= nextId) nextId = id+1; 
	}
	
	bm->unfixPage(frame, false);
	
	// Recursive call, gets called only if additional extents were detected
	for (size_t i = 0; i < exts.size(); i++)
	{
		Extent e = exts[i];
		for (uint64_t j = e.start; j < e.end; j++)
		{
			BufferFrame& bf = bm->fixPage(j, true);
			parseSIExtents(mapping, bf, counter);
		}
	}
}



// _____________________________________________________________________________
void SegmentInventory::initializeFromFile()
{
	// Read in information available starting in frame #0
	BufferFrame& bootFrame = bm->fixPage(0, true);
	numEntries = reinterpret_cast<uint64_t*>(bootFrame.getData())[0];
	
	// File is yet to be initialized and contains no information
	if (numEntries == 0)
	{
		bm->unfixPage(bootFrame, false);
		
		// update data structures
		nextId = 2;
		numEntries = 1;
		Extent ext(0, 1);
		extents.push_back(ext);
		
		// reflect current state of data structures to file
		writeToFile();
		return;
	}
	
	// File contains meaningful data -> create segments in main memory
	// based on given data.
	//
	// The SI starts on page 0, so read this page first, and accumulate
	// all references to other pages (tuples of the form <0, x, y>), then
	// recursively read these pages.
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
			if (segId == 1) newSeg = new FreeSpaceInventory(bm, false, segId);
			else            newSeg = new RegularSegment(true, segId);
			
			if (newSeg != nullptr) 
			{
				newSeg->extents.push_back(it->second);
				segments.insert(pair<uint64_t, Segment*>(segId, newSeg));
			}
		}
	}
}

// _____________________________________________________________________________
void SegmentInventory::writeToFile()
{	
	// Handle SI extents:
	//
    // Get frames within those extents. Note: there might be more pages
    // in the extents than necessary (i.e. inventory fills up a little more
    // than the first page -> extra extent assigned to the SI, even though
    // only a small portion of it is actually used)
	priority_queue<uint64_t, vector<uint64_t>, greater<uint64_t> > frames;
	for (size_t i = 0; i < extents.size(); i++)
	{
		Extent e = extents[i];
		for(uint64_t i = e.start; i < e.end; i++)
			frames.push(i);
	}
	
	
	// Handle all other segments:
	//
	// Accumulate the tupels in a buffer, and when it overflows, write to
	// the next page given by the queue
	vector<uint64_t> buffer;
	
	// Every page in the SI carries this header
	buffer.push_back(numEntries);
	
	// The number of entries that may still be written to page,
	// controls overflow
	uint64_t entryCounter = maxEntries;
	
	// Mark last iteration of the following outer loop
	auto final_iter = segments.end();
	--final_iter;
	
	// Loop through all data to be written to file
	for (auto it=segments.begin(); it!=segments.end(); ++it)
	{
		Segment* seg = it->second;
		vector<Extent>& exts = seg->extents;
		
		// Current segment's extents
		for (size_t j = 0; j < exts.size(); j++)
		{
			// SegId | StartPageNo | EndPageNo
			buffer.push_back(it->first);
			buffer.push_back(exts[j].start);
			buffer.push_back(exts[j].end);
			
			entryCounter--;
			
			// If no more tuples fit on page or this is the final iteration
			// on both loops, flush the buffer
			if (entryCounter == 0 || (it == final_iter && j == exts.size()-1))
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
}
