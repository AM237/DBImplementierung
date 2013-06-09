
///////////////////////////////////////////////////////////////////////////////
// BTree.h
//////////////////////////////////////////////////////////////////////////////


#ifndef BTREE_H
#define BTREE_H

#include "BTreeNode.h"
#include <gtest/gtest.h>

// Class representing a B+ Tree. Parametrized to allow for generic element
// storage and comparison. CMP is a class / struct implementing a binary 
// operator() function
template<class T, class CMP> class BTree
{
public:	
	// Constructor. Inner nodes underflow at k entries, and overflow at 2k
	// entries. Leaves underflow at l entries, and overflow at 2l entries.
	// The root has either max 2k entries, or it is a leaf with max 2l entries.
	BTree(int k, int l);
	
	// Inserts a new key/TID pair into the tree.
	void insert(T key, TID tid);
	
	// Deletes a specified key. Underfull pages are accepted. Returns false
	// iff key was not found.
	bool erase(T key);
	
	// Returns a TID or indicates that the key was not found (via exception)
	TID lookup(T key);
	
	// Returns an iterator to the first element of the result set.
	void lookupRange(T start, T end);

private:
	// Underflow / overflow parameters
	int k, l;
	
	// Data structure to contain all nodes
	std::vector<BTreeNode<T>*> nodes;
};

#endif  // BTREE_H
