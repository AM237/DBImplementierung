
///////////////////////////////////////////////////////////////////////////////
// SPSegment.h
//////////////////////////////////////////////////////////////////////////////


#ifndef SPSEGMENT_H
#define SPSEGMENT_H

#include "../BufferManager/BufferManager.h"
#include "RegularSegment.h"
#include "SegmentFSI.h"
#include "Record.h"
#include "TID.h"

// A segment based on slotted pages.
class SPSegment : public RegularSegment
{
public:

	// Constructor. Initializes the segment's FSI.
	//
	// The FSI can be initialized in two different ways. If the respective
	// segment is created for the first time, then the FSI must be initialized
	// to signal that all of the segment's pages are empty. Otherwise, the
	// segment has been recovered from file (see SegmentInventory::
	// initializeFromFile). In this case the FSI must be recovered from file,
	// starting from the first page of the segment.
	SPSegment(BufferManager* bm, bool visible, uint64_t id, Extent* base =NULL);
	~SPSegment();

	// Searches through the segment's pages looking for a page with enough 
	// space to store r. Returns nullptr iff there is no space, i.e. segment 
	// must be grown. Otherwise returns the TID identifying the location 
	// where r was stored. This is implemented effciently using the seg's FSI.
	TID* insert(const Record& r);

	// Deletes the record pointed to by tid and updates the 
	// page header accordingly
	bool remove(TID tid);

	// Returns a pointer or reference to the read-only record 
	// associated with TID tid.
	Record* lookup(TID tid);

	//Updates the record pointed to by tid with the content of record r.
	bool update(TID tid, const Record& r);

	// Override
	void notifySegGrowth(Extent e);
		
private:

	// Deserializes file contents and fills an SPSegment object with the
	// respective data. Typical use case: An SPSegment is returned by the SM
	// via the retrieveSegmentById interface (knowledge that the returned 
	// segment is actually an SPSegment is given by the metadata layer). 
	// This already specifies which extents belong to this segment.
	// Then the initialize method is called to update the objects internal data
	// from the information encoded in these extents.
	void initializeFromFile();

	// The free space inventory for this segment.
	SegmentFSI* fsi;

	// Reference to the Buffer Manager. This class does not take responsibility
	// for the destruction of this pointer.
	BufferManager* bm;
};


#endif  // SPSEGMENT_H
