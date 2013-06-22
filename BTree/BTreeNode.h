
///////////////////////////////////////////////////////////////////////////////
// BTreeNode.h
//////////////////////////////////////////////////////////////////////////////


#ifndef BTREENODE_H
#define BTREENODE_H

#include "../SegmentManager/TID.h"
#include <gtest/gtest.h>


// Class representing a node in a B+ Tree. -------------------------------------
template<class T> class BTreeNode
{
public:

	// Constructor. Underflow and overflow are controlled by the BTree itself.
	BTreeNode(BTreeNode<T>* parent = NULL) : parent(parent) { }
	~BTreeNode() { }

private:

	// The number of entries
	uint64_t count;
		
	// (Opaque) Keys held by this node
	std::vector<T> keys;

	// The parent of this node
	BTreeNode<T>* parent;
	
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
	BTreeInnerNode() : BTreeNode<T>() 
	{ 
		this->isLeaf = false;
		values.push_back(nullptr);
	}

private:

	// Values (pointers to other nodes) held by this node
	std::vector<BTreeNode<T>*> values;
};

#endif  // BTREENODE_H