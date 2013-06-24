
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

	// Initialize this segment's internal FSI directly from file.
	if (this->recovered)
	{
		// Read FSI size and extents
		vector<Extent> fsiExtents;
		BufferFrame& bf = bm->fixPage(this->firstPage(), false);
		auto header = reinterpret_cast<SegmentFSI*>(bf.getData());
		uint64_t fsiSize = header->size;
		for ( Extent& e : header->extents) fsiExtents.push_back(e);
		bm->unfixPage(bf, false);

		// Read FSI from extents and stitch FSI object
		int pages = 0;
		for ( Extent& e : fsiExtents) pages += (e.end - e.start);
		unsigned char* fsibytes = new unsigned char[pages*BM_CONS::pageSize];
		unsigned char* it = fsibytes;
		for ( Extent& e : fsiExtents)
			for ( uint64_t i = e.start; i < e.end; i++)
			{
				BufferFrame& bf = bm->fixPage(i, false);
				memcpy(it, bf.getData(), BM_CONS::pageSize);
				bm->unfixPage(bf, false);
				it = it + BM_CONS::pageSize;
			}

		// take only first #fsiSize bytes
		vector<unsigned char> fsiVec(fsibytes, fsibytes+fsiSize);
		this->fsi = reinterpret_cast<SegmentFSI*>(fsiVec.data());
	}

	// If segment is being created for the first time, create a new FSI
	// and materialize it to file.
	else
	{
		fsi = new SegmentFSI(this->getSize(), this->firstPage());
		BufferFrame& bf = bm->fixPage(this->firstPage(), true);
		memcpy(bf.getData(), fsi, sizeof(*fsi));
		bm->unfixPage(bf, true);
	}
}

// _____________________________________________________________________________
SPSegment::~SPSegment()
{
	delete fsi;
}

// _____________________________________________________________________________
void SPSegment::notifySegGrowth(Extent e)
{
	return;
}

