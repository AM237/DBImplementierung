
///////////////////////////////////////////////////////////////////////////////
// SegmentFSI.cpp
///////////////////////////////////////////////////////////////////////////////


#include "SegmentFSI.h"
#include "SlottedPage.h"
#include <assert.h> 
#include <math.h> 

using namespace std;

// _____________________________________________________________________________
SegmentFSI::SegmentFSI(BufferManager* bm, uint64_t numPages, uint64_t pageStart)
{ 
	// Initialize inv (marking empty pages). Mark first page as belonging
	// to the FSI (value = 0)
	this->bm = bm;
	for (int i = 0; i < ceil(numPages/2); i++)
	{
		FreeSpaceEntry e;
		if (i == 0) e = {0 , 12};
		else 	    e = {12, 12};
		inv.push_back(e);
	}

	// Initialize free space mapping
	int x = sizeof(SlottedPageHeader);
	assert(4096 - x > 3072);
	freeBytes = {0,8,16,32,64,128,256,512,1024,2048,3072,4096-x,4096};

	// Initialize extents
	Extent e = {pageStart, pageStart+1};
	extents.push_back(e);
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

// _____________________________________________________________________________
uint64_t SegmentFSI::getRuntimeSize()
{ 
	return 3*sizeof(uint64_t)+ extents.size()*sizeof(Extent) + 
	       inv.size()*sizeof(FreeSpaceEntry);
}


