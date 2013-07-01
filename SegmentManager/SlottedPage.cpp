
///////////////////////////////////////////////////////////////////////////////
// SlottedPage.cpp
///////////////////////////////////////////////////////////////////////////////

#include "SlottedPage.h"

using namespace std;

// _____________________________________________________________________________
void SlottedPage::compactify() 
{ 
	return;
}

// _____________________________________________________________________________
shared_ptr<pair<uint8_t, uint32_t>> SlottedPage::insert(const Record& r,bool in) 
{ 
	auto dataSize = BM_CONS::pageSize - sizeof(SlottedPageHeader);
	auto slotSize = sizeof(SlottedPageSlot);
	auto rLength = r.getLen();

	// Header not initialized -> initialize header properties, add data
	// at the very end of the page, and a slot right after the header
	if (!in)
	{
		header.lsn = 0;
		header.slotCount = 1;
		header.firstFreeSlot = 1;
		header.dataStart = dataSize-rLength;
		header.freeSpace = dataSize-slotSize-rLength;

		SlottedPageSlot s = { header.dataStart, rLength };

		// Write info
		auto it = (unsigned char*)memcpy(data, &s, slotSize);
		it += header.dataStart;
		memcpy(it, r.getData(), rLength);
		auto returnPair = shared_ptr<pair<uint8_t, uint32_t>>
			 (new pair<uint8_t, uint32_t>(0, header.freeSpace));
		return returnPair;	
	}

	// Otherwise perform a standard insert procedure:
	// 1. See if there is an available free slot with enough length to store r
	// 2. If no, check if the page can still fit the record plus a new slot
	// 3. If no, return nullptr to signal caller that insert was unsuccessful.
	else
	{
		// Case 1: first free slot is an index to an existing slot
		if (header.firstFreeSlot < header.slotCount)
		{
			auto slots = reinterpret_cast<SlottedPageSlot*>(data);
			auto slot = slots[header.firstFreeSlot];

			// The length of the record must be at most as large as the
			// piece of memory pointed to by the slot.
			if (rLength <= slot.length)
			{
				auto freed = slot.length - rLength;
				slot.length = rLength;
				
				//header.firstFreeSlot = ?
				header.freeSpace += freed; 

				memcpy(data+slot.offset, r.getData(), slot.length);
				return shared_ptr<pair<uint8_t,uint32_t>>
					   (new pair<uint8_t,uint32_t>(header.firstFreeSlot, 
					   	                           header.freeSpace));
			}
		}

		// Case 2: Check if record can be added normally (update dataStart)
		else if (header.freeSpace >= rLength + slotSize)
		{
			// compactification may be required
			if ((header.dataStart-slotSize*header.slotCount)<(rLength+slotSize))
				this->compactify();

			header.slotCount++;
			header.dataStart = header.dataStart-rLength;
			header.freeSpace = header.freeSpace-slotSize-rLength;
			if (header.firstFreeSlot==header.slotCount) header.firstFreeSlot++;

			SlottedPageSlot s = { header.dataStart, rLength };

			// Write info
			memcpy(data+(header.slotCount-1)*slotSize, &s, slotSize);
			memcpy(data+header.dataStart, r.getData(), rLength);
			auto returnPair = shared_ptr<pair<uint8_t, uint32_t>>
			 	(new pair<uint8_t, uint32_t>(header.slotCount-1, 
			 		                         header.freeSpace));
			return returnPair;	
		}

		// Case 3
		else return nullptr;
	}

	return nullptr;
}