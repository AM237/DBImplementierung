
///////////////////////////////////////////////////////////////////////////////
// SegmentFSI.h
///////////////////////////////////////////////////////////////////////////////


#ifndef SEGMENTFSI_H
#define SEGMENTFSI_H

#include "../BufferManager/BufferManager.h"
#include "Segment.h"
#include "SMConst.h"
#include <gtest/gtest.h>
#include <stdint.h>


// An entry in the SegmentFSI, marking 16 discrete fullness values for a pair
// of pages.
struct FreeSpaceEntry {
   unsigned int page1 : 4;
   unsigned int page2 : 4;
};


// The free space management entity for specialized segments (inherit from 
// RegularSegment). Represents a set of pages within a given segment which
// encode the fullness degree of each of the segment's pages. The FSI always
// starts on the first page of the segment. It is assumed that the segment's
// baseExtentSize (number of pages in its default extent) is chosen so that
// the initial FSI fits on the first page of the segment.
//
// The FSI encodes the following information:
// | FSI size | Extents size | Inventory Size | Extents | Inventory |
//
// It is assumed that the FSI extent entries and all size markers fit on the 
// first page of the segment. All size markers are 8 byte integers.
// Any given inventory entry uses 4 bit pairs to encode a degree of fullness
// according to the following linear / logarithmic scale:
//
// Value -> At least N remaining bytes:
// 0 -> 0
// 1 -> 8
// 2 -> 16
// 3 -> 32
// 4 -> 64
// 5 -> 128
// 6 -> 256
// 7 -> 512
// 8 -> 1024
// 9 -> 2048
// 10 -> 3072
// 11 -> 4096
//
// A value of 16 for a page entry in a FreeSpaceEntry marks the given page
// as being used by the SegmentFSI
class SegmentFSI
{
	friend class SPSegment;
	
public:

	// Constructor. Takes the size of the segment in pages, and the page on
	// which the segment containing this FSI starts. 
	SegmentFSI(BufferManager* bm, uint64_t pages, uint64_t pageStart);
	~SegmentFSI() { }

	// Serializes this SegmentFSI object. Returns byte array and its length.
	// This object does not take responsiblity for freeing the allocated
	// memory
	std::pair<unsigned char*, uint64_t> serialize();

	// De-serializes this SegmentFSI object, re-writes data structures.
	// Assumes bytes contains the FSI header, that is, 
	// | FSI size | Extents size | Inventory Size | Extents |
	// After extracting the FSI's extents, looks up all pages on which FSI
	// is found and then deserializes the FSI completely.
	void deserialize(unsigned char* bytes);

	// Returns the runtime size of this SegmentFSI in bytes: size of 
	// extents + size of inv
	uint64_t getRuntimeSize();

	// Updates the inventory by performing a discretization of the new
	// available free space, and updating the appropriate page entry.
	// page is the relative page index of the page inside this segment.
	void update(uint64_t page, uint32_t value);

	// Returns the index of the page in this segment (assume numbering
	// from 0 to n) that can accomdate #requiredSize bytes. Throws 
	// SM_EXC::SegmentFullException when this is true for no page, i.e.
	// the segment must be grown. Second value in pair is true iff the returned
	// page is empty.
	std::pair<uint64_t, bool> getPage(unsigned requiredSize);

	// Adds #numPages page makers to the inventory. Starts with last entry
	// in current inventory iff useLast == true
	void grow(Extent e, bool useLast);

	// Instructs the FSI to look for an empty page in the segment, and to use
	// this page as part of the FSI. #surplus dictates whether the FSI currently
	// holds an extra page marker or not (see FreeSpaceEntry)
	void absorbPage(bool surplus);

private:

	// BufferManager handler
	BufferManager* bm;

	// The free space mapping discretization (see above), maps integer keys 
	// (index) to integer values representing free bytes (see above). 
	// Constraint: last entry contains number of bytes in an empty page.
	std::vector<int> freeBytes;

	// Free space information for pairs of pages. Note: may contain a surplus
	// page entry if the size of the segment is uneven.
	std::vector<FreeSpaceEntry> inv;

	// The set of pages over which this SegmentFSI is spread. Assumed to fit
	// on the first page at all times.
	std::vector<Extent> extents;
};

#endif  // SEGMENTFSI_H