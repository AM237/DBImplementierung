
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
	BTree<T, CMP>(uint64_t k, uint64_t l);
	
	// Destructor
	~BTree<T, CMP>();
	
	// Inserts a new key/TID pair into the tree. Does not support non-unique
	// entries.
	void insert(T key, TID tid);
	
	// Deletes a specified key. Underfull pages are accepted. Returns false
	// iff key was not found.
	bool erase(T key);
	
	// Returns a TID or indicates that the key was not found (via exception)
	TID lookup(T key);
	
	// Returns an iterator to the first element of the result set.
	void lookupRange(T start, T end);

private:

	// Materializes the b tree to file.
	void writeToFile();
	
	// Starting at the given node, navigates down the tree to the leaf node
	// where the given key should be inserted. Returns pointer to that leaf.
	// Assumes that the keys are sorted with respect to comp, and that the keys
	// and values match with respect to their indices (e.g. first key separates 
	// the first pointer from the second pointer)
	BTreeLeafNode<T>* navigateToLeaf(T key, BTreeNode<T>* start);

	// Splits the given node and accomodates the entries along the split.
	// Called recursively if necessary. Assumes given node has an overflow of 1
	void splitNode(BTreeNode<T>* node);
	
	// Data structure to contain all nodes
	std::vector<BTreeNode<T>*> nodes;
	
	// The root of the tree
	BTreeNode<T>* root;

	// Comparison predicate
	CMP comp;

	// Underflow / overflow parameters
	uint64_t k, l;
};

#endif  // BTREE_H