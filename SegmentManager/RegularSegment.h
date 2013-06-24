
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

	// Constructor/destructor.
	FRIEND_TEST(SegmentManagerTest, initializeWithFile);
	RegularSegment(bool visible, uint64_t id, Extent* base = NULL, 
				   bool recovered = false) : Segment(true, visible, id, base)
	{ 
		this->recovered = recovered;
	}
	      	  
	~RegularSegment() { }

	// Notifies this RegularSegment that the SM has added an extent.
	// This is necessary, since specializations of this class might
	// need to make memory management arrangements based on the free
	// space in the segment.
	virtual void notifySegGrowth(Extent e) {  }
	
protected:

	// true iff this segment was recovered directly from file (see 
	// SegmentInventory::initializeFromFile) instead of being created for the
	// first time via the SM interface.
	bool recovered;

};

#endif  // REGULARSEGMENT_H
