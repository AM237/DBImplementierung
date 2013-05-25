
///////////////////////////////////////////////////////////////////////////////
// FreeSpaceInventory.cpp
///////////////////////////////////////////////////////////////////////////////


#include "FreeSpaceInventory.h"

using namespace std;

// _____________________________________________________________________________
FreeSpaceInventory::FreeSpaceInventory(int fd, bool visible, uint64_t id) 
                  : Segment(visible, id)
{


}

// _____________________________________________________________________________
void* FreeSpaceInventory::nextPage()
{

	return nullptr;
}

