
///////////////////////////////////////////////////////////////////////////////
// FreeSpaceInventory.cpp
///////////////////////////////////////////////////////////////////////////////


#include "FreeSpaceInventory.h"
#include <queue>

using namespace std;

// _____________________________________________________________________________
FreeSpaceInventory::FreeSpaceInventory(SegmentInventory* si, 
                                       BufferManager* bm, bool visible,
                                       uint64_t id) : Segment(true, visible, id)
{
	this->si = si;
	this->bm = bm;
	maxEntries = (constants::pageSize-sizeof(uint64_t)) / (2*sizeof(uint64_t));
	initializeFromFile();
}

// _____________________________________________________________________________
void FreeSpaceInventory::registerExtent(Extent e)
{
	if (e.end <= e.start)
	{
		cout << "Attempting to register an invalid extent: [" << e.start
		     << ", " << e.end << ")" << endl;
		exit(1);
	}
	
	// The FSI contains a set of extents describing free pages.
	// At all times, these extents do not overlap and describe maximal
	// continuous regions (i.e. [1, 4) instead of [1, 3), [3, 4))
	// Therefore, when marking a new extent as free, check three cases:
	//
	// 1. The extent starts where another free extent ends -> extend previous
	// 2. The extent ends where another free extent begins -> extend into next
	// 3. Otherwise just add this extent to the set of extents.
	
	auto endIt = reverseMap.find(e.start);
	auto startIt = forwardMap.find(e.end);
	uint64_t begin = e.start;
	bool merge = false;
	
	if (endIt != reverseMap.end())
	{
		merge = true;
		begin = endIt->second;
		forwardMap.find(begin)->second = e.end;
		reverseMap.insert(pair<uint64_t, uint64_t>(e.end, begin));
		reverseMap.erase(endIt);
	}
	
	if (startIt != forwardMap.end())
	{
		uint64_t end = forwardMap.find(e.end)->second;
		
		if (merge) forwardMap.find(begin)->second = end;
		else	   forwardMap.insert(pair<uint64_t, uint64_t>(begin, end)); 	   
			
		reverseMap.find(end)->second = begin;
		forwardMap.erase(e.end);
						
		merge = true;
	}
	
	// No merge with any other extent took place -> add new extent entry.
	// Note: this is the only instance where numEntries is increased in the FSI
	if (!merge)
	{
		numEntries++;
		forwardMap.insert(pair<uint64_t, uint64_t>(e.start, e.end));
		reverseMap.insert(pair<uint64_t, uint64_t>(e.end, e.start));
		
		// grow segment if necessary
		if (maxEntries * getSize() <= numEntries) grow();
	}
}

// _____________________________________________________________________________
void FreeSpaceInventory::grow()
{	
	// Grow segment according to dynamic extent mapping.
	// See SegmentManager::growSegment
	float numExtents = extents.size();
	uint64_t newExtentSize = ceil(pow(2, 
	                         	  pow(params.extentIncrease, numExtents)));
	
	// Get an extent that is known to be clean -> allocate new set of pages	
	pair<uint64_t, uint64_t> growth = bm->growDB(newExtentSize);
	Extent e(growth.first, growth.second);
	extents.push_back(e);
	si->notifySegGrowth(1, 1);
}

// _____________________________________________________________________________
Extent FreeSpaceInventory::getExtent(uint64_t numPages)
{
	for (auto it= forwardMap.begin(); it!=forwardMap.end(); ++it)
		if ((it->second - it->first) >= numPages)
		{
			Extent e(it->first, it->first + numPages);
			
			// Space required fills up an entire extent -> remove entire entry
			if (e.end == it->second)
			{
				numEntries--;
				forwardMap.erase(it->first);
				reverseMap.erase(it->second);
				return e;
			}
			
			// Otherwise space required fills up only a portion of an available
			// extent -> update data structures approriately and return extent
			forwardMap.erase(e.start);
			forwardMap.insert(pair<uint64_t, uint64_t>(e.end, it->second));
			reverseMap.find(it->second)->second = e.end;

			return e;
		}


	Extent e(0, 0);
	return e;
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
		Extent ext(1, 2);
		extents.push_back(ext);
		
		// page #2 is free
		numEntries = 1;
		forwardMap.insert(pair<uint64_t, uint64_t>(2, 3));
		reverseMap.insert(pair<uint64_t, uint64_t>(3, 2));
		return;
	}
	
	
	// File contains meaningful data -> the SI has already initialized the
	// FSI and set its extents -> initialize mappings and other structures
	else 
	{
		if (extents.size() == 0)
		{
			cout << "Error initializing FSI: SI loaded data from file, but "
			     << "FSI has no defined extents" << endl;
			exit(1);
		}
		
		for (size_t i = 0; i < extents.size(); i++)
		{
			numEntries++;
			Extent e = extents[i];
			forwardMap.insert(pair<uint64_t, uint64_t>(e.start, e.end));
			reverseMap.insert(pair<uint64_t, uint64_t>(e.end, e.start));
		}
	
		return;
	} 
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
		{
			frames.push(i);
		}
	}

	// Accumulate the ranges in a buffer, and when it overflows, write to
	// the next page given by the queue
	vector<uint64_t> buffer;
	
	// Every page in the FSI carries this header
	buffer.push_back(numEntries);
	
	// The number of entries that may still be written to page,
	// controls overflow
	uint64_t entryCounter = maxEntries;
	
	// Loop through all data to be written to file
	for (auto it=forwardMap.begin(); it!=forwardMap.end(); ++it)
	{
		buffer.push_back(it->first);
		buffer.push_back(it->second);
			
		entryCounter--;
			
		// If no more tuples fit on page or this is the final iteration
		// on both loops, flush the buffer
		if (entryCounter == 0 || it == (--forwardMap.end()))
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


