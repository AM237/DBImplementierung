
///////////////////////////////////////////////////////////////////////////////
// RegularSegment.cpp
///////////////////////////////////////////////////////////////////////////////


#include "RegularSegment.h"
#include <algorithm>

using namespace std;


// _____________________________________________________________________________
RegularSegment::RegularSegment(Extent base, bool visible, uint64_t id) 
               : Segment(true, visible, id)
{
	extents.push_back(base);
}

//______________________________________________________________________________
RegularSegment::RegularSegment(bool visible, uint64_t id) 
               : Segment(true, visible, id)
{
}

