
///////////////////////////////////////////////////////////////////////////////
// RegularSegment.h
//////////////////////////////////////////////////////////////////////////////


#ifndef REGULARSEGMENT_H
#define REGULARSEGMENT_H

#include "Segment.h"


// A regular segment (public, permanent) in the database. Segments of this 
// type do not define any layout of the managed pages, i.e. they provide
// "formatless" memory (in page sized byte arrays).
class RegularSegment : public Segment
{
public:

	// Constructor/destructor
	FRIEND_TEST(SegmentManagerTest, initializeWithFile);
	RegularSegment(bool visible, uint64_t id, Extent* base = NULL);
	      	  
	~RegularSegment() { }	
	
private:


};

#endif  // REGULARSEGMENT_H
