
///////////////////////////////////////////////////////////////////////////////
// BTreeRangeIterator.h
//////////////////////////////////////////////////////////////////////////////


#ifndef BTREERANGEITERATOR_H
#define BTREERANGEITERATOR_H

#include "BTree.h"
#include <gtest/gtest.h>


// Exception thrown when searching for key that is not found
class StartKeyOutOfBounds: public std::exception
{
  virtual const char* what() const throw() 
  	{ return "Start key is larger than largest element in tree"; }
};

// Exception thrown when searching for key that is not found
class InvalidRange: public std::exception
{
  virtual const char* what() const throw() 
  	{ return "Given lookup range is invalid"; }
};


// Forward declaration of the BTree
template<class T, class CMP> class BTree;

// An iterator that iterates over a given key range on a given BTree.
template<class T, class CMP> class BTreeRangeIterator
{
public:

	// Constructor, gets the key bounds of the range to be searched as well
	// as a pointer to the btree.
	// Search semantics: 
	// 		keyStart = nullptr, keyEnd != nullptr : (-inf, keyEnd]
	//      keyStart != nullptr, keyEnd = nullptr : [keyStart, inf)
	//      keyStart != nullptr, keyEnd != nullptr : [keyStart, keyEnd]
	//		keyStart == nullptr, keyEnd == nullptr: not defined
	// TODO: one side unbounded
	BTreeRangeIterator(BTree<T, CMP>* btree, T* keyStart, T* keyEnd); 

	~BTreeRangeIterator() { }

	// Returns the next TID in the range specified by [keyStart, keyEnd]
	TID next();

private:

	// Comparison equivalent to comp;
	bool cmp(T key1, T key2);

	// The tree containing the keys
	BTree<T, CMP>* btree;

	// The next leaf to be examined by the iterator
	BTreeLeafNode<T>* currentLeaf, lastSafeLeaf;

	// Exception instances
	StartKeyOutOfBounds keyOutOfBounds;
	InvalidRange invalidRange;

	// The specified range
	T keyStart, keyEnd;

	// last TID compared
	TID currentTID;

	// Comparison predicate
	CMP comp;

	// The under in the currentLeaf current which the next TID to be returned 
	// by the iterator is found.
	int index;

	// Tell wether search interval is unbounded
	bool leftBound, rightBound;
};

#endif  // BTREERANGEITERATOR_H