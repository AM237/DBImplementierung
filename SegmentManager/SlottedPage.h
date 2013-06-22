
///////////////////////////////////////////////////////////////////////////////
// SlottedPage.h
//////////////////////////////////////////////////////////////////////////////


#ifndef SLOTTEDPAGE_H
#define SLOTTEDPAGE_H

#include "RegularSegment.h"
#include "SlottedPage.h"
#include "Record.h"
#include "TID.h"

// A segment based on slotted pages.
class SlottedPage
{
public:

	SlottedPage();
	~SlottedPage() { }
	
private:

	// The header of the slotted page
	void* header;

	// The slots containing start and offset ids
	std::vector<uint64_t> slots;

	// The records maintained by this slotted page.
	std::vector<Record> records;

};

#endif  // SLOTTEDPAGE_H
