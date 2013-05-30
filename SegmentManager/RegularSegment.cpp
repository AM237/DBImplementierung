
///////////////////////////////////////////////////////////////////////////////
// RegularSegment.cpp
///////////////////////////////////////////////////////////////////////////////


#include "RegularSegment.h"
#include <algorithm>

using namespace std;


// _____________________________________________________________________________
RegularSegment::RegularSegment(bool visible, uint64_t id) : Segment(visible, id)
{

}

// _____________________________________________________________________________
uint64_t RegularSegment::nextPage()
{
	/*
	vector<uint64_t> pages;
	
	// Get all extents for SI, get pages from each extent and return next page
	// in order (note: there should be no duplicates due to def. of extents)
    for (auto it=range.first; it!=range.second; ++it)
	{
		Extent e = it->second;
		for(uint64_t i = e.start; i <= e.end; i++)
			pages.push_back(i);
	}
	sort(pages.begin(), pages.end());
	
	if (nextPageCounter >= pages.size())
	{
		nextPageCounter++;
		return pages[pages.size()-1];
	}
	
	uint64_t value = pages[nextPageCounter];
	nextPageCounter++;
	return value;*/
}

