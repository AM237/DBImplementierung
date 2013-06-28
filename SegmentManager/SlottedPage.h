
///////////////////////////////////////////////////////////////////////////////
// SlottedPage.h
///////////////////////////////////////////////////////////////////////////////


#ifndef SLOTTEDPAGE_H
#define SLOTTEDPAGE_H

#include <stdint.h>
#include <vector>
#include "../BufferManager/BMConst.h"

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

	// Getter methods
	SlottedPageHeader& getHeader() { return header; }
	unsigned char* getData() { return data; }

	// Presses data blocks together to make space for more incoming data.
	// Updates header and the corresponding slots.
	void compactify();

	
private:

	// The header of the slotted page
	SlottedPageHeader header;

	// The data held by this slotted page. Encodes both the slots as well as
	// the actual data. Access to slots via slotCount, and the knowledge that
	// the slots immediately follow the header on file.
	unsigned char data[BM_CONS::pageSize - sizeof(SlottedPageHeader)];
};

#endif  // SLOTTEDPAGE_H
