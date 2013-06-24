
///////////////////////////////////////////////////////////////////////////////
// SPSegment.cpp
///////////////////////////////////////////////////////////////////////////////

#include "SPSegment.h"

using namespace std;

// _____________________________________________________________________________
SPSegment::SPSegment(BufferManager* bm, bool visible, uint64_t id, Extent* base) 
               : RegularSegment(visible, id, base)
{ 
	this->bm = bm;

	// Initialize this segment's internal FSI and materialize it to file
	// See SegmentFSI: it is assumed that the fsi initially fits on one page
	fsi = new SegmentFSI(this->getSize(), this->firstPage());
	BufferFrame& bf = bm->fixPage(this->firstPage(), true);
	memcpy(bf.getData(), fsi, sizeof(*fsi));
	bm->unfixPage(bf, true);
}

// _____________________________________________________________________________
SPSegment::~SPSegment()
{
	delete fsi;
}

