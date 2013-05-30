
///////////////////////////////////////////////////////////////////////////////
// Segment.h
//////////////////////////////////////////////////////////////////////////////


#ifndef SEGMENT_H
#define SEGMENT_H

#include <stdint.h>
#include <vector>
#include <algorithm>

// An object representing an extent in a segment. Bound given as [start, end)
struct Extent
{
public: 
	Extent(uint64_t start, uint64_t end) : start(start), end(start) { }
	uint64_t start;
	uint64_t end;
};



// Abstract class, represents a segment in the DBMS
class Segment
{
	friend class SegmentInventory;

public:

	// Constructor, destructor
	Segment(bool visible, uint64_t id)
	{ 
		this->visible = visible; 
		this->id = id;
		this->nextPageCounter = 0;
	}
	
	virtual ~Segment() { }
	
	// Returns whether this segment is public (true) or private (false)
	bool getVisibility() { return visible; }
	
	// Returns the id of this segment
	uint64_t getId() { return id; }
	
	// Returns the size of this segment in frames	
	uint64_t getSize() { return size; }

	// Returns the page id of the next page in the segment. Calling this method
	// for the first time gives the first page of the segment, calling it twice
	// gives the second page, and so on. If no more pages are available, the
	// id of the last available page is returned.
	uint64_t nextPage()
	{
		std::vector<uint64_t> pages;
	
		// Get all extents, get pages from each extent and return next page
		// in order (note: there should be no duplicates due to def. of extents)
		for (size_t i = 0; i < extents.size(); i++)
		{
			Extent e = extents[i];
			for(uint64_t j = e.start; j < e.end; j++)
				pages.push_back(j);
		}
	
		sort(pages.begin(), pages.end());
		if (nextPageCounter >= pages.size())
			return pages[pages.size()-1];

		return pages[nextPageCounter++];
	}

	
protected:

	// Writes (copies) data from one array to another
	void writeToArray(uint64_t* from, void* to, int elems, int offset)
	{
		for (int i = offset; i < offset+elems; i++)
			reinterpret_cast<uint64_t*>(to)[i] = from[i];
	}

	// The extents in this segment
	std::vector<Extent> extents;

	// Defines whether this segment is public or private
	bool visible;
	
	// Id of this segment
	uint64_t id;
	
	// the size of this segment in pages
	uint64_t size;
	
	// the number of times nextPage has been called on this segment
	uint64_t nextPageCounter;
};

#endif  // SEGMENT_H
