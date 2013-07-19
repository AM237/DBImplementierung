
///////////////////////////////////////////////////////////////////////////////
// SMConst.h
//////////////////////////////////////////////////////////////////////////////

#ifndef SMCONST_H
#define SMCONST_H

namespace SM_EXC
{
	struct FsiOverflowException: public std::exception
	{
  		virtual const char* what() const throw()
  		{ return "No new page can be assigned to this segment's FSI"; }
	};

	struct InputLengthException: public std::exception
	{
  		virtual const char* what() const throw()
  		{ return "Input is too large to fit on any page"; }
	};

	struct RecordLengthException: public std::exception
	{
  		virtual const char* what() const throw()
  		{ return "Record is too large to fit on any page with a header"; }
	};

	struct SPSegmentFullException: public std::exception
	{
  		virtual const char* what() const throw()
  		{ return "SPSegment is full -> grow segment using SM"; }
	};
}

struct SMConst
{
	// DB Size in pages
	static const uint64_t dbSize = 50000;

	// Size of the buffer manager
	static const uint64_t bufferSize = 1000;
	
	// Required for dynamic extent mapping, this is the exponential factor
	// by which the size of extents increases every time a segment is grown.
	static constexpr float extentIncrease = 1.2f;
	
	// The size of a base extent
	static const uint64_t baseExtentSize = 30;
};


#endif  // SMCONST_H
