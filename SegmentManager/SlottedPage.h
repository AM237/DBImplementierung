
///////////////////////////////////////////////////////////////////////////////
// SlottedPage.h
///////////////////////////////////////////////////////////////////////////////


#ifndef SLOTTEDPAGE_H
#define SLOTTEDPAGE_H

#include <stdint.h>
#include <vector>

// A slotted page header -------------------------------------------------------
struct SlottedPageHeader
{
	// recovery component, number of used slots, id of first free slot to
	// speed up locating free slots, lower end of data, and the space that would
	// be available in this slotted page after compactification (in bytes)
	uint32_t lsn, slotCount, firstFreeSlot, dataStart, freeSpace;
};


// A slotted page slot ---------------------------------------------------------
struct SlottedPageSlot
{
	// offset and length of corresponding data item
	uint32_t offset, length;
};


// A slotted page --------------------------------------------------------------
class SlottedPage
{
public:

	// Constructor/destructor
	SlottedPage();
	~SlottedPage() { }

	// Presses data blocks together to make space for more incoming data.
	// Updates header and the corresponding slots.
	void compactify();

	
private:

	// The header of the slotted page
	SlottedPageHeader header;

	// The slots containing start and offset ids
	std::vector<SlottedPageSlot> slots;

	// The records maintained by this slotted page.
	void* data;
};

#endif  // SLOTTEDPAGE_H
