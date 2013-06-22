
///////////////////////////////////////////////////////////////////////////////
// TID.h
//////////////////////////////////////////////////////////////////////////////


#ifndef TID_H
#define TID_H


// An object representing a tupel identifier. 
// Marks the pageNo and the internal (slot) tid using a a single 8 byte int
// value. The TID is divided as follows:
// | 56 Bit = pageId | 8 bit = slot id |
struct TID
{
public: 
	TID(uint64_t tid) : tid(tid) { }
	uint64_t tid;
};

#endif  // TID_H