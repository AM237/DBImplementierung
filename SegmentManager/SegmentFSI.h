
///////////////////////////////////////////////////////////////////////////////
// SegmentFSI.h
///////////////////////////////////////////////////////////////////////////////


#ifndef SEGMENTFSI_H
#define SEGMENTFSI_H

#include "Segment.h"
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
// | FSI extents | size in bytes | page1 fullness | ... | pageN fullness |
//
// It is assumed that the FSI extent entries and 'size in bytes' all fit on the 
// first page of the segment. The 'size in bytes' field is an 8 byte integer.
// Any given 'pageN fullness' entry uses 4 bits to encode its degree of fullness
// according to the following linear / logarithmic scale:
//
// Value -> At least N remaining bytes:
// 0 -> 8
// 1 -> 16
// 2 -> 32
// 3 -> 64
// 4 -> 128
// 5 -> 256
// 6 -> 512
// 7 -> 1024
// 8 -> 2048
// 9 -> 3072
// 10 -> 4096 - headerSize
class SegmentFSI
{
public:

	// Constructor. Takes the size of the segment in pages, and the page on
	// which the segment containing this FSI starts. Initializes the
	// FSI to contain an entry per page.
	SegmentFSI(uint64_t pages, uint64_t pageStart);
	~SegmentFSI() { }

private:

	// The size of this SegmentFSI object in bytes. This is necessary to know
	// how much byte data needs to be read from this SegmentFSI's extents.
	uint64_t size;

	// The set of pages over which this SegmentFSI is spread.
	std::vector<Extent> extents;

	// Free space information for pairs of pages.
	std::vector<FreeSpaceEntry> inventory;
};

#endif  // SEGMENTFSI_H