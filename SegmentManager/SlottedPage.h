
///////////////////////////////////////////////////////////////////////////////
// SlottedPage.h
///////////////////////////////////////////////////////////////////////////////


#ifndef SLOTTEDPAGE_H
#define SLOTTEDPAGE_H


#include <stdint.h>
#include <vector>
#include <memory>
#include "Record.h"
#include "../BufferManager/BMConst.h"


// A slotted page header -------------------------------------------------------
struct SlottedPageHeader
{
	// recovery component, number of used slots, id of first free slot to
	// speed up locating free slots, lower end of data, and the space that would
	// be available in this slotted page after compactification (in bytes)
	// Latter 3 refer to offsets wrt data pointer.
	uint16_t lsn, slotCount, firstFreeSlot, dataStart, freeSpace;
};


// A slotted page slot ---------------------------------------------------------
struct SlottedPageSlot
{
	// offset and length of corresponding data item. Reserve a bit to tell
	// whether this slot is free or not.
	unsigned int empty: 1;
	unsigned int offset: 15;
	unsigned int length: 16;
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

	// Creates a header for this page, and inserts r along with its slot
	// as the first entry in an empty page. This method does not check whether
	// space requirements are fulfilled. Returns the respective slot id and
	// the new exact free space available.
	//
	// If no free slot big enough is found, and inserting the record plus a
	// new slot is not possible, returns nullptr.
	std::shared_ptr<std::pair<uint8_t,uint32_t>>insert(const Record& r,bool in);

	// Mark the given slot as empty, update firstFreeSlot in header.
	// Returns true iff slot is valid.
	bool remove(uint8_t slotId);

	// Returns the record under the given slot. Returns nullptr iff slot invalid
	std::shared_ptr<Record> lookup(uint8_t slotId);

	// Updates the record at the given slot with r. Constraint: r is at most
	// as large as the current record at the given slot. Return true iff 
	// update successful.
	bool update(uint8_t slotId, const Record& r);

	// Rearranges/ Presses data blocks together to make space for more incoming 
	// data. Updates header and the corresponding slots (cleans out slots
	// so that only non empty slots are stored)
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
