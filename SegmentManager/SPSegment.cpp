
///////////////////////////////////////////////////////////////////////////////
// SPSegment.cpp
///////////////////////////////////////////////////////////////////////////////

#include "SPSegment.h"
#include <algorithm>

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
		BufferFrame& bf = bm->fixPage(this->firstPage(), false);
		auto header = reinterpret_cast<unsigned char*>(bf.getData());
		fsi = new SegmentFSI(bm, this->getSize(), this->firstPage());
		fsi->deserialize(header);
		bm->unfixPage(bf, false);
	}

	// If segment is being created for the first time, create a new FSI
	// and materialize it to file. It is assumed that the FSI will fit on the
	// first page in this case.
	else
	{
		fsi = new SegmentFSI(bm, this->getSize(), this->firstPage());
		auto serialized = fsi->serialize();
		BufferFrame& bf = bm->fixPage(this->firstPage(), true);
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
		fsi->inv.back().page2 = 12;
		e.start = e.start + 1;
	}

	for (uint64_t i = e.start; i < e.end; i=i+2)
	{
		FreeSpaceEntry e = {12, 12};
		fsi->inv.push_back(e);
	}

	// Materialize changes
	//
	// Calculate available space given in extents. If this is not enough,
	// look for an empty page, mark it as being used by the FSI, and add
	// it to the FSI's extents. Then, materialize FSI across its extents.
	vector<uint64_t> pages;
	for (Extent& e : fsi->extents) 
		for (unsigned int i = e.start; i < e.end; i++) pages.push_back(i);
	sort(pages.begin(), pages.end());
	uint64_t availableSpace = pages.size() * BM_CONS::pageSize;
	uint64_t requiredSpace = fsi->getRuntimeSize();

	
	// Must look through the FSI for an empty page to add to the FSI's extents
	while (requiredSpace > availableSpace)
	{
		uint64_t segmentSize = this->getSize();
		uint64_t newFSIPageIndex = 0;
		for (size_t i = 0; i < fsi->inv.size(); i++)
		{
			// First page of entry, must not be the first page of the segment.
			if (i != 0 && fsi->inv[i].page1 == 12)
			{
				newFSIPageIndex = 2*i;
				fsi->inv[i].page1 = 0;
				break;
			}

			// Second page of entry, if last entry in inventory, only valid
			// if the segment has an even number of pages. Otherwise
			// the last entry of the inventory has a surplus page marker.
			else if (!(i == fsi->inv.size()-1 && segmentSize % 2 != 0) &&
				     fsi->inv[i].page2 == 12)
			{
				newFSIPageIndex = 2*i + 1;
				fsi->inv[i].page2 = 0;
				break;
			}
		}

		// Add empty page found to FSI's extents.
		if (newFSIPageIndex == 0) { SM_EXC::FsiOverflowException e; throw e; }
		bool pageAbsorbed = false;

		// Check if page can be integrated into an existing extent, otherwise
		// create new extent
		for (Extent& e : fsi->extents)
		{
			if (e.start == newFSIPageIndex+1) e.start = newFSIPageIndex;
			else if (e.end == newFSIPageIndex) e.end = newFSIPageIndex+1;
			pageAbsorbed = true;
			break;
		}
		if (!pageAbsorbed)
		{
			Extent e = { newFSIPageIndex, newFSIPageIndex+1};
			fsi->extents.push_back(e);
		}

		// Update loop condition
		pages.clear();
		for (Extent& e : fsi->extents) 
			for (unsigned int i = e.start; i < e.end; i++) pages.push_back(i);
		sort(pages.begin(), pages.end());
		availableSpace = pages.size() * BM_CONS::pageSize;
		requiredSpace = fsi->getRuntimeSize();
	}

	// Extents now have enough space to hold the serialized FSI
	auto serialized = fsi->serialize();
	auto it = serialized.first;
	uint64_t remainingBytes = serialized.second;
	for (size_t i = 0; i < pages.size(); i++)
	{
		uint64_t bytesToWrite = min(remainingBytes, (uint64_t)BM_CONS::pageSize);
		BufferFrame& bf = bm->fixPage(pages[i], true);
		memcpy(bf.getData(), it, bytesToWrite);
		bm->unfixPage(bf, true);

		it += bytesToWrite; 
		remainingBytes -= bytesToWrite;
		if (remainingBytes == 0) break;
	}
	delete[] serialized.first;
}

