
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

	// Constructor. Underflow and overflow are controlled by the BTree itself.
	BTreeNode() { }
	~BTreeNode() { }

private:

	// Converts this node into a sequence of bytes.
	// The actual length and format depends on whether this node is a leaf node
	// or an inner node.
	//virtual std::vector<char> serialize()=0;
	
	// Parses the given vector according to the agreed format, and fills 
	// the node's data structures appropriately.
	//virtual void deserialize(std::vector<char> serialized)=0;
	
	// Gets the size in bytes of the serialized object
	//virtual uint64_t getSize()=0;

	// The number of entries
	uint64_t count;
		
	// (Opaque) Keys held by this node
	std::vector<T> keys;
	
	// Marks whether this node is a leaf or not
	bool isLeaf;
};



// Class representing a leaf node in a B+ Tree ---------------------------------
template<class T> class BTreeLeafNode : public BTreeNode<T>
{
public:	

	// Constructor
	BTreeLeafNode(BTreeLeafNode<T>* next = NULL) : BTreeNode<T>() 
	{ this->next = next; this->isLeaf = true; }

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
	BTreeInnerNode() : BTreeNode<T>() { this->isLeaf = false; }

private:

	// Values (pointers to other nodes) held by this node
	std::vector<BTreeNode<T>*> values;
};

#endif  // BTREENODE_H
