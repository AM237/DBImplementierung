
///////////////////////////////////////////////////////////////////////////////
// TID.h
//////////////////////////////////////////////////////////////////////////////


#ifndef TID_H
#define TID_H


// An object representing a tupel identifier. 
// Marks the pageNo and the internal (slot) tid using a a single 8 byte int
// value. The TID is divided as follows:
// | 56 Bit = pageId | 8 bit = slot id |
//
// Reason: 8 bits for the slot id make it possible to address 256 different
// slots. This indicates that a page can hold 256 record entries, averaging
// out to a size per entry of 16 bytes. Addressing any more slots would
// necessarily reduce this entry size. Addressing any less slots would allow
// too few records to be addressed.
//
// 56 Bits for the pageId makes it possible to address more than enough pages
// Choosing a total of 64 bits allows a TID to be naturally byte aligned.
typedef struct {
  uint64_t pageId:56;
  uint8_t slotId:8;
} TID;

#endif  // TID_H