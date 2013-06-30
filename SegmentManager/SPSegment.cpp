
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
	// Query this segment's FSI for the index of a page in this segment
	// (assume pages are numbered from 0 to n) which can accomodate r.
	// Throws exception if necessary to signal layer above that this 
	// segment must be grown, or if required size is too large for any page.
	//
	// Start with best case scenario, where only the length of the record is
	// relevant. In this case, try to find an free and compatible slot in the
	// page. If this is not possible, check if the page has enough space for
	// the record and a new slot. If this is again not possible, then begin
	// a new search, this time for r.getLen() + sizeof(slot) bytes.
	auto insertPage = fsi->getPage(r.getLen());
	bool pageInitialized = insertPage.second;

	// Now that one of this segment's pages has been chosen for the insert,
	// load the page, and if it has not yet been initialized, add an SP header.
	// Otherwise, it has already been initialized, so update the header, the
	// first free slot, and the data pointer.
	//
	// If the page has not been initialized, then by definition it contains no
	// free slots -> check that record + 1x slot fit in pageSize-sizeof(header)
	auto dataSize = BM_CONS::pageSize-sizeof(SlottedPageHeader);
	if (!pageInitialized && (r.getLen() + sizeof(SlottedPageSlot) > dataSize))
		{ SM_EXC::RecordLengthException e; throw e;}
	
	// Otherwise, page has been initialized and was chosen now already
	// considering the space taken up by the header. Therefore, proceed as above
	uint64_t fixedPage = this->nextPage(insertPage.first);
	BufferFrame& bf = bm->fixPage(fixedPage, true);
	SlottedPage* slottedPage = reinterpret_cast<SlottedPage*>(bf.getData());
	auto insertResult = slottedPage->insert(r,pageInitialized);
	// if (insertResult == nullptr) ...
	bm->unfixPage(bf, true);

	// Update the page in the FSI in which the record was inserted.
	fsi->update(insertPage.first, insertResult->second);
	TID returnTID = { fixedPage, insertResult->first };
	return returnTID;
}

// _____________________________________________________________________________
void SPSegment::notifySegGrowth(Extent e)
{
	// Add entries to the FSI for as many pages as exist in the given extent,
	// and materialize the changes.
	bool useLast = this->getSize() % 2 == 0? false : true;
	fsi->grow(e, useLast);

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
		bool surplus = this->getSize() % 2 != 0 ? true : false;
		fsi->absorbPage(surplus);

		// Update loop condition
		pages.clear();
		for (Extent& e : fsi->extents) 
			for (unsigned int i = e.start; i < e.end; i++) pages.push_back(i);
		sort(pages.begin(), pages.end());
		availableSpace = pages.size() * BM_CONS::pageSize;
		requiredSpace = fsi->getRuntimeSize();
	}

	// Extents now have enough space to hold the serialized FSI,
	// pages vector is sorted.
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

