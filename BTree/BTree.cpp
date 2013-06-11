
///////////////////////////////////////////////////////////////////////////////
// BTree.cpp
///////////////////////////////////////////////////////////////////////////////


#include "BTree.h"

#include <algorithm>
#include <vector>

using namespace std;

// _____________________________________________________________________________
template<class T, class CMP> BTree<T, CMP>::BTree(int k, int l)
{
	this->k = k;
	this->l = l;
	root = new BTreeLeafNode<T>();
}

// _____________________________________________________________________________
template<class T, class CMP> BTree<T, CMP>::~BTree()
{
	writeToFile();
	delete root;
	for (auto &node : nodes) delete node;
}

// _____________________________________________________________________________
template<class T, class CMP> BTreeLeafNode<T>* BTree<T, CMP>::
	navigateToLeaf(T key, BTreeNode<T>* start)
{
	// Recursion base case
	if (start.isLeaf) return (BTreeLeafNode<T>*)start;

	// Navigate through an inner node: first check if all keys held in the node
	// are smaller than the given key -> follow last pointer in node


	// Otherwise, find the first key in the vector that is greater than
	// the search key -> follow pointer that immediately precedes this key



}


// _____________________________________________________________________________
template<class T, class CMP> void BTree<T, CMP>::insert(T key, TID tid)
{
	auto insertLeaf = navigateToLeaf(root);
}
