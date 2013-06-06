
///////////////////////////////////////////////////////////////////////////////
// RegularSegment.cpp
///////////////////////////////////////////////////////////////////////////////


#include "RegularSegment.h"
#include <algorithm>

using namespace std;


// _____________________________________________________________________________
RegularSegment::RegularSegment(bool visible, uint64_t id, Extent* base) 
               : Segment(true, visible, id, base)
{

}

/*
//______________________________________________________________________________
RegularSegment::RegularSegment(bool visible, uint64_t id) 
               : Segment(true, visible, id)
{
}*/

