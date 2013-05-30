
///////////////////////////////////////////////////////////////////////////////
// RegularSegment.h
//////////////////////////////////////////////////////////////////////////////


#ifndef REGULARSEGMENT_H
#define REGULARSEGMENT_H

#include "Segment.h"
#include <map>


// A regular segment (public, permanent) in the database
class RegularSegment : public Segment
{

public:

	// Constructor/destructor
	RegularSegment(bool visible, uint64_t id); 
		     	  
	~RegularSegment() { }	

	// override
	uint64_t nextPage();
	
private:


};

#endif  // REGULARSEGMENT_H
