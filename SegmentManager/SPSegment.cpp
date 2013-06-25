
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
		// Read FSI size and extents. Assumption: size field and extents
		// all fit / can be found on the first page of the segment.
		vector<Extent> fsiExtents;
		BufferFrame& bf = bm->fixPage(this->firstPage(), false);
		auto header = reinterpret_cast<SegmentFSI*>(bf.getData());
		//uint64_t fsiSize = header->size;
		uint64_t fsiSize = sizeof(*fsi);
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
	// and materialize it to file. It is assumed that the FSI will fit on the
	// first page in this case.
	else
	{
		fsi = new SegmentFSI(bm, this->getSize(), this->firstPage());
		auto serialized = fsi->serialize();
		BufferFrame& bf = bm->fixPage(this->firstPage(), true);
		//memcpy(bf.getData(), fsi, sizeof(*fsi));
		memcpy(bf.getData(), serialized.first, serialized.second);
		bm->unfixPage(bf, true);
		delete[] serialized.first;
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
	// Add entries to the FSI for as many pages as exist in the given extent,
	// and materialize the changes.
	//
	// If current number of pages in the
	// extent is uneven, then the inventory contains a surplus marker
	// (see constructor)
	if (this->getSize() % 2 != 0)
	{
		fsi->inv.back().page2 = 11;
		e.start = e.start + 1;
	}

	for (uint64_t i = e.start; i < e.end; i=i+2)
	{
		FreeSpaceEntry e = {11, 11};
		fsi->inv.push_back(e);
	}

	// Materialize changes
	//
	// Calculate available space given in extents. If this is not enough,
	// look for an empty page, mark it as being used by the FSI, and add
	// it to the FSI's extents. Then, materialize FSI to its extents.
	//uint64_t pages = 0;
	//for (Extent& e : fsi->extents) pages += (e.end - e.start);
}

