
///////////////////////////////////////////////////////////////////////////////
// SegManConst.h
//////////////////////////////////////////////////////////////////////////////

#ifndef SEGMANCONST_H
#define SEGMANCONST_H

struct SegManConst
{
	// Size of the buffer manager
	static const uint64_t bufferSize = 20;
	
	// Required for dynamic extent mapping, this is the exponential factor
	// by which the size of extents increases every time a segment is grown.
	static constexpr float extentIncrease = 1.2f;
	
	// The size of a base extent
	static const uint64_t baseExtentSize = 10;
};


#endif  // SEGMANCONST_H
