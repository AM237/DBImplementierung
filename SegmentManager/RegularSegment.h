
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
	RegularSegment(bool visible, uint64_t id, Extent* base = NULL);
	//RegularSegment(bool visible, uint64_t id);
	      	  
	~RegularSegment() { }	
	
private:


};

#endif  // REGULARSEGMENT_H
