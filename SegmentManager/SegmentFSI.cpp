
///////////////////////////////////////////////////////////////////////////////
// SegmentFSI.cpp
///////////////////////////////////////////////////////////////////////////////


#include "SegmentFSI.h"
#include <assert.h> 
#include <algorithm>
#include <math.h> 
#include <memory>

using namespace std;

// _____________________________________________________________________________
SegmentFSI::SegmentFSI(BufferManager* bm, uint64_t numPages, uint64_t pageStart)
{ 
	
	// Initialize free space mapping
	freeBytes = {0,8,16,32,64,128,256,512,1024,2048,3072,4096};

	// Initialize inv (marking empty pages). Mark first page as belonging
	// to the FSI (value = 15)
	unsigned size = (unsigned)freeBytes.size();
	for (int i = 0; i < ceil(numPages/2); i++)
	{
		FreeSpaceEntry e;
		if (i == 0) e = {15, size};
		else 	    e = {size, size};
		inv.push_back(e);
	}

	// Initialize extents
	Extent e = {pageStart, pageStart+1};
	extents.push_back(e);
	this->bm = bm;
}


// _____________________________________________________________________________
uint64_t SegmentFSI::getRuntimeSize()
{ 
	return 3*sizeof(uint64_t)+ extents.size()*sizeof(Extent) + 
	       inv.size()*sizeof(FreeSpaceEntry);
}


// _____________________________________________________________________________
void SegmentFSI::update(uint64_t page, uint32_t value)
{
	auto it = upper_bound(freeBytes.begin(), freeBytes.end(), value);
	unsigned int discretizedValue = freeBytes[it - freeBytes.begin() - 1];

	if (page % 2 == 0) inv[page/2].page1 = discretizedValue;
	else 			   inv[page/2].page2 = discretizedValue;
}


// _____________________________________________________________________________
pair<uint64_t, bool> SegmentFSI::getPage(unsigned requiredSize)
{
	// Get the index in the discretized free space
	// mapping, so that its mapped value (guaranteed free space) is the smallest
	// mapped value that is still greater than or equal to the required space.
	// Prefers fuller pages, and non empty pages to empty pages.
	bool empty = false;
	int spaceIndex = -1;
	for (size_t i = 0; i < freeBytes.size(); i++)
		if (freeBytes[i] >= (int)requiredSize) { spaceIndex = i; break; } 
	
	// If requiredSize is larger than any empty space -> throw exception
	if (spaceIndex == -1) { SM_EXC::InputLengthException e; throw e; }
	if (spaceIndex == (int)freeBytes.size()-1) empty = true;

	// Find a page with the closest fullness degree to the above value.
	uint64_t page = 0;
	while (page != 0)
	{
		for (size_t i = 0; i < inv.size(); i++)
		{
			if (inv[i].page1 == spaceIndex) { page = 2*i; break; }
			if (inv[i].page2 == spaceIndex) { page = 2*i+1; break; }
		}

		// If no page with exactly the required free space is available,
		// search for a page with the next order of available free space
		if (++spaceIndex >= (int)freeBytes.size()) break;
	}

	if (page == 0) { SM_EXC::SPSegmentFullException e; throw e; }
	return pair<uint64_t, bool>(page, empty);
}


// _____________________________________________________________________________
void SegmentFSI::grow(Extent e, bool useLast)
{
	// The inventory has a surplus marker iff useLast == true
	uint64_t newEntryStart = e.start;
	unsigned int size = freeBytes.size();
	if (useLast)
	{
		inv.back().page2 = size;
		newEntryStart++;
	}

	for (uint64_t i = newEntryStart; i < e.end; i=i+2)
	{
		FreeSpaceEntry f = { size, size };
		inv.push_back(f);
	}
}

// _____________________________________________________________________________
void SegmentFSI::absorbPage(bool surplus)
{
	uint64_t newFSIPageIndex = 0;
	for (size_t i = 0; i < inv.size(); i++)
	{
		// First page of entry, must not be the first page of the segment.
		if (i != 0 && inv[i].page1 == freeBytes.size())
		{
			newFSIPageIndex = 2*i;
			inv[i].page1 = freeBytes.size();
			break;
		}
		// Second page of entry, if last entry in inventory, only valid
		// if the segment has an even number of pages. Otherwise
		// the last entry of the inventory has a surplus page marker.
		else if (!(i == inv.size()-1 && !surplus) && 
			       inv[i].page2 == freeBytes.size())
		{
			newFSIPageIndex = 2*i + 1;
			inv[i].page2 = freeBytes.size();
			break;
		}
	}

	// Add empty page found to FSI's extents.
	if (newFSIPageIndex == 0) { SM_EXC::FsiOverflowException e; throw e; }
	bool pageAbsorbed = false;

	// Check if page can be integrated into an existing extent, otherwise
	// create new extent
	for (Extent& e : this->extents)
	{
		if (e.start == newFSIPageIndex+1) e.start = newFSIPageIndex;
		else if (e.end == newFSIPageIndex) e.end = newFSIPageIndex+1;
		pageAbsorbed = true;
		break;
	}
	if (!pageAbsorbed)
	{
		Extent e = { newFSIPageIndex, newFSIPageIndex+1};
		this->extents.push_back(e);
	}
}

	
// _____________________________________________________________________________
pair<unsigned char*, uint64_t> SegmentFSI::serialize()
{ 
	// FSI format:
	// | FSI size | Extents size | Inventory Size | Extents | Inventory |
	uint64_t fsiSize = getRuntimeSize();
	uint64_t extentsSize = extents.size();
	uint64_t inventorySize = inv.size();

	unsigned char* fsibytes = new unsigned char[fsiSize];

	// serialize components
	auto exArray = reinterpret_cast<unsigned char*>(extents.data());
	auto inArray = reinterpret_cast<unsigned char*>(inv.data());
	
	// write to output array
	fsibytes[0] = fsiSize;
	fsibytes[1] = extentsSize;
	fsibytes[2] = inventorySize;
	auto it = fsibytes + 3;
	memcpy(it, exArray, extentsSize * sizeof(Extent));
	it += extentsSize * sizeof(Extent);
	memcpy(it, inArray, inventorySize * sizeof(FreeSpaceEntry));

	return pair<unsigned char*, uint64_t>(fsibytes, fsiSize);
}

// _____________________________________________________________________________
void SegmentFSI::deserialize(unsigned char* bytes)
{ 
	// Reset data structures
	extents.clear();
	inv.clear();

	// Get the extents / pages on which the FSI is found. Assumed to always
	// be found exclusively on the first page of the segment.
	auto header = reinterpret_cast<uint64_t*>(bytes);
	uint64_t fsiSize = header[0];
	uint64_t extentsSize = header[1];
	uint64_t inventorySize = header[2];

	vector<unsigned char> extentBytes(bytes+3*sizeof(uint64_t), 
		                              bytes+3*sizeof(uint64_t) + 
		                              extentsSize * sizeof(Extent));
	auto deserializedExtents = reinterpret_cast<Extent*>(extentBytes.data());
	vector<uint64_t> pages;
	for (unsigned int i = 0; i < extentsSize; i++)
	{
		Extent e = deserializedExtents[i];
		this->extents.push_back(e);
		for (auto j = e.start; j < e.end; j++) pages.push_back(j);
	}

	// Get byte array from all pages on which FSI is found.
	unsigned char* fsibytes = new unsigned char[pages.size()*BM_CONS::pageSize];
	unsigned char* it = fsibytes;
	for (uint64_t& page : pages)
	{
		BufferFrame& bf = bm->fixPage(page, false);
		memcpy(it, bf.getData(), BM_CONS::pageSize);
		bm->unfixPage(bf, false);
		it = it + BM_CONS::pageSize;
	}

	// take only inventory body bytes, deserialize completely
	vector<unsigned char> invVec(fsibytes+3*sizeof(uint64_t) +
		                         extentsSize*sizeof(Extent), fsibytes+fsiSize);
	auto deserializedInv = reinterpret_cast<FreeSpaceEntry*>(invVec.data());
	for (unsigned int i = 0; i < inventorySize; i++)
	{
		FreeSpaceEntry e = deserializedInv[i];
		this->inv.push_back(e);
	}
}