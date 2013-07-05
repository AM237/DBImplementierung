
///////////////////////////////////////////////////////////////////////////////
// SPSegment.h
//////////////////////////////////////////////////////////////////////////////


#ifndef SPSEGMENT_H
#define SPSEGMENT_H

#include "../BufferManager/BufferManager.h"
#include "RegularSegment.h"
#include "SlottedPage.h"
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
	// space to store r. Throws SPSegmentFullException iff there is no space,
	// i.e. segment must be grown. Otherwise returns the TID identifying the 
	// location where r was stored. This is implemented effciently using the 
	// segments's FSI. Throws RecordLengthException iff record too large to fit
	// on any page.
	TID insert(const Record& r);

	// Deletes the record pointed to by tid and updates the 
	// page header accordingly. Returns true iff remove was successful
	bool remove(TID tid);

	// Returns a pointer or reference to the read-only record 
	// associated with TID tid. Returns nullptr iff page / slot invalid.
	std::shared_ptr<Record> lookup(TID tid);

	// Updates the record pointed to by tid with the content of record r.
	// Constraint: r must be at most as large as the record it replaces.
	// Return true iff update successful.
	bool update(TID tid, const Record& r);

	// Override
	void notifySegGrowth(Extent e);
		
private:

	// The free space inventory for this segment.
	SegmentFSI* fsi;

	// Reference to the Buffer Manager. This class does not take responsibility
	// for the destruction of this pointer.
	BufferManager* bm;
};


#endif  // SPSEGMENT_H
