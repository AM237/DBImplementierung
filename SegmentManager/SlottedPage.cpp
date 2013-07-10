
///////////////////////////////////////////////////////////////////////////////
// SlottedPage.cpp
///////////////////////////////////////////////////////////////////////////////

#include "SlottedPage.h"
#include <iostream>

using namespace std;

// _____________________________________________________________________________
void SlottedPage::compactify() 
{ 
	cout << "entered compactify" << endl;
	auto dataSize = BM_CONS::pageSize - sizeof(SlottedPageHeader);
	vector<pair<uint16_t,uint16_t>> freeSpace;
	vector<SlottedPageSlot> nonFreeSlots;
	auto slots = reinterpret_cast<SlottedPageSlot*>(data);

	// Get non free slots in reverse order
	for (auto i = header.slotCount-1; i >= 0; i--)
	{
		auto slot = slots[i];
		if (slot.offset != 0 || slot.length != 0) nonFreeSlots.push_back(slot);
	}
		

	// Get free space between non empty slots, also in reverse order
	for (unsigned i = 0; i < nonFreeSlots.size(); i++)
	{
		auto slot = nonFreeSlots[i];

		// Last slot -> get space between end of data and end of page.
		if (i == 0) 
			freeSpace.push_back(pair<uint16_t,uint16_t>
				(slot.offset+slot.length, dataSize));

		else
		{
			uint16_t lastOffset = nonFreeSlots[i-1].offset;
			freeSpace.push_back(pair<uint16_t,uint16_t>
				(slot.offset+slot.length, lastOffset));
		}
	}

	// Starting at the end of file, "swap" the data item with the empty space
	// that immediately follows it, add the current item's free space to the
	// next data item's free space and repeat until all data items have been
	// processed.
	for (size_t i = 0; i < freeSpace.size(); i++)
	{
		// If there is no space between the two data blocks, move to next
		// pair of data blocks.
		auto space = freeSpace[i];
		if (space.first == space.second) continue;

		// Otherwise move preceding data block right
		auto slot = nonFreeSlots[i];
		for (auto j = 1; j <= slot.length; j++)
			data[space.second-1] = data[slot.offset+slot.length-j];

		// Update slot info
		auto spaceAmount = space.second - space.first;
		slot.offset += spaceAmount;

		// Update empty space following the next data block
		if (i+1 < freeSpace.size()) freeSpace[i+1].second += spaceAmount;	
	}

	// Now that defragmentation of the data blocks is done, update the page
	// header, and write the updated slots (in reverse order)
	
	//header.slotCount = nonFreeSlots.size();
	//header.firstFreeSlot = nonFreeSlots.size();
	header.dataStart = nonFreeSlots[0].offset;

	/*for (size_t i = 0; i < nonFreeSlots.size(); i++)
	{
		auto nonFreeSlot = nonFreeSlots[nonFreeSlots.size()-1-i]; 
		slots[i].offset = nonFreeSlot.offset;
		slots[i].length = nonFreeSlot.length;
	}*/
}



// _____________________________________________________________________________
shared_ptr<pair<uint8_t, uint32_t>> SlottedPage::insert(const Record& r,bool in) 
{ 
	/*cout << "entered slotted page insert " << endl;
	cout << "lsn: " << header.lsn << endl;
	cout << "slot count: " << header.slotCount << endl;
	cout << "firstFreeSlot: " << header.firstFreeSlot << endl;
	cout << "dataStart: " << header.dataStart << endl;
	cout << "free space: " << header.freeSpace << endl;*/

	auto dataSize = BM_CONS::pageSize - sizeof(SlottedPageHeader);
	auto slotSize = sizeof(SlottedPageSlot);
	auto rLength = r.getLen();

	// Header not initialized -> initialize header properties, add data
	// at the very end of the page, and a slot right after the header
	if (!in)
	{
		//cout << "sp page not initialized" << endl;
		header.lsn = 0;
		header.slotCount = 1;
		header.firstFreeSlot = 1;
		header.dataStart = dataSize-rLength;
		header.freeSpace = dataSize-slotSize-rLength;

		SlottedPageSlot s = { header.dataStart, rLength };

		// Write info
		memcpy(data, &s, slotSize);
		//reinterpret_cast<SlottedPageSlot*>(data)[0] = s;
		memcpy(data+header.dataStart, r.getData(), rLength);
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
		//cout << "sp page initialized" << endl;

		// Case 1: first free slot is an index to an existing slot
		auto firstFreeSlot = header.firstFreeSlot;
		if (firstFreeSlot < header.slotCount)
		{
			//cout << "using free slot " << firstFreeSlot << endl;
			auto slots = reinterpret_cast<SlottedPageSlot*>(data);
			auto slot = slots[firstFreeSlot];

			// The length of the record must be at most as large as the
			// piece of memory pointed to by the slot.
			if (rLength <= slot.length)
			{
				// update slot and header
				auto freed = slot.length - rLength;
				slot.length = rLength;
				header.freeSpace += freed;
				
				// update the next free slot
				bool nextFreeSlotFound = false;
				for (auto i = firstFreeSlot+1; i < header.slotCount; i++)
				{
					auto slot = slots[i];
					if (slot.offset == 0 && slot.length == 0) 
					{ 
						nextFreeSlotFound = true; 
						header.firstFreeSlot = i;
						break;
					}
				}
				if (!nextFreeSlotFound) header.firstFreeSlot = header.slotCount;

				memcpy(data+slot.offset, r.getData(), slot.length);
				return shared_ptr<pair<uint8_t,uint32_t>>
					   (new pair<uint8_t,uint32_t>( firstFreeSlot, 
					   	                            header.freeSpace));
			}
		}

		// Case 2: Check if record can be added normally (update dataStart)
		else if (header.freeSpace >= rLength + slotSize)
		{
			//cout << "add new slot" << endl;
			// compactification may be required
			if ((header.dataStart-slotSize*header.slotCount)<(rLength+slotSize))
				this->compactify();

			header.slotCount++;
			header.dataStart = header.dataStart-rLength;
			header.freeSpace = header.freeSpace-slotSize-rLength;
			if (header.firstFreeSlot==header.slotCount-1)header.firstFreeSlot++;

			SlottedPageSlot s = { header.dataStart, rLength };

			// Write info
			memcpy(data+(header.slotCount-1)*slotSize, &s, slotSize);
			//reinterpret_cast<SlottedPageSlot*>(data)[header.slotCount-1] = s;
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


// _____________________________________________________________________________
bool SlottedPage::remove(uint8_t slotId)
{
	 if (slotId >= header.slotCount) return false;
	 reinterpret_cast<SlottedPageSlot*>(data)[slotId].length = 0;
	 reinterpret_cast<SlottedPageSlot*>(data)[slotId].offset = 0;
	 if (header.firstFreeSlot > slotId) header.firstFreeSlot = slotId;
	 return true;
}

// _____________________________________________________________________________
shared_ptr<Record> SlottedPage::lookup(uint8_t slotId)
{
	//cout << "looking for slot " << (uint16_t)slotId << endl;
	//cout << "slot count " << header.slotCount << endl;


	 if (slotId >= header.slotCount) return nullptr;
	 auto slot = reinterpret_cast<SlottedPageSlot*>(data)[slotId];
	 if (slot.length == 0 && slot.offset == 0) return nullptr;

	 // Extract record from file
	 auto ptrData = (const char*)data+slot.offset;
	 shared_ptr<Record> recordPtr (new Record(slot.length, ptrData));
	 return recordPtr;
}

// _____________________________________________________________________________
bool SlottedPage::update(uint8_t slotId, const Record& r)
{
	if (slotId >= header.slotCount) return false;

	auto slot = reinterpret_cast<SlottedPageSlot*>(data)[slotId];
	if (slot.length == 0 && slot.offset == 0) return false;

	// Check difference between the allotted slot length and the size of the
	// record. If it fits, then insert directly, otherwise check space right
	// before dataStart, if it fits there, change the offset, otherwise
	// compactify and check again. If still not possible, return false.
	auto spaceDiff = slot.length - r.getLen();

	// Case 1
	if (spaceDiff >= 0)
	{
		// Update header and record on file
		header.freeSpace += spaceDiff;
		slot.length = r.getLen();
		memcpy(data + slot.offset, r.getData(), r.getLen());
		return true;
	}

	else
	{
		bool secondRun = false;
		while(true)
		{
			auto slotsSize = header.slotCount * sizeof(SlottedPageHeader);
			auto preStartSpace = header.dataStart - slotsSize;

			// Case 2
			if (preStartSpace >= r.getLen())
			{
				header.freeSpace -= spaceDiff;
				slot.offset = header.dataStart - r.getLen();
				slot.length = r.getLen();
				memcpy(data + slot.offset, r.getData(), r.getLen());
				return true;
			}

			// Case 3
			else
			{
				if (secondRun) return false;
				compactify();
				secondRun = true;
			}	
		}
		return false;
	}
}