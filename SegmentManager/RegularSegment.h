
///////////////////////////////////////////////////////////////////////////////
// RegularSegment.h
//////////////////////////////////////////////////////////////////////////////


#ifndef REGULARSEGMENT_H
#define REGULARSEGMENT_H

#include "Segment.h"


// A regular segment (public, permanent) in the database
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
