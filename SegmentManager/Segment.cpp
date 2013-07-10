
///////////////////////////////////////////////////////////////////////////////
// Segment.cpp
///////////////////////////////////////////////////////////////////////////////


#include "Segment.h"
#include <vector>
#include <algorithm>

using namespace std;

// _____________________________________________________________________________
Segment::Segment(bool permanent, bool visible, uint64_t id, Extent* ex)
{
	this->permanent = permanent;
	this->visible = visible; 
	this->id = id;
	
	if (ex != nullptr)
	{
		Extent e(ex->start, ex->end);
		this->extents.push_back(e);
	}
}

// _____________________________________________________________________________
uint64_t Segment::getSize()
{
	uint64_t counter = 0;
	for (size_t i = 0; i < extents.size(); i++)
	{
		Extent e = extents[i];
		for(uint64_t j = e.start; j < e.end; j++) counter++;
	}
	return counter;
}

// _____________________________________________________________________________
bool Segment::inSegment(uint64_t page)
{
	bool inSegment = false;
	for (Extent& e : this->extents)
		if (e.start <= page && page < e.end)
			{ inSegment = true; break; }

	return inSegment;
}


// _____________________________________________________________________________
uint64_t Segment::nextPage(uint64_t& nextPageCounter)
{
	std::vector<uint64_t> pages;

	// Get all extents, get pages from each extent and return next page
	// in order (note: there should be no duplicates due to def. of extents)
	for (size_t i = 0; i < extents.size(); i++)
	{
		Extent e = extents[i];
		for(uint64_t j = e.start; j < e.end; j++) pages.push_back(j);
	}

	sort(pages.begin(), pages.end());
	if (nextPageCounter >= pages.size()) return pages.back();
	return pages[nextPageCounter++];
}

// _____________________________________________________________________________
uint64_t Segment::firstPage()
{
	std::vector<uint64_t> pages;
	for (size_t i = 0; i < extents.size(); i++)
	{
		Extent e = extents[i];
		for(uint64_t j = e.start; j < e.end; j++) pages.push_back(j);
	}

	sort(pages.begin(), pages.end());
	return pages[0];
}

// _____________________________________________________________________________
void Segment::writeToArray(uint64_t* from, void* to, int elems, int offset)
{
	for (int i = offset; i < offset+elems; i++)
		reinterpret_cast<uint64_t*>(to)[i] = from[i];
}
