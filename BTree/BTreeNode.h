
///////////////////////////////////////////////////////////////////////////////
// BTreeNode.h
//////////////////////////////////////////////////////////////////////////////


#ifndef BTREENODE_H
#define BTREENODE_H

#include <gtest/gtest.h>

// An object representing a tupel identifier. 
// Marks the pageNo and the internal tid
struct TID
{
public: 
	TID(uint64_t pageno, uint64_t tid) : pageno(pageno), tid(tid) { }
	uint64_t pageno;
	uint64_t tid;
};



// Class representing a node in a B+ Tree. -------------------------------------
template<class T> class BTreeNode
{
public:
	// Constructor. This Node underflows when it has less then #min entries,
	// and overflows when it has more than #max entries.
	BTreeNode(int min, int max) : min(min), max(max) { }

private:
	// The number of entries, underflow, overflow markers
	uint64_t count;
	int min, max;
	
	// (Opaque) Keys held by this node
	std::vector<T> keys;
};



// Class representing a leaf node in a B+ Tree ---------------------------------
template<class T> class BTreeLeafNode : public BTreeNode<T>
{
public:	
	// Constructor
	BTreeLeafNode(int min, int max, BTreeLeafNode<T>* next = NULL) 
		: BTreeNode<T>(min, max) { this->next = next; }


private:
	// Pointer to the next leaf node
	BTreeLeafNode<T>* next;
	
	// Values (TIDs) held by this node
	std::vector<TID> values;
};



// Class representing an inner node in a B+ Tree -------------------------------
template<class T> class BTreeInnerNode : public BTreeNode<T>
{
public:	
	// Constructor
	BTreeInnerNode(int min, int max) : BTreeNode<T>(min, max) { }

private:
	// Values (pointers to other nodes) held by this node
	std::vector<BTreeNode<T>*> values;
};

#endif  // BTREENODE_H
