
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
	fsi = new SegmentFSI(bm, this->getSize(), this->firstPage());

	// Initialize this segment's internal FSI directly from file.
	if (this->recovered)
	{
		// Read FSI size and extents. Assumption: size field and extents
		// all fit / can be found on the first page of the segment.
		BufferFrame& bf = bm->fixPage(this->firstPage(), false);
		auto header = reinterpret_cast<unsigned char*>(bf.getData());
		fsi->deserialize(header);
		bm->unfixPage(bf, false);
	}

	// If segment is being created for the first time, create a new FSI
	// and materialize it to file. It is assumed that the FSI will fit on the
	// first page in this case.
	else
	{
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
TID SPSegment::insert(const Record& r)
{
	// Look up a page in this segment's FSI that is guaranteed to be able to 
	// store r. The length of the record is given in bytes.
	//
	// First, dynamically determine what the required space identifier must be.
	// The required space identifier is the index in the discretized free space
	// mapping, so that its mapped value (guaranteed free space) is the smallest
	// mapped value that is still greater than or equal to the required space.
	auto recordLength = r.getLen();
	int spaceIndex = -1;
	for (size_t i = 0; i < fsi->freeBytes.size(); i++)
		if (fsi->freeBytes[i] >= (int)recordLength) { spaceIndex = i; break; }
	if (spaceIndex == -1) { SM_EXC::RecordLengthException e; throw e; }

	// Next, look through the FSI for a page with this discretized value.
	// Prefer fuller pages, and initialized pages over non initialized pages.
	uint64_t insertPage = 0;
	while (insertPage != 0)
	{
		for (size_t i = 0; i < fsi->inv.size(); i++)
		{
			if (fsi->inv[i].page1 == spaceIndex) { insertPage = 2*i; break; }
			if (fsi->inv[i].page2 == spaceIndex) { insertPage = 2*i+1; break; }
		}

		// If no page with exactly the required free space is available,
		// search for a page with the next order of available free space
		spaceIndex++;	
	}

	// No page with required free space found -> segment must be grown.
	// Throw exception to signal layer above that this segment must be grown.
	if (insertPage == 0) { SM_EXC::SPSegmentFullException e; throw e; }

	// Now that one of this segment's pages has been chosen for the insert,
	// load the page, and if it has not yet been initialized, add an SP header.
	// Otherwise, it has already been initialized, so update the header, the
	// first free slot, and the data pointer.
	bool pageInitialized = true;
	if (spaceIndex == (int)fsi->freeBytes.size()-1) pageInitialized = false;
	uint64_t pageIterator = insertPage;

	BufferFrame& bf = bm->fixPage(this->nextPage(pageIterator), true);
	SlottedPage* slottedPage = reinterpret_cast<SlottedPage*>(bf.getData());

	// Initialize page header to reflect state after insertion of record
	SlottedPageHeader& header = slottedPage->getHeader();
	if (!pageInitialized)
	{
		header.lsn = 0;
		header.slotCount = 1;
		header.firstFreeSlot = 1;
		header.dataStart = BM_CONS::pageSize - recordLength;
		header.freeSpace = BM_CONS::pageSize - sizeof(SlottedPageHeader) - 
		                   sizeof(SlottedPageSlot);
	}
	
	bm->unfixPage(bf, true);




	




	TID t = { 0, 0};
	return t;
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
	uint64_t newEntryStart = e.start;
	if (this->getSize() % 2 != 0)
	{
		fsi->inv.back().page2 = 12;
		newEntryStart++;
	}

	for (uint64_t i = newEntryStart; i < e.end; i=i+2)
	{
		FreeSpaceEntry f = {12, 12};
		fsi->inv.push_back(f);
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
		uint64_t bytesToWrite = min(remainingBytes,(uint64_t)BM_CONS::pageSize);
		BufferFrame& bf = bm->fixPage(pages[i], true);
		memcpy(bf.getData(), it, bytesToWrite);
		bm->unfixPage(bf, true);

		it += bytesToWrite; 
		remainingBytes -= bytesToWrite;
		if (remainingBytes == 0) break;
	}
	delete[] serialized.first;
}

