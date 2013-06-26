
///////////////////////////////////////////////////////////////////////////////
// SegmentFSI.h
///////////////////////////////////////////////////////////////////////////////


#ifndef SEGMENTFSI_H
#define SEGMENTFSI_H

#include "../BufferManager/BufferManager.h"
#include "Segment.h"
#include <gtest/gtest.h>
#include <stdint.h>

namespace SM_EXC
{
	struct FsiOverflowException: public std::exception
	{
  		virtual const char* what() const throw()
  		{ return "No new page can be assigned to this segment's FSI"; }
	};
}

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
// 11 -> 4096 - headerSize
// 12 -> 4096 (uninitialized)
//
// A value of 0 for a page entry in a FreeSpaceEntry marks the given page
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

	// Returns the runtime size of this SegmentFSI in bytes.
	uint64_t getRuntimeSize();

private:

	// BufferManager handler
	BufferManager* bm;

	// The set of pages over which this SegmentFSI is spread. Assumed to fit
	// on the first page at all times.
	std::vector<Extent> extents;

	// Free space information for pairs of pages. Note: may contain a surplus
	// page entry if the size of the segment is uneven.
	std::vector<FreeSpaceEntry> inv;
};

#endif  // SEGMENTFSI_H